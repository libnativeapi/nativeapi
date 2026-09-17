#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <string>
#include <vector>

#include "../../drop_target_impl.h"

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

}  // namespace

struct DropTarget::Impl::Platform {
  explicit Platform(Impl* impl) : impl(impl) {}

  static unsigned SourceOperations(GdkDragContext* context) {
    const GdkDragAction actions = gdk_drag_context_get_actions(context);
    unsigned result = 0;
    for (DragOperation operation :
         {DragOperation::Copy, DragOperation::Move, DragOperation::Link}) {
      if (actions & ToGdkAction(operation)) {
        result |= OperationBit(operation);
      }
    }
    return result;
  }

  static GdkAtom FindTarget(GtkWidget* widget, GdkDragContext* context) {
    // Prefer files over text when the source offers both.
    GtkTargetList* files = gtk_target_list_new(nullptr, 0);
    gtk_target_list_add_uri_targets(files, kUriList);
    GdkAtom target = gtk_drag_dest_find_target(widget, context, files);
    gtk_target_list_unref(files);
    if (target == GDK_NONE) {
      target = gtk_drag_dest_find_target(widget, context, nullptr);
    }
    return target;
  }

  void CancelPendingLeave() {
    if (leave_source_id != 0) {
      g_source_remove(leave_source_id);
      leave_source_id = 0;
    }
  }

  static gboolean OnMotion(GtkWidget* widget,
                           GdkDragContext* context,
                           gint x,
                           gint y,
                           guint time,
                           gpointer user_data) {
    auto* self = static_cast<Platform*>(user_data);
    // A leave followed by a motion is a drag moving between child widgets.
    self->CancelPendingLeave();
    const Point position{static_cast<double>(x), static_cast<double>(y)};
    DragOperation operation;
    if (!self->inside) {
      self->inside = true;
      const bool has_data = FindTarget(widget, context) != GDK_NONE;
      operation = self->impl->Entered(position, SourceOperations(context), has_data);
    } else {
      operation = self->impl->Moved(position, SourceOperations(context));
    }
    gdk_drag_status(context, ToGdkAction(operation), time);
    return TRUE;
  }

  static gboolean OnLeaveIdle(gpointer user_data) {
    auto* self = static_cast<Platform*>(user_data);
    self->leave_source_id = 0;
    self->inside = false;
    self->impl->Exited(self->impl->last_position);
    return G_SOURCE_REMOVE;
  }

  static void OnLeave(GtkWidget*, GdkDragContext*, guint, gpointer user_data) {
    auto* self = static_cast<Platform*>(user_data);
    // GTK also emits drag-leave right before drag-drop; only a leave that is
    // not followed by a drop in the same turn is an exit.
    if (self->leave_source_id == 0) {
      self->leave_source_id = g_idle_add(&OnLeaveIdle, self);
    }
  }

  static gboolean OnDrop(GtkWidget* widget,
                         GdkDragContext* context,
                         gint x,
                         gint y,
                         guint time,
                         gpointer user_data) {
    auto* self = static_cast<Platform*>(user_data);
    self->CancelPendingLeave();
    self->inside = false;
    const GdkAtom target = FindTarget(widget, context);
    if (!self->impl->accepted || target == GDK_NONE) {
      self->impl->Exited({static_cast<double>(x), static_cast<double>(y)});
      gtk_drag_finish(context, FALSE, FALSE, time);
      return TRUE;
    }
    self->drop_position = {static_cast<double>(x), static_cast<double>(y)};
    gtk_drag_get_data(widget, context, target, time);
    return TRUE;
  }

  static void OnDataReceived(GtkWidget*,
                             GdkDragContext* context,
                             gint,
                             gint,
                             GtkSelectionData* selection,
                             guint,
                             guint time,
                             gpointer user_data) {
    auto* self = static_cast<Platform*>(user_data);
    std::vector<std::string> file_paths;
    std::string text;
    if (gchar** uris = gtk_selection_data_get_uris(selection)) {
      for (gchar** uri = uris; *uri; ++uri) {
        if (gchar* path = g_filename_from_uri(*uri, nullptr, nullptr)) {
          file_paths.emplace_back(path);
          g_free(path);
        }
      }
      g_strfreev(uris);
    } else if (guchar* chars = gtk_selection_data_get_text(selection)) {
      text = reinterpret_cast<const char*>(chars);
      g_free(chars);
    }
    const bool ok = !file_paths.empty() || !text.empty();
    const bool move = self->impl->operation == DragOperation::Move;
    gtk_drag_finish(context, ok, ok && move, time);
    if (ok) {
      self->impl->Dropped(self->drop_position, std::move(file_paths), std::move(text));
    } else {
      self->impl->Exited(self->drop_position);
    }
  }

  Impl* impl;
  GtkWidget* widget = nullptr;
  std::vector<gulong> handler_ids;
  guint leave_source_id = 0;
  bool inside = false;
  Point drop_position{0, 0};
};

bool DropTarget::IsSupported() {
  return true;
}

DropTarget::Impl::Impl(DropTarget* owner, std::shared_ptr<Window> window)
    : owner(owner),
      window(std::move(window)),
      window_id(this->window ? this->window->GetId() : 0),
      platform(std::make_unique<Platform>(this)) {}

DropTarget::Impl::~Impl() = default;

bool DropTarget::Impl::Register() {
  GtkWidget* widget = static_cast<GtkWidget*>(window->GetNativeObject());
  if (!widget || !GTK_IS_WIDGET(widget)) {
    return false;
  }
  // No GTK_DEST_DEFAULT_* flags: acceptance depends on the offered operation,
  // which the handlers decide.
  gtk_drag_dest_set(widget, static_cast<GtkDestDefaults>(0), nullptr, 0,
                    static_cast<GdkDragAction>(GDK_ACTION_COPY | GDK_ACTION_MOVE |
                                               GDK_ACTION_LINK));
  gtk_drag_dest_add_uri_targets(widget);
  GtkTargetList* targets = gtk_drag_dest_get_target_list(widget);
  gtk_target_list_add_text_targets(targets, kText);

  g_object_ref(widget);
  platform->widget = widget;
  auto* p = platform.get();
  p->handler_ids = {
      g_signal_connect(widget, "drag-motion", G_CALLBACK(&Platform::OnMotion), p),
      g_signal_connect(widget, "drag-leave", G_CALLBACK(&Platform::OnLeave), p),
      g_signal_connect(widget, "drag-drop", G_CALLBACK(&Platform::OnDrop), p),
      g_signal_connect(widget, "drag-data-received", G_CALLBACK(&Platform::OnDataReceived), p),
  };
  return true;
}

void DropTarget::Impl::Unregister() {
  platform->CancelPendingLeave();
  platform->inside = false;
  GtkWidget* widget = platform->widget;
  platform->widget = nullptr;
  if (!widget) {
    return;
  }
  for (gulong id : platform->handler_ids) {
    g_signal_handler_disconnect(widget, id);
  }
  platform->handler_ids.clear();
  gtk_drag_dest_unset(widget);
  g_object_unref(widget);
}

}  // namespace nativeapi
