#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <string>
#include <vector>

#include "../../drag_source_impl.h"
#include "../../image.h"

namespace nativeapi {
namespace {

enum TargetInfo : guint { kUriList = 0, kText = 1 };

GdkDragAction ToGdkAction(DragOperation operation) {
  switch (operation) {
    case DragOperation::Copy:
      return GDK_ACTION_COPY;
    case DragOperation::Move:
      return GDK_ACTION_MOVE;
    case DragOperation::Link:
      return GDK_ACTION_LINK;
    case DragOperation::None:
      break;
  }
  return static_cast<GdkDragAction>(0);
}

DragOperation FromGdkAction(GdkDragAction action) {
  if (action & GDK_ACTION_COPY) return DragOperation::Copy;
  if (action & GDK_ACTION_MOVE) return DragOperation::Move;
  if (action & GDK_ACTION_LINK) return DragOperation::Link;
  return DragOperation::None;
}

GdkDevice* Pointer() {
  GdkDisplay* display = gdk_display_get_default();
  GdkSeat* seat = display ? gdk_display_get_default_seat(display) : nullptr;
  return seat ? gdk_seat_get_pointer(seat) : nullptr;
}

bool QueryPointer(Point& position, bool& primary_button_down) {
  GdkDevice* pointer = Pointer();
  GdkWindow* root = gdk_get_default_root_window();
  if (!pointer || !root) {
    return false;
  }
  gint x = 0;
  gint y = 0;
  GdkModifierType mask = static_cast<GdkModifierType>(0);
  gdk_window_get_device_position(root, pointer, &x, &y, &mask);
  position = {static_cast<double>(x), static_cast<double>(y)};
  primary_button_down = (mask & GDK_BUTTON1_MASK) != 0;
  return true;
}

}  // namespace

struct DragSource::Impl::Platform {
  explicit Platform(Impl* impl) : impl(impl) {}
  ~Platform() { Disconnect(); }

  void Disconnect() {
    if (!widget) {
      return;
    }
    for (gulong id : handler_ids) {
      g_signal_handler_disconnect(widget, id);
    }
    handler_ids.clear();
    g_object_unref(widget);
    widget = nullptr;
    context = nullptr;
  }

  static void OnDataGet(GtkWidget*,
                        GdkDragContext* context,
                        GtkSelectionData* selection,
                        guint info,
                        guint,
                        gpointer user_data) {
    auto* self = static_cast<Platform*>(user_data);
    if (context != self->context) {
      return;
    }
    if (info == kUriList) {
      std::vector<gchar*> uris;
      for (const auto& path : self->file_paths) {
        if (gchar* uri = g_filename_to_uri(path.c_str(), nullptr, nullptr)) {
          uris.push_back(uri);
        }
      }
      uris.push_back(nullptr);
      gtk_selection_data_set_uris(selection, uris.data());
      for (gchar* uri : uris) {
        g_free(uri);
      }
    } else if (info == kText) {
      gtk_selection_data_set_text(selection, self->text.c_str(), -1);
    }
  }

  static gboolean OnFailed(GtkWidget*, GdkDragContext* context, GtkDragResult, gpointer user_data) {
    auto* self = static_cast<Platform*>(user_data);
    if (context == self->context) {
      self->failed = true;
    }
    return FALSE;  // Let GTK animate the icon back.
  }

  static void OnEnd(GtkWidget*, GdkDragContext* context, gpointer user_data) {
    auto* self = static_cast<Platform*>(user_data);
    if (context != self->context) {
      return;
    }
    const DragOperation operation =
        self->failed ? DragOperation::None
                     : FromGdkAction(gdk_drag_context_get_selected_action(context));
    self->Disconnect();
    Point position{0, 0};
    bool button_down = false;
    QueryPointer(position, button_down);
    self->impl->Finish(position, operation);
  }

  Impl* impl;
  GtkWidget* widget = nullptr;
  GdkDragContext* context = nullptr;
  std::vector<gulong> handler_ids;
  bool failed = false;
  // A copy of the data taken at the start: changes to the source must not
  // affect the drag in progress.
  std::vector<std::string> file_paths;
  std::string text;
};

bool DragSource::IsSupported() {
  return true;
}

DragSource::Impl::Impl(DragSource* owner)
    : owner(owner), platform(std::make_unique<Platform>(this)) {}

DragSource::Impl::~Impl() = default;

bool DragSource::Impl::Start() {
  GtkWidget* widget = static_cast<GtkWidget*>(window->GetNativeObject());
  if (!widget || !GTK_IS_WIDGET(widget) || !gtk_widget_get_realized(widget)) {
    return false;
  }
  Point position{0, 0};
  bool button_down = false;
  if (!QueryPointer(position, button_down) || !button_down) {
    return false;
  }

  GtkTargetList* targets = gtk_target_list_new(nullptr, 0);
  if (!file_paths.empty()) {
    gtk_target_list_add_uri_targets(targets, kUriList);
  }
  if (text.has_value()) {
    gtk_target_list_add_text_targets(targets, kText);
  }

  auto* p = platform.get();
  p->Disconnect();
  p->file_paths = file_paths;
  p->text = text.value_or(std::string());
  p->failed = false;
  g_object_ref(widget);
  p->widget = widget;
  p->handler_ids = {
      g_signal_connect(widget, "drag-data-get", G_CALLBACK(&Platform::OnDataGet), p),
      g_signal_connect(widget, "drag-failed", G_CALLBACK(&Platform::OnFailed), p),
      g_signal_connect(widget, "drag-end", G_CALLBACK(&Platform::OnEnd), p),
  };

  // The triggering event, when there is one, gives GTK the right grab time.
  GdkEvent* event = gtk_get_current_event();
  GdkDragContext* context = gtk_drag_begin_with_coordinates(
      widget, targets, ToGdkAction(operation), 1, event, -1, -1);
  if (event) {
    gdk_event_free(event);
  }
  gtk_target_list_unref(targets);
  if (!context) {
    p->Disconnect();
    return false;
  }
  p->context = context;

  GdkPixbuf* pixbuf = image ? static_cast<GdkPixbuf*>(image->GetNativeObject()) : nullptr;
  if (pixbuf) {
    gtk_drag_set_icon_pixbuf(context, pixbuf, gdk_pixbuf_get_width(pixbuf) / 2,
                             gdk_pixbuf_get_height(pixbuf) / 2);
  } else {
    gtk_drag_set_icon_default(context);
  }
  return true;
}

}  // namespace nativeapi
