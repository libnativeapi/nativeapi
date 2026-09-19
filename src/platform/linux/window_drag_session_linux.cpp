#include "../../window_drag_session.h"

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#ifdef GDK_WINDOWING_WAYLAND
#include <gdk/gdkwayland.h>
#endif

namespace nativeapi {

class WindowDragSession::Impl {
 public:
  // The X server reports the primary button as released for a moment after a press
  // that also moves the keyboard focus — 10 to 20 ms on GNOME 46 through Xwayland,
  // measured 2026-09-18. A session started by such a press (a tear-off recognized from
  // a focus change) would end on its first tick, so a released button is only believed
  // once the button has been seen down, or once that moment has passed.
  static constexpr gint64 kButtonSettleUs = 60 * 1000;

  guint timer_source_id_ = 0;
  gint64 ticking_since_us_ = 0;
  bool seen_button_down_ = false;

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
  pimpl_->ticking_since_us_ = g_get_monotonic_time();
  pimpl_->seen_button_down_ = false;
  pimpl_->timer_source_id_ = g_timeout_add(8, &Impl::OnTimer, this);
}

void WindowDragSession::StopTicking() {
  if (pimpl_->timer_source_id_ == 0) {
    return;
  }
  // Safe from inside OnTimer: the source is destroyed once the callback returns.
  g_source_remove(pimpl_->timer_source_id_);
  pimpl_->timer_source_id_ = 0;
  pimpl_->ticking_since_us_ = 0;
}

bool WindowDragSession::QueryPointer(Point& position, bool& primary_button_down) const {
  GdkDisplay* display = gdk_display_get_default();
  if (!display) {
    return false;
  }
#ifdef GDK_WINDOWING_WAYLAND
  // A Wayland client is not told where the pointer is, whether a button is held
  // outside its own surfaces, or where its windows are — and it cannot move them.
  // Nothing here can be made to work, so the session refuses to start rather than
  // following a cursor that always reads (0, 0). An app that needs this on a Wayland
  // session runs under Xwayland (GDK_BACKEND=x11).
  if (GDK_IS_WAYLAND_DISPLAY(display)) {
    return false;
  }
#endif
  GdkSeat* seat = gdk_display_get_default_seat(display);
  GdkDevice* pointer = seat ? gdk_seat_get_pointer(seat) : nullptr;
  GdkWindow* root = gdk_get_default_root_window();
  if (!pointer || !root) {
    return false;
  }

  // Relative to the root window this is the global position and button state on X11.
  gint x = 0;
  gint y = 0;
  GdkModifierType mask = static_cast<GdkModifierType>(0);
  gdk_window_get_device_position(root, pointer, &x, &y, &mask);
  position = {static_cast<double>(x), static_cast<double>(y)};

  bool button_down = (mask & GDK_BUTTON1_MASK) != 0;
  if (button_down) {
    pimpl_->seen_button_down_ = true;
  } else if (!pimpl_->seen_button_down_ && pimpl_->ticking_since_us_ != 0 &&
             g_get_monotonic_time() - pimpl_->ticking_since_us_ < Impl::kButtonSettleUs) {
    button_down = true;  // too early to believe it, see kButtonSettleUs
  }
  primary_button_down = button_down;
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
