#include "../../window_drag_session.h"

#include <gdk/gdk.h>
#include <gtk/gtk.h>

namespace nativeapi {

class WindowDragSession::Impl {
 public:
  guint timer_source_id_ = 0;

  static gboolean OnTimer(gpointer user_data) {
    static_cast<WindowDragSession*>(user_data)->HandleTick();
    return G_SOURCE_CONTINUE;
  }
};

WindowDragSession::WindowDragSession() : pimpl_(std::make_unique<Impl>()) {}

WindowDragSession::~WindowDragSession() {
  active_ = false;
  StopTicking();
}

void WindowDragSession::StartTicking() {
  if (pimpl_->timer_source_id_ != 0) {
    return;
  }
  pimpl_->timer_source_id_ = g_timeout_add(8, &Impl::OnTimer, this);
}

void WindowDragSession::StopTicking() {
  if (pimpl_->timer_source_id_ == 0) {
    return;
  }
  // Safe from inside OnTimer: the source is destroyed once the callback returns.
  g_source_remove(pimpl_->timer_source_id_);
  pimpl_->timer_source_id_ = 0;
}

bool WindowDragSession::QueryPointer(Point& position, bool& primary_button_down) const {
  GdkDisplay* display = gdk_display_get_default();
  if (!display) {
    return false;
  }
  GdkSeat* seat = gdk_display_get_default_seat(display);
  GdkDevice* pointer = seat ? gdk_seat_get_pointer(seat) : nullptr;
  GdkWindow* root = gdk_get_default_root_window();
  if (!pointer || !root) {
    return false;
  }

  // Relative to the root window this is the global position and button state
  // on X11. Wayland does not expose either outside the application's own
  // surfaces, which shows up as the button reading released.
  gint x = 0;
  gint y = 0;
  GdkModifierType mask = static_cast<GdkModifierType>(0);
  gdk_window_get_device_position(root, pointer, &x, &y, &mask);
  position = {static_cast<double>(x), static_cast<double>(y)};
  primary_button_down = (mask & GDK_BUTTON1_MASK) != 0;
  return true;
}

void WindowDragSession::MoveWindow(Window& window, Point cursor_position) const {
  const gint x = static_cast<gint>(cursor_position.x - anchor_.x);
  const gint y = static_cast<gint>(cursor_position.y - anchor_.y);
  GtkWidget* widget = static_cast<GtkWidget*>(window.GetNativeObject());
  if (widget && GTK_IS_WINDOW(widget)) {
    // gtk_window_move() positions the frame, including decorations, which is
    // what the anchor is relative to.
    gtk_window_move(GTK_WINDOW(widget), x, y);
    return;
  }
  window.SetPosition({static_cast<double>(x), static_cast<double>(y)});
}

}  // namespace nativeapi
