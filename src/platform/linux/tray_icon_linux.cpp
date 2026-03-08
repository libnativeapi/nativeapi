// Linux tray icon implemented via the StatusNotifierItem D-Bus specification.
// https://www.freedesktop.org/wiki/Specifications/StatusNotifierItem/
//
// Uses GDBus (part of GLib/GIO, already a transitive dependency of GTK) so that
// no GPL-licensed libayatana-appindicator dependency is required.

#include <gio/gio.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <sys/types.h>
#include <unistd.h>
#include <atomic>
#include <cstdint>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "../../foundation/id_allocator.h"
#include "../../image.h"
#include "../../menu.h"
#include "../../positioning_strategy.h"
#include "../../tray_icon.h"
#include "../../tray_icon_event.h"

namespace nativeapi {

// ── D-Bus introspection XML for org.kde.StatusNotifierItem ───────────────────

static const char kSniIntrospectionXml[] =
    "<node>"
    "  <interface name='org.kde.StatusNotifierItem'>"
    "    <property name='Category'           type='s'        access='read'/>"
    "    <property name='Id'                 type='s'        access='read'/>"
    "    <property name='Title'              type='s'        access='read'/>"
    "    <property name='Status'             type='s'        access='read'/>"
    "    <property name='WindowId'           type='u'        access='read'/>"
    "    <property name='IconName'           type='s'        access='read'/>"
    "    <property name='IconPixmap'         type='a(iiay)'  access='read'/>"
    "    <property name='OverlayIconName'    type='s'        access='read'/>"
    "    <property name='OverlayIconPixmap'  type='a(iiay)'  access='read'/>"
    "    <property name='AttentionIconName'  type='s'        access='read'/>"
    "    <property name='AttentionIconPixmap' type='a(iiay)' access='read'/>"
    "    <property name='AttentionMovieName' type='s'        access='read'/>"
    "    <property name='ToolTip'            type='(sa(iiay)ss)' access='read'/>"
    "    <property name='ItemIsMenu'         type='b'        access='read'/>"
    "    <property name='Menu'               type='o'        access='read'/>"
    "    <method name='ContextMenu'>"
    "      <arg type='i' direction='in' name='x'/>"
    "      <arg type='i' direction='in' name='y'/>"
    "    </method>"
    "    <method name='Activate'>"
    "      <arg type='i' direction='in' name='x'/>"
    "      <arg type='i' direction='in' name='y'/>"
    "    </method>"
    "    <method name='SecondaryActivate'>"
    "      <arg type='i' direction='in' name='x'/>"
    "      <arg type='i' direction='in' name='y'/>"
    "    </method>"
    "    <method name='Scroll'>"
    "      <arg type='i' direction='in' name='delta'/>"
    "      <arg type='s' direction='in' name='orientation'/>"
    "    </method>"
    "    <signal name='NewTitle'/>"
    "    <signal name='NewIcon'/>"
    "    <signal name='NewAttentionIcon'/>"
    "    <signal name='NewOverlayIcon'/>"
    "    <signal name='NewToolTip'/>"
    "    <signal name='NewStatus'>"
    "      <arg type='s' name='status'/>"
    "    </signal>"
    "  </interface>"
    "</node>";

// ── Icon pixel-data conversion ───────────────────────────────────────────────

// Returns a GVariant of type a(iiay) containing one entry for the supplied
// pixbuf (or an empty array when pixbuf is nullptr).  Each pixel is encoded as
// four bytes in network byte order: Alpha, Red, Green, Blue (ARGB32).
static GVariant* PixbufToSniIconPixmaps(GdkPixbuf* pixbuf) {
  GVariantBuilder array_builder;
  g_variant_builder_init(&array_builder, G_VARIANT_TYPE("a(iiay)"));

  if (pixbuf) {
    const int width = gdk_pixbuf_get_width(pixbuf);
    const int height = gdk_pixbuf_get_height(pixbuf);
    const int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    const int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    const gboolean has_alpha = gdk_pixbuf_get_has_alpha(pixbuf);
    const guchar* pixels = gdk_pixbuf_get_pixels(pixbuf);

    std::vector<uint8_t> argb;
    argb.reserve(static_cast<size_t>(width * height * 4));

    for (int row = 0; row < height; ++row) {
      const guchar* p = pixels + row * rowstride;
      for (int col = 0; col < width; ++col) {
        const uint8_t r = p[0];
        const uint8_t g = p[1];
        const uint8_t b = p[2];
        const uint8_t a = has_alpha ? p[3] : 255u;
        argb.push_back(a);
        argb.push_back(r);
        argb.push_back(g);
        argb.push_back(b);
        p += n_channels;
      }
    }

    GVariantBuilder entry_builder;
    g_variant_builder_init(&entry_builder, G_VARIANT_TYPE("(iiay)"));
    g_variant_builder_add(&entry_builder, "i", width);
    g_variant_builder_add(&entry_builder, "i", height);
    g_variant_builder_add_value(
        &entry_builder,
        g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, argb.data(), argb.size(), sizeof(uint8_t)));
    g_variant_builder_add_value(&array_builder, g_variant_builder_end(&entry_builder));
  }

  return g_variant_builder_end(&array_builder);
}

// ── Private implementation ───────────────────────────────────────────────────

class TrayIcon::Impl {
 public:
  TrayIcon* owner_;
  TrayIconId id_;

  std::shared_ptr<Image> image_;
  std::optional<std::string> title_;
  std::optional<std::string> tooltip_;
  std::shared_ptr<Menu> context_menu_;
  bool visible_;
  ContextMenuTrigger context_menu_trigger_;

  // D-Bus state
  GDBusConnection* connection_;
  guint registration_id_;
  guint name_owner_id_;
  std::string service_name_;

  explicit Impl(TrayIcon* owner)
      : owner_(owner),
        image_(nullptr),
        title_(std::nullopt),
        tooltip_(std::nullopt),
        context_menu_(nullptr),
        visible_(false),
        context_menu_trigger_(ContextMenuTrigger::None),
        connection_(nullptr),
        registration_id_(0),
        name_owner_id_(0) {
    id_ = IdAllocator::Allocate<TrayIcon>();
  }

  ~Impl() { Cleanup(); }

  // Connect to the session bus, register the SNI object, and request a
  // well-known service name.  Returns false on error (icon will be invisible).
  bool Init() {
    GError* error = nullptr;
    connection_ = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &error);
    if (!connection_) {
      if (error) {
        std::cerr << "[nativeapi] SNI: D-Bus session connection failed: " << error->message
                  << std::endl;
        g_error_free(error);
      }
      return false;
    }

    static std::atomic<int> next_sni_index{1};
    service_name_ = "org.kde.StatusNotifierItem-" +
                    std::to_string(static_cast<long>(getpid())) + "-" +
                    std::to_string(next_sni_index++);

    GDBusNodeInfo* node_info = g_dbus_node_info_new_for_xml(kSniIntrospectionXml, &error);
    if (!node_info) {
      if (error) {
        std::cerr << "[nativeapi] SNI: Bad introspection XML: " << error->message << std::endl;
        g_error_free(error);
      }
      return false;
    }

    GDBusInterfaceInfo* iface_info =
        g_dbus_node_info_lookup_interface(node_info, "org.kde.StatusNotifierItem");

    static const GDBusInterfaceVTable vtable = {
        &Impl::OnMethodCall,
        &Impl::OnGetProperty,
        nullptr  // no writable properties
    };

    registration_id_ = g_dbus_connection_register_object(connection_, "/StatusNotifierItem",
                                                          iface_info, &vtable,
                                                          this,     // user_data
                                                          nullptr,  // user_data_free_func
                                                          &error);
    g_dbus_node_info_unref(node_info);

    if (registration_id_ == 0) {
      if (error) {
        std::cerr << "[nativeapi] SNI: Object registration failed: " << error->message
                  << std::endl;
        g_error_free(error);
      }
      return false;
    }

    name_owner_id_ = g_bus_own_name_on_connection(connection_, service_name_.c_str(),
                                                   G_BUS_NAME_OWNER_FLAGS_NONE, &Impl::OnNameAcquired,
                                                   &Impl::OnNameLost, this, nullptr);
    return true;
  }

  void Cleanup() {
    // Nullify the back-pointer first so any in-flight GLib callbacks that
    // haven't been dispatched yet will see a null owner and skip event emission.
    owner_ = nullptr;

    if (name_owner_id_ != 0) {
      g_bus_unown_name(name_owner_id_);
      name_owner_id_ = 0;
    }
    if (connection_ && registration_id_ != 0) {
      g_dbus_connection_unregister_object(connection_, registration_id_);
      registration_id_ = 0;
    }
    if (connection_) {
      g_object_unref(connection_);
      connection_ = nullptr;
    }
  }

  void EmitSignal(const char* signal_name, GVariant* params = nullptr) {
    if (!connection_ || registration_id_ == 0) return;
    GError* error = nullptr;
    g_dbus_connection_emit_signal(connection_, nullptr, "/StatusNotifierItem",
                                   "org.kde.StatusNotifierItem", signal_name, params, &error);
    if (error) g_error_free(error);
  }

  // ── D-Bus name callbacks ──────────────────────────────────────────────────

  static void OnNameAcquired(GDBusConnection* conn, const gchar* name, gpointer user_data) {
    Impl* self = static_cast<Impl*>(user_data);
    if (self) self->RegisterWithWatcher(conn, name);
  }

  static void OnNameLost(GDBusConnection*, const gchar* name, gpointer) {
    std::cerr << "[nativeapi] SNI: lost D-Bus name " << (name ? name : "(null)") << std::endl;
  }

  // Try both KDE and Canonical watcher service names.
  void RegisterWithWatcher(GDBusConnection* conn, const gchar* service_name) {
    static const char* const kWatchers[] = {
        "org.kde.StatusNotifierWatcher",
        "com.canonical.StatusNotifierWatcher",
        nullptr,
    };
    for (int i = 0; kWatchers[i]; ++i) {
      GError* error = nullptr;
      GVariant* reply = g_dbus_connection_call_sync(
          conn, kWatchers[i], "/StatusNotifierWatcher", kWatchers[i],
          "RegisterStatusNotifierItem", g_variant_new("(s)", service_name), nullptr,
          G_DBUS_CALL_FLAGS_NONE, 2000, nullptr, &error);
      if (reply) {
        g_variant_unref(reply);
        return;
      }
      if (error) g_error_free(error);
    }
    std::cerr << "[nativeapi] SNI: no StatusNotifierWatcher found; tray icon may not appear"
              << std::endl;
  }

  // ── D-Bus method-call handler ─────────────────────────────────────────────

  static void OnMethodCall(GDBusConnection*, const gchar*, const gchar*, const gchar*,
                            const gchar* method_name, GVariant* parameters,
                            GDBusMethodInvocation* invocation, gpointer user_data) {
    Impl* self = static_cast<Impl*>(user_data);
    if (!self) {
      g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
                                             "Internal error");
      return;
    }

    gint x = 0, y = 0;

    if (g_strcmp0(method_name, "Activate") == 0) {
      if (parameters) g_variant_get(parameters, "(ii)", &x, &y);
      g_dbus_method_invocation_return_value(invocation, nullptr);
      if (self->owner_) {
        self->owner_->Emit(TrayIconClickedEvent(self->id_));
        if (self->context_menu_trigger_ == ContextMenuTrigger::Clicked) {
          self->owner_->OpenContextMenu();
        }
      }

    } else if (g_strcmp0(method_name, "SecondaryActivate") == 0) {
      if (parameters) g_variant_get(parameters, "(ii)", &x, &y);
      g_dbus_method_invocation_return_value(invocation, nullptr);
      if (self->owner_) {
        self->owner_->Emit(TrayIconDoubleClickedEvent(self->id_));
        if (self->context_menu_trigger_ == ContextMenuTrigger::DoubleClicked) {
          self->owner_->OpenContextMenu();
        }
      }

    } else if (g_strcmp0(method_name, "ContextMenu") == 0) {
      if (parameters) g_variant_get(parameters, "(ii)", &x, &y);
      g_dbus_method_invocation_return_value(invocation, nullptr);
      if (self->owner_) {
        self->owner_->Emit(TrayIconRightClickedEvent(self->id_));
        if (self->context_menu_trigger_ == ContextMenuTrigger::RightClicked) {
          self->owner_->OpenContextMenu();
        }
      }

    } else if (g_strcmp0(method_name, "Scroll") == 0) {
      g_dbus_method_invocation_return_value(invocation, nullptr);

    } else {
      g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR,
                                             G_DBUS_ERROR_UNKNOWN_METHOD,
                                             "Unknown method: %s", method_name);
    }
  }

  // ── D-Bus property getter ─────────────────────────────────────────────────

  static GVariant* OnGetProperty(GDBusConnection*, const gchar*, const gchar*, const gchar*,
                                  const gchar* property_name, GError** error,
                                  gpointer user_data) {
    Impl* self = static_cast<Impl*>(user_data);
    if (!self) return nullptr;

    if (g_strcmp0(property_name, "Category") == 0)
      return g_variant_new_string("ApplicationStatus");

    if (g_strcmp0(property_name, "Id") == 0)
      return g_variant_new_string("nativeapi-tray");

    if (g_strcmp0(property_name, "Title") == 0)
      return g_variant_new_string(self->title_.value_or("").c_str());

    if (g_strcmp0(property_name, "Status") == 0)
      return g_variant_new_string(self->visible_ ? "Active" : "Passive");

    if (g_strcmp0(property_name, "WindowId") == 0)
      return g_variant_new_uint32(0);

    if (g_strcmp0(property_name, "IconName") == 0)
      return g_variant_new_string("");  // we use IconPixmap instead

    if (g_strcmp0(property_name, "IconPixmap") == 0) {
      // image_linux.cpp's GetNativeObjectInternal() returns GdkPixbuf* on Linux.
      GdkPixbuf* pb =
          self->image_ ? static_cast<GdkPixbuf*>(self->image_->GetNativeObject()) : nullptr;
      return PixbufToSniIconPixmaps(pb);
    }

    if (g_strcmp0(property_name, "OverlayIconName") == 0) return g_variant_new_string("");
    if (g_strcmp0(property_name, "OverlayIconPixmap") == 0) return PixbufToSniIconPixmaps(nullptr);
    if (g_strcmp0(property_name, "AttentionIconName") == 0) return g_variant_new_string("");
    if (g_strcmp0(property_name, "AttentionIconPixmap") == 0)
      return PixbufToSniIconPixmaps(nullptr);
    if (g_strcmp0(property_name, "AttentionMovieName") == 0) return g_variant_new_string("");

    if (g_strcmp0(property_name, "ToolTip") == 0) {
      // (sa(iiay)ss): iconName, iconPixmap[], title, description
      const std::string& tip = self->tooltip_.value_or(self->title_.value_or(""));
      return g_variant_new("(s@a(iiay)ss)", "", PixbufToSniIconPixmaps(nullptr),
                           self->title_.value_or("").c_str(), tip.c_str());
    }

    if (g_strcmp0(property_name, "ItemIsMenu") == 0) return g_variant_new_boolean(FALSE);
    if (g_strcmp0(property_name, "Menu") == 0) return g_variant_new_object_path("/");

    if (error) {
      *error = g_error_new(G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_PROPERTY,
                           "Unknown property: %s", property_name);
    }
    return nullptr;
  }
};

// ── TrayIcon public interface ─────────────────────────────────────────────────

TrayIcon::TrayIcon() : pimpl_(std::make_unique<Impl>(this)) {
  if (pimpl_->Init()) {
    pimpl_->visible_ = true;
  } else {
    std::cerr << "[nativeapi] TrayIcon: D-Bus initialisation failed; icon will not appear"
              << std::endl;
  }
}

TrayIcon::TrayIcon(void* /*tray*/) : pimpl_(std::make_unique<Impl>(this)) {
  // For API compatibility; create a fresh SNI tray icon ignoring the raw pointer.
  if (pimpl_->Init()) {
    pimpl_->visible_ = true;
  } else {
    std::cerr << "[nativeapi] TrayIcon: D-Bus initialisation failed; icon will not appear"
              << std::endl;
  }
}

TrayIcon::~TrayIcon() {
  // Impl::~Impl calls Cleanup(), which unregisters the D-Bus object and
  // releases the connection before pimpl_ is destroyed.
}

TrayIconId TrayIcon::GetId() {
  return pimpl_->id_;
}

void TrayIcon::SetIcon(std::shared_ptr<Image> image) {
  pimpl_->image_ = image;
  pimpl_->EmitSignal("NewIcon");
}

std::shared_ptr<Image> TrayIcon::GetIcon() const {
  return pimpl_->image_;
}

void TrayIcon::SetTitle(std::optional<std::string> title) {
  pimpl_->title_ = title;
  pimpl_->EmitSignal("NewTitle");
}

std::optional<std::string> TrayIcon::GetTitle() {
  return pimpl_->title_;
}

void TrayIcon::SetTooltip(std::optional<std::string> tooltip) {
  pimpl_->tooltip_ = tooltip;
  pimpl_->EmitSignal("NewToolTip");
}

std::optional<std::string> TrayIcon::GetTooltip() {
  return pimpl_->tooltip_;
}

void TrayIcon::SetContextMenu(std::shared_ptr<Menu> menu) {
  pimpl_->context_menu_ = menu;
}

std::shared_ptr<Menu> TrayIcon::GetContextMenu() {
  return pimpl_->context_menu_;
}

Rectangle TrayIcon::GetBounds() {
  // The SNI specification does not expose icon geometry; return empty bounds.
  return {0, 0, 0, 0};
}

bool TrayIcon::SetVisible(bool visible) {
  pimpl_->visible_ = visible;
  const char* status = visible ? "Active" : "Passive";
  pimpl_->EmitSignal("NewStatus", g_variant_new("(s)", status));
  return true;
}

bool TrayIcon::IsVisible() {
  return pimpl_->visible_;
}

bool TrayIcon::OpenContextMenu() {
  if (!pimpl_->context_menu_) return false;
  return pimpl_->context_menu_->Open(PositioningStrategy::CursorPosition());
}

bool TrayIcon::CloseContextMenu() {
  if (!pimpl_->context_menu_) return true;
  return pimpl_->context_menu_->Close();
}

void TrayIcon::SetContextMenuTrigger(ContextMenuTrigger trigger) {
  pimpl_->context_menu_trigger_ = trigger;
}

ContextMenuTrigger TrayIcon::GetContextMenuTrigger() {
  return pimpl_->context_menu_trigger_;
}

void* TrayIcon::GetNativeObjectInternal() const {
  return static_cast<void*>(pimpl_->connection_);
}

void TrayIcon::StartEventListening() {
  // GDBus dispatches D-Bus method calls via the GLib main loop automatically.
}

void TrayIcon::StopEventListening() {
  // Nothing to tear down; GDBus uses the GLib main loop.
}

}  // namespace nativeapi
