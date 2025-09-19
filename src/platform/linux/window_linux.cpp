#include <iostream>
#include "../../window.h"
#include "../../window_manager.h"

// Import GTK headers
#include <gtk/gtk.h>
#include <gdk/gdk.h>

namespace nativeapi {

// Private implementation class
class Window::Impl {
 public:
  Impl(GdkWindow* window) : gdk_window_(window) {}
  GdkWindow* gdk_window_;
};

Window::Window() : pimpl_(new Impl(nullptr)) {
  id = -1;
}

Window::Window(void* window) : pimpl_(new Impl((GdkWindow*)window)) {
  // Use pointer address as ID since GDK doesn't provide direct window IDs
  id = pimpl_->gdk_window_ ? (WindowID)pimpl_->gdk_window_ : 0;
}

Window::~Window() {
  delete pimpl_;
}

void Window::Focus() {
  if (pimpl_->gdk_window_) {
    gdk_window_focus(pimpl_->gdk_window_, GDK_CURRENT_TIME);
  }
}

void Window::Blur() {
  if (pimpl_->gdk_window_) {
    gdk_window_lower(pimpl_->gdk_window_);
  }
}

bool Window::IsFocused() const {
  if (!pimpl_->gdk_window_) return false;
  // Check if this window is the focus window of its display
  GdkDisplay* display = gdk_window_get_display(pimpl_->gdk_window_);
  GdkSeat* seat = gdk_display_get_default_seat(display);
  if (seat) {
    GdkDevice* keyboard = gdk_seat_get_keyboard(seat);
    if (keyboard) {
      GdkWindow* focus_window = gdk_device_get_window_at_position(keyboard, nullptr, nullptr);
      return focus_window == pimpl_->gdk_window_;
    }
  }
  return false;
}

void Window::Show() {
  if (pimpl_->gdk_window_) {
    gdk_window_show(pimpl_->gdk_window_);
  }
}

void Window::ShowInactive() {
  if (pimpl_->gdk_window_) {
    gdk_window_show_unraised(pimpl_->gdk_window_);
  }
}

void Window::Hide() {
  if (pimpl_->gdk_window_) {
    gdk_window_hide(pimpl_->gdk_window_);
  }
}

bool Window::IsVisible() const {
  if (!pimpl_->gdk_window_) return false;
  return gdk_window_is_visible(pimpl_->gdk_window_);
}

void Window::Maximize() {
  if (pimpl_->gdk_window_) {
    gdk_window_maximize(pimpl_->gdk_window_);
  }
}

void Window::Unmaximize() {
  if (pimpl_->gdk_window_) {
    gdk_window_unmaximize(pimpl_->gdk_window_);
  }
}

bool Window::IsMaximized() const {
  if (!pimpl_->gdk_window_) return false;
  GdkWindowState state = gdk_window_get_state(pimpl_->gdk_window_);
  return state & GDK_WINDOW_STATE_MAXIMIZED;
}

void Window::Minimize() {
  if (pimpl_->gdk_window_) {
    gdk_window_iconify(pimpl_->gdk_window_);
  }
}

void Window::Restore() {
  if (pimpl_->gdk_window_) {
    gdk_window_deiconify(pimpl_->gdk_window_);
  }
}

bool Window::IsMinimized() const {
  if (!pimpl_->gdk_window_) return false;
  GdkWindowState state = gdk_window_get_state(pimpl_->gdk_window_);
  return state & GDK_WINDOW_STATE_ICONIFIED;
}

void Window::SetFullScreen(bool is_full_screen) {
  if (!pimpl_->gdk_window_) return;
  if (is_full_screen) {
    gdk_window_fullscreen(pimpl_->gdk_window_);
  } else {
    gdk_window_unfullscreen(pimpl_->gdk_window_);
  }
}

bool Window::IsFullScreen() const {
  if (!pimpl_->gdk_window_) return false;
  GdkWindowState state = gdk_window_get_state(pimpl_->gdk_window_);
  return state & GDK_WINDOW_STATE_FULLSCREEN;
}

void Window::SetBounds(Rectangle bounds) {
  if (pimpl_->gdk_window_) {
    gdk_window_move_resize(pimpl_->gdk_window_, 
                          (gint)bounds.x, (gint)bounds.y, 
                          (gint)bounds.width, (gint)bounds.height);
  }
}

Rectangle Window::GetBounds() const {
  Rectangle bounds = {0, 0, 0, 0};
  if (pimpl_->gdk_window_) {
    gint x, y, width, height;
    gdk_window_get_geometry(pimpl_->gdk_window_, &x, &y, &width, &height);
    bounds.x = x;
    bounds.y = y;
    bounds.width = width;
    bounds.height = height;
  }
  return bounds;
}

void Window::SetSize(Size size, bool animate) {
  if (pimpl_->gdk_window_) {
    gdk_window_resize(pimpl_->gdk_window_, (gint)size.width, (gint)size.height);
  }
}

Size Window::GetSize() const {
  Size size = {0, 0};
  if (pimpl_->gdk_window_) {
    gint width, height;
    gdk_window_get_geometry(pimpl_->gdk_window_, nullptr, nullptr, &width, &height);
    size.width = width;
    size.height = height;
  }
  return size;
}

void Window::SetContentSize(Size size) {
  // For GDK windows, content size is the same as window size
  SetSize(size, false);
}

Size Window::GetContentSize() const {
  // For GDK windows, content size is the same as window size
  return GetSize();
}

void Window::SetMinimumSize(Size size) {
  // GTK minimum size constraints would need to be set on the widget level
  // For now, we'll provide a basic implementation that doesn't enforce constraints
}

Size Window::GetMinimumSize() const {
  return Size{0, 0};
}

void Window::SetMaximumSize(Size size) {
  // GTK maximum size constraints would need to be set on the widget level
  // For now, we'll provide a basic implementation that doesn't enforce constraints
}

Size Window::GetMaximumSize() const {
  return Size{-1, -1}; // -1 indicates no maximum
}

void Window::SetResizable(bool is_resizable) {
  // This would typically be set at window creation time in GTK
  // For now, provide stub implementation
}

bool Window::IsResizable() const {
  return true; // Default assumption
}

void Window::SetMovable(bool is_movable) {
  // Window movability is typically a window manager property
  // Provide stub implementation
}

bool Window::IsMovable() const {
  return true; // Default assumption
}

void Window::SetMinimizable(bool is_minimizable) {
  // This would typically be set via window hints
  // Provide stub implementation
}

bool Window::IsMinimizable() const {
  return true; // Default assumption
}

void Window::SetMaximizable(bool is_maximizable) {
  // This would typically be set via window hints
  // Provide stub implementation
}

bool Window::IsMaximizable() const {
  return true; // Default assumption
}

void Window::SetFullScreenable(bool is_full_screenable) {
  // Provide stub implementation
}

bool Window::IsFullScreenable() const {
  return true; // Default assumption
}

void Window::SetClosable(bool is_closable) {
  // This would typically be set via window hints
  // Provide stub implementation
}

bool Window::IsClosable() const {
  return true; // Default assumption
}

void Window::SetAlwaysOnTop(bool is_always_on_top) {
  if (pimpl_->gdk_window_) {
    gdk_window_set_keep_above(pimpl_->gdk_window_, is_always_on_top);
  }
}

bool Window::IsAlwaysOnTop() const {
  if (!pimpl_->gdk_window_) return false;
  GdkWindowState state = gdk_window_get_state(pimpl_->gdk_window_);
  return state & GDK_WINDOW_STATE_ABOVE;
}

void Window::SetPosition(Point point) {
  if (pimpl_->gdk_window_) {
    gdk_window_move(pimpl_->gdk_window_, (gint)point.x, (gint)point.y);
  }
}

Point Window::GetPosition() const {
  Point point = {0, 0};
  if (pimpl_->gdk_window_) {
    gint x, y;
    gdk_window_get_position(pimpl_->gdk_window_, &x, &y);
    point.x = x;
    point.y = y;
  }
  return point;
}

void Window::SetTitle(std::string title) {
  // GDK windows don't have titles directly - this would be set on the GTK widget
  // For now, provide stub implementation
}

std::string Window::GetTitle() const {
  // GDK windows don't have titles directly - this would come from the GTK widget
  return std::string();
}

void Window::SetHasShadow(bool has_shadow) {
  // Window shadows are typically managed by the window manager
  // Provide stub implementation
}

bool Window::HasShadow() const {
  return true; // Default assumption
}

void Window::SetOpacity(float opacity) {
  if (pimpl_->gdk_window_) {
    gdk_window_set_opacity(pimpl_->gdk_window_, opacity);
  }
}

float Window::GetOpacity() const {
  // GDK doesn't provide a direct way to get opacity
  return 1.0f; // Default assumption
}

void Window::SetVisibleOnAllWorkspaces(bool is_visible_on_all_workspaces) {
  if (pimpl_->gdk_window_) {
    gdk_window_stick(pimpl_->gdk_window_);
  }
}

bool Window::IsVisibleOnAllWorkspaces() const {
  if (!pimpl_->gdk_window_) return false;
  GdkWindowState state = gdk_window_get_state(pimpl_->gdk_window_);
  return state & GDK_WINDOW_STATE_STICKY;
}

void Window::SetIgnoreMouseEvents(bool is_ignore_mouse_events) {
  // This would involve setting input shapes or event masks
  // Provide stub implementation
}

bool Window::IsIgnoreMouseEvents() const {
  return false; // Default assumption
}

void Window::SetFocusable(bool is_focusable) {
  // This would typically be set via window hints
  // Provide stub implementation
}

bool Window::IsFocusable() const {
  return true; // Default assumption
}

void Window::StartDragging() {
  // Window dragging would typically involve listening to mouse events
  // Provide stub implementation
}

void Window::StartResizing() {
  // Window resizing would typically involve listening to mouse events at edges
  // Provide stub implementation
}

void* Window::GetNSWindow() const {
  // This method is macOS-specific, return nullptr on Linux
  return nullptr;
}

}  // namespace nativeapi