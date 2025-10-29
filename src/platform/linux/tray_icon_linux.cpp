#include <unistd.h>
#include <iostream>
#include <optional>
#include <string>

// Platform-specific includes for Linux
#ifdef __linux__
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <libayatana-appindicator/app-indicator.h>
#define HAS_GTK 1
#define HAS_AYATANA_APPINDICATOR 1
#else
#define HAS_GTK 0
#define HAS_AYATANA_APPINDICATOR 0
#endif

#include "../../foundation/id_allocator.h"
#include "../../image.h"
#include "../../menu.h"
#include "../../tray_icon.h"

namespace nativeapi {

// Private implementation class
class TrayIcon::Impl {
 public:
  std::shared_ptr<Image> image_;

  Impl(AppIndicator* indicator)
      : app_indicator_(indicator),
        title_(std::nullopt),
        tooltip_(std::nullopt),
        context_menu_(nullptr),
        visible_(false) {
    id_ = IdAllocator::Allocate<TrayIcon>();
  }

  ~Impl() {
    // Cancel any pending cleanup timeouts
    // Check if source exists before removing to avoid GLib warnings
    GMainContext* context = g_main_context_default();
    for (guint source_id : pending_cleanup_sources_) {
      if (source_id > 0) {
        GSource* source = g_main_context_find_source_by_id(context, source_id);
        if (source) {
          g_source_remove(source_id);
        }
      }
    }
    pending_cleanup_sources_.clear();
  }

  AppIndicator* app_indicator_;
  std::shared_ptr<Menu> context_menu_;  // Store menu shared_ptr to keep it alive
  std::optional<std::string> title_;
  std::optional<std::string> tooltip_;
  bool visible_;
  TrayIconId id_;
  std::vector<guint> pending_cleanup_sources_;  // Track GLib timeout source IDs
};

TrayIcon::TrayIcon() : pimpl_(std::make_unique<Impl>(nullptr)) {
#if HAS_GTK && HAS_AYATANA_APPINDICATOR
  // Create a unique ID for this tray icon
  static int next_indicator_id = 1;
  std::string indicator_id = "nativeapi-tray-" + std::to_string(next_indicator_id++);

  // Create a new tray using AppIndicator
  G_GNUC_BEGIN_IGNORE_DEPRECATIONS  // TODO: Use libayatana-appindicator-glib instead of
                                    // libayatana-appindicator in the future
      AppIndicator* app_indicator =
          app_indicator_new(indicator_id.c_str(),
                            "application-default-icon",  // Default icon name
                            APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
  G_GNUC_END_IGNORE_DEPRECATIONS

  if (app_indicator) {
    // Reinitialize the Impl with the created indicator
    pimpl_ = std::make_unique<Impl>(app_indicator);
    pimpl_->visible_ = true;
  }
#endif
}

TrayIcon::TrayIcon(void* tray) : pimpl_(std::make_unique<Impl>((AppIndicator*)tray)) {
  // Make the indicator visible by default
  if (pimpl_->app_indicator_) {
    pimpl_->visible_ = true;
  }
}

TrayIcon::~TrayIcon() {
  if (pimpl_->app_indicator_) {
    app_indicator_set_status(pimpl_->app_indicator_, APP_INDICATOR_STATUS_PASSIVE);
    g_object_unref(pimpl_->app_indicator_);
    pimpl_->app_indicator_ = nullptr;
  }
}

TrayIconId TrayIcon::GetId() {
  return pimpl_->id_;
}

void TrayIcon::SetIcon(std::shared_ptr<Image> image) {
  if (!pimpl_->app_indicator_) {
    return;
  }

  // Store the image reference
  pimpl_->image_ = image;

  // If no image provided, use default icon
  if (!image) {
    app_indicator_set_icon_full(pimpl_->app_indicator_, "application-default-icon", "Tray Icon");
    return;
  }

  // Get the native GdkPixbuf object
  GdkPixbuf* pixbuf = static_cast<GdkPixbuf*>(image->GetNativeObject());
  if (!pixbuf) {
    app_indicator_set_icon_full(pimpl_->app_indicator_, "application-default-icon", "Tray Icon");
    return;
  }

  // Create temporary PNG file
  char temp_path[] = "/tmp/tray_icon_XXXXXX";
  int fd = mkstemp(temp_path);
  if (fd == -1) {
    app_indicator_set_icon_full(pimpl_->app_indicator_, "application-default-icon", "Tray Icon");
    return;
  }
  close(fd);

  // Append .png extension
  std::string png_path(temp_path);
  png_path += ".png";

  // Save pixbuf to PNG file
  GError* error = nullptr;
  gboolean success = gdk_pixbuf_save(pixbuf, png_path.c_str(), "png", &error, nullptr);

  // Always clean up the original temporary file
  unlink(temp_path);

  if (error) {
    g_error_free(error);
  }

  if (success) {
    // Set the icon and schedule cleanup
    app_indicator_set_icon_full(pimpl_->app_indicator_, png_path.c_str(), "");

    // Track the cleanup timeout source ID so we can cancel it if needed
    guint source_id = g_timeout_add(
        5000,
        [](gpointer data) -> gboolean {
          unlink(static_cast<char*>(data));
          g_free(data);
          return FALSE;  // Don't repeat
        },
        g_strdup(png_path.c_str()));

    // Store source ID for cleanup in destructor
    pimpl_->pending_cleanup_sources_.push_back(source_id);
  } else {
    // Fallback to default icon
    app_indicator_set_icon_full(pimpl_->app_indicator_, "application-default-icon", "Tray Icon");
    unlink(png_path.c_str());
  }
}

std::shared_ptr<Image> TrayIcon::GetIcon() const {
  return pimpl_->image_;
}

void TrayIcon::SetTitle(std::optional<std::string> title) {
  pimpl_->title_ = title;
  // AppIndicator uses the title as the accessible name and in some desktop
  // environments
  if (pimpl_->app_indicator_) {
    const char* title_str = title.has_value() ? title->c_str() : "";
    app_indicator_set_label(pimpl_->app_indicator_, title_str, NULL);
  }
}

std::optional<std::string> TrayIcon::GetTitle() {
  return pimpl_->title_;
}

void TrayIcon::SetTooltip(std::optional<std::string> tooltip) {
  pimpl_->tooltip_ = tooltip;
  // AppIndicator doesn't have direct tooltip support like GtkStatusIcon
  // The tooltip functionality is typically handled through the title
  // or through custom menu items. We'll store it for potential future use.
}

std::optional<std::string> TrayIcon::GetTooltip() {
  return pimpl_->tooltip_;
}

void TrayIcon::SetContextMenu(std::shared_ptr<Menu> menu) {
  // Store the menu shared_ptr to keep it alive
  pimpl_->context_menu_ = menu;

  // AppIndicator requires a menu to be set
  if (pimpl_->app_indicator_ && menu && menu->GetNativeObject()) {
    GtkMenu* gtk_menu = static_cast<GtkMenu*>(menu->GetNativeObject());
    app_indicator_set_menu(pimpl_->app_indicator_, gtk_menu);
    // Ensure the menu and its children are realized; actual popup is controlled
    // by the indicator. Using map/unmap signals in menu_linux.cpp prevents
    // premature opened/closed emissions.
    gtk_widget_show_all(GTK_WIDGET(gtk_menu));
  }
}

std::shared_ptr<Menu> TrayIcon::GetContextMenu() {
  return pimpl_->context_menu_;
}

Rectangle TrayIcon::GetBounds() {
  Rectangle bounds = {0, 0, 0, 0};

  // AppIndicator doesn't provide geometry information like GtkStatusIcon did
  // This is a limitation of the AppIndicator API as it's handled by the
  // system tray implementation. We return empty bounds.
  // In most modern desktop environments, this information isn't available
  // to applications for security reasons.

  return bounds;
}

bool TrayIcon::SetVisible(bool visible) {
  if (!pimpl_->app_indicator_) {
    return false;
  }

  if (visible) {
    app_indicator_set_status(pimpl_->app_indicator_, APP_INDICATOR_STATUS_ACTIVE);
  } else {
    app_indicator_set_status(pimpl_->app_indicator_, APP_INDICATOR_STATUS_PASSIVE);
  }

  pimpl_->visible_ = visible;
  return true;
}

bool TrayIcon::IsVisible() {
  if (pimpl_->app_indicator_) {
    AppIndicatorStatus status = app_indicator_get_status(pimpl_->app_indicator_);
    return status == APP_INDICATOR_STATUS_ACTIVE;
  }
  return false;
}

bool TrayIcon::OpenContextMenu() {
  if (!pimpl_->context_menu_ || !pimpl_->context_menu_->GetNativeObject()) {
    return false;
  }

  // AppIndicator shows context menu automatically on right-click
  // We don't need to manually show it as it's managed by the indicator
  // framework
  return true;
}

bool TrayIcon::CloseContextMenu() {
  if (!pimpl_->context_menu_) {
    return true;  // No menu to close, consider success
  }

  // AppIndicator manages menu visibility automatically
  // There's no direct way to programmatically close the menu
  // but we can return true as the operation is conceptually successful
  return true;
}

void* TrayIcon::GetNativeObjectInternal() const {
  return static_cast<void*>(pimpl_->app_indicator_);
}

void TrayIcon::StartEventListening() {
  // Called automatically when first listener is added
  // AppIndicator handles events automatically, no platform-specific setup needed
}

void TrayIcon::StopEventListening() {
  // Called automatically when last listener is removed
  // AppIndicator handles events automatically, no platform-specific cleanup needed
}

}  // namespace nativeapi
