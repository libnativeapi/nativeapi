#include <windows.h>
#include <iostream>
#include "../../window.h"
#include "../../window_manager.h"

namespace nativeapi {

// Private implementation class
class Window::Impl {
 public:
  Impl(HWND hwnd) : hwnd_(hwnd) {}
  HWND hwnd_;
};

Window::Window() : pimpl_(std::make_unique<Impl>(nullptr)) {
}

Window::Window(void* window) : pimpl_(std::make_unique<Impl>(static_cast<HWND>(window))) {
}

Window::~Window() {

}

void Window::Focus() {
  if (pimpl_->hwnd_) {
    SetForegroundWindow(pimpl_->hwnd_);
    SetFocus(pimpl_->hwnd_);
  }
}

void Window::Blur() {
  if (pimpl_->hwnd_) {
    SetFocus(nullptr);
  }
}

bool Window::IsFocused() const {
  return pimpl_->hwnd_ && GetForegroundWindow() == pimpl_->hwnd_;
}

void Window::Show() {
  if (pimpl_->hwnd_) {
    ShowWindow(pimpl_->hwnd_, SW_SHOW);
    SetForegroundWindow(pimpl_->hwnd_);
  }
}

void Window::ShowInactive() {
  if (pimpl_->hwnd_) {
    ShowWindow(pimpl_->hwnd_, SW_SHOWNOACTIVATE);
  }
}

void Window::Hide() {
  if (pimpl_->hwnd_) {
    ShowWindow(pimpl_->hwnd_, SW_HIDE);
  }
}

bool Window::IsVisible() const {
  return pimpl_->hwnd_ && IsWindowVisible(pimpl_->hwnd_);
}

void Window::Maximize() {
  if (pimpl_->hwnd_ && !IsMaximized()) {
    ShowWindow(pimpl_->hwnd_, SW_MAXIMIZE);
  }
}

void Window::Unmaximize() {
  if (pimpl_->hwnd_ && IsMaximized()) {
    ShowWindow(pimpl_->hwnd_, SW_RESTORE);
  }
}

bool Window::IsMaximized() const {
  if (!pimpl_->hwnd_) return false;
  WINDOWPLACEMENT wp = {};
  wp.length = sizeof(WINDOWPLACEMENT);
  GetWindowPlacement(pimpl_->hwnd_, &wp);
  return wp.showCmd == SW_MAXIMIZE;
}

void Window::Minimize() {
  if (pimpl_->hwnd_ && !IsMinimized()) {
    ShowWindow(pimpl_->hwnd_, SW_MINIMIZE);
  }
}

void Window::Restore() {
  if (pimpl_->hwnd_ && IsMinimized()) {
    ShowWindow(pimpl_->hwnd_, SW_RESTORE);
  }
}

bool Window::IsMinimized() const {
  if (!pimpl_->hwnd_) return false;
  WINDOWPLACEMENT wp = {};
  wp.length = sizeof(WINDOWPLACEMENT);
  GetWindowPlacement(pimpl_->hwnd_, &wp);
  return wp.showCmd == SW_MINIMIZE;
}

void Window::SetFullScreen(bool is_full_screen) {
  if (!pimpl_->hwnd_) return;
  
  static WINDOWPLACEMENT g_wpPrev = { sizeof(g_wpPrev) };
  static DWORD g_dwStyle = 0;
  static DWORD g_dwExStyle = 0;
  
  if (is_full_screen) {
    if (!IsFullScreen()) {
      // Save current window placement and style
      GetWindowPlacement(pimpl_->hwnd_, &g_wpPrev);
      g_dwStyle = GetWindowLong(pimpl_->hwnd_, GWL_STYLE);
      g_dwExStyle = GetWindowLong(pimpl_->hwnd_, GWL_EXSTYLE);
      
      // Remove window decorations
      SetWindowLong(pimpl_->hwnd_, GWL_STYLE, g_dwStyle & ~(WS_CAPTION | WS_THICKFRAME));
      SetWindowLong(pimpl_->hwnd_, GWL_EXSTYLE, g_dwExStyle & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));
      
      // Get monitor info
      MONITORINFO mi = { sizeof(mi) };
      GetMonitorInfo(MonitorFromWindow(pimpl_->hwnd_, MONITOR_DEFAULTTONEAREST), &mi);
      
      // Set window to cover entire monitor
      SetWindowPos(pimpl_->hwnd_, nullptr, mi.rcMonitor.left, mi.rcMonitor.top,
                   mi.rcMonitor.right - mi.rcMonitor.left,
                   mi.rcMonitor.bottom - mi.rcMonitor.top,
                   SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    }
  } else {
    if (IsFullScreen()) {
      // Restore window style and placement
      SetWindowLong(pimpl_->hwnd_, GWL_STYLE, g_dwStyle);
      SetWindowLong(pimpl_->hwnd_, GWL_EXSTYLE, g_dwExStyle);
      SetWindowPlacement(pimpl_->hwnd_, &g_wpPrev);
      SetWindowPos(pimpl_->hwnd_, nullptr, 0, 0, 0, 0,
                   SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
  }
}

bool Window::IsFullScreen() const {
  if (!pimpl_->hwnd_) return false;
  
  RECT windowRect, monitorRect;
  GetWindowRect(pimpl_->hwnd_, &windowRect);
  
  MONITORINFO mi = { sizeof(mi) };
  GetMonitorInfo(MonitorFromWindow(pimpl_->hwnd_, MONITOR_DEFAULTTONEAREST), &mi);
  monitorRect = mi.rcMonitor;
  
  return (windowRect.left == monitorRect.left &&
          windowRect.top == monitorRect.top &&
          windowRect.right == monitorRect.right &&
          windowRect.bottom == monitorRect.bottom);
}

void Window::SetBounds(Rectangle bounds) {
  if (pimpl_->hwnd_) {
    SetWindowPos(pimpl_->hwnd_, nullptr,
                 static_cast<int>(bounds.x), static_cast<int>(bounds.y),
                 static_cast<int>(bounds.width), static_cast<int>(bounds.height),
                 SWP_NOZORDER);
  }
}

Rectangle Window::GetBounds() const {
  Rectangle bounds = {0, 0, 0, 0};
  if (pimpl_->hwnd_) {
    RECT rect;
    GetWindowRect(pimpl_->hwnd_, &rect);
    bounds.x = rect.left;
    bounds.y = rect.top;
    bounds.width = rect.right - rect.left;
    bounds.height = rect.bottom - rect.top;
  }
  return bounds;
}

void Window::SetSize(Size size, bool animate) {
  if (pimpl_->hwnd_) {
    // Windows doesn't have built-in animation for window resizing
    // Animation would require custom implementation
    SetWindowPos(pimpl_->hwnd_, nullptr, 0, 0,
                 static_cast<int>(size.width), static_cast<int>(size.height),
                 SWP_NOMOVE | SWP_NOZORDER);
  }
}

Size Window::GetSize() const {
  Size size = {0, 0};
  if (pimpl_->hwnd_) {
    RECT rect;
    GetWindowRect(pimpl_->hwnd_, &rect);
    size.width = rect.right - rect.left;
    size.height = rect.bottom - rect.top;
  }
  return size;
}

void Window::SetContentSize(Size size) {
  if (pimpl_->hwnd_) {
    RECT windowRect, clientRect;
    GetWindowRect(pimpl_->hwnd_, &windowRect);
    GetClientRect(pimpl_->hwnd_, &clientRect);
    
    // Calculate the difference between window and client area
    int borderWidth = (windowRect.right - windowRect.left) - clientRect.right;
    int borderHeight = (windowRect.bottom - windowRect.top) - clientRect.bottom;
    
    SetWindowPos(pimpl_->hwnd_, nullptr, 0, 0,
                 static_cast<int>(size.width) + borderWidth,
                 static_cast<int>(size.height) + borderHeight,
                 SWP_NOMOVE | SWP_NOZORDER);
  }
}

Size Window::GetContentSize() const {
  Size size = {0, 0};
  if (pimpl_->hwnd_) {
    RECT rect;
    GetClientRect(pimpl_->hwnd_, &rect);
    size.width = rect.right;
    size.height = rect.bottom;
  }
  return size;
}

void Window::SetMinimumSize(Size size) {
  // Windows minimum size would be handled in WM_GETMINMAXINFO message
  // This is a placeholder implementation
}

Size Window::GetMinimumSize() const {
  // Return default minimum size
  return {0, 0};
}

void Window::SetMaximumSize(Size size) {
  // Windows maximum size would be handled in WM_GETMINMAXINFO message
  // This is a placeholder implementation
}

Size Window::GetMaximumSize() const {
  // Return default maximum size (screen size)
  return {static_cast<double>(GetSystemMetrics(SM_CXSCREEN)), static_cast<double>(GetSystemMetrics(SM_CYSCREEN))};
}

void Window::SetResizable(bool is_resizable) {
  if (pimpl_->hwnd_) {
    LONG style = GetWindowLong(pimpl_->hwnd_, GWL_STYLE);
    if (is_resizable) {
      style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
    } else {
      style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
    }
    SetWindowLong(pimpl_->hwnd_, GWL_STYLE, style);
    SetWindowPos(pimpl_->hwnd_, nullptr, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
  }
}

bool Window::IsResizable() const {
  if (!pimpl_->hwnd_) return false;
  LONG style = GetWindowLong(pimpl_->hwnd_, GWL_STYLE);
  return (style & WS_THICKFRAME) != 0;
}

void Window::SetMovable(bool is_movable) {
  // Windows doesn't have a direct way to disable window movement
  // This would require custom window procedure handling
}

bool Window::IsMovable() const {
  // Windows windows are movable by default
  return true;
}

void Window::SetMinimizable(bool is_minimizable) {
  if (pimpl_->hwnd_) {
    LONG style = GetWindowLong(pimpl_->hwnd_, GWL_STYLE);
    if (is_minimizable) {
      style |= WS_MINIMIZEBOX;
    } else {
      style &= ~WS_MINIMIZEBOX;
    }
    SetWindowLong(pimpl_->hwnd_, GWL_STYLE, style);
    SetWindowPos(pimpl_->hwnd_, nullptr, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
  }
}

bool Window::IsMinimizable() const {
  if (!pimpl_->hwnd_) return false;
  LONG style = GetWindowLong(pimpl_->hwnd_, GWL_STYLE);
  return (style & WS_MINIMIZEBOX) != 0;
}

void Window::SetMaximizable(bool is_maximizable) {
  if (pimpl_->hwnd_) {
    LONG style = GetWindowLong(pimpl_->hwnd_, GWL_STYLE);
    if (is_maximizable) {
      style |= WS_MAXIMIZEBOX;
    } else {
      style &= ~WS_MAXIMIZEBOX;
    }
    SetWindowLong(pimpl_->hwnd_, GWL_STYLE, style);
    SetWindowPos(pimpl_->hwnd_, nullptr, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
  }
}

bool Window::IsMaximizable() const {
  if (!pimpl_->hwnd_) return false;
  LONG style = GetWindowLong(pimpl_->hwnd_, GWL_STYLE);
  return (style & WS_MAXIMIZEBOX) != 0;
}

void Window::SetFullScreenable(bool is_full_screenable) {
  // This is a concept more relevant to macOS
  // On Windows, any window can potentially go fullscreen
}

bool Window::IsFullScreenable() const {
  return true; // All Windows windows can go fullscreen
}

void Window::SetClosable(bool is_closable) {
  if (pimpl_->hwnd_) {
    LONG style = GetWindowLong(pimpl_->hwnd_, GWL_STYLE);
    if (is_closable) {
      style |= WS_SYSMENU;
    } else {
      style &= ~WS_SYSMENU;
    }
    SetWindowLong(pimpl_->hwnd_, GWL_STYLE, style);
    SetWindowPos(pimpl_->hwnd_, nullptr, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
  }
}

bool Window::IsClosable() const {
  if (!pimpl_->hwnd_) return false;
  LONG style = GetWindowLong(pimpl_->hwnd_, GWL_STYLE);
  return (style & WS_SYSMENU) != 0;
}

void Window::SetAlwaysOnTop(bool is_always_on_top) {
  if (pimpl_->hwnd_) {
    SetWindowPos(pimpl_->hwnd_, is_always_on_top ? HWND_TOPMOST : HWND_NOTOPMOST,
                 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
  }
}

bool Window::IsAlwaysOnTop() const {
  if (!pimpl_->hwnd_) return false;
  LONG exStyle = GetWindowLong(pimpl_->hwnd_, GWL_EXSTYLE);
  return (exStyle & WS_EX_TOPMOST) != 0;
}

void Window::SetPosition(Point point) {
  if (pimpl_->hwnd_) {
    SetWindowPos(pimpl_->hwnd_, nullptr,
                 static_cast<int>(point.x), static_cast<int>(point.y),
                 0, 0, SWP_NOSIZE | SWP_NOZORDER);
  }
}

Point Window::GetPosition() const {
  Point point = {0, 0};
  if (pimpl_->hwnd_) {
    RECT rect;
    GetWindowRect(pimpl_->hwnd_, &rect);
    point.x = rect.left;
    point.y = rect.top;
  }
  return point;
}

void Window::SetTitle(std::string title) {
  if (pimpl_->hwnd_) {
    SetWindowText(pimpl_->hwnd_, title.c_str());
  }
}

std::string Window::GetTitle() const {
  if (!pimpl_->hwnd_) return "";
  
  int length = GetWindowTextLength(pimpl_->hwnd_);
  if (length == 0) return "";
  
  std::string title(length, '\0');
  GetWindowText(pimpl_->hwnd_, &title[0], length + 1);
  return title;
}

void Window::SetHasShadow(bool has_shadow) {
  // Windows shadow is typically handled automatically
  // Custom shadow implementation would be complex
}

bool Window::HasShadow() const {
  return true; // Windows typically have shadows by default
}

void Window::SetOpacity(float opacity) {
  if (pimpl_->hwnd_) {
    LONG exStyle = GetWindowLong(pimpl_->hwnd_, GWL_EXSTYLE);
    
    if (opacity < 1.0f) {
      // Enable layered window and set opacity
      SetWindowLong(pimpl_->hwnd_, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
      SetLayeredWindowAttributes(pimpl_->hwnd_, 0, static_cast<BYTE>(opacity * 255), LWA_ALPHA);
    } else {
      // Disable layered window
      SetWindowLong(pimpl_->hwnd_, GWL_EXSTYLE, exStyle & ~WS_EX_LAYERED);
    }
  }
}

float Window::GetOpacity() const {
  if (!pimpl_->hwnd_) return 1.0f;
  
  LONG exStyle = GetWindowLong(pimpl_->hwnd_, GWL_EXSTYLE);
  if (exStyle & WS_EX_LAYERED) {
    BYTE alpha;
    if (GetLayeredWindowAttributes(pimpl_->hwnd_, nullptr, &alpha, nullptr)) {
      return alpha / 255.0f;
    }
  }
  return 1.0f;
}

void Window::SetVisibleOnAllWorkspaces(bool is_visible_on_all_workspaces) {
  // Windows doesn't have the same concept of workspaces as macOS
  // This would require integration with virtual desktop APIs
}

bool Window::IsVisibleOnAllWorkspaces() const {
  return false; // Not supported on Windows by default
}

void Window::SetIgnoreMouseEvents(bool is_ignore_mouse_events) {
  if (pimpl_->hwnd_) {
    LONG exStyle = GetWindowLong(pimpl_->hwnd_, GWL_EXSTYLE);
    if (is_ignore_mouse_events) {
      exStyle |= WS_EX_TRANSPARENT;
    } else {
      exStyle &= ~WS_EX_TRANSPARENT;
    }
    SetWindowLong(pimpl_->hwnd_, GWL_EXSTYLE, exStyle);
  }
}

bool Window::IsIgnoreMouseEvents() const {
  if (!pimpl_->hwnd_) return false;
  LONG exStyle = GetWindowLong(pimpl_->hwnd_, GWL_EXSTYLE);
  return (exStyle & WS_EX_TRANSPARENT) != 0;
}

void Window::SetFocusable(bool is_focusable) {
  // Windows focusability is typically controlled by window style
  // This is a simplified implementation
}

bool Window::IsFocusable() const {
  if (!pimpl_->hwnd_) return false;
  LONG style = GetWindowLong(pimpl_->hwnd_, GWL_STYLE);
  return (style & WS_DISABLED) == 0;
}

void Window::StartDragging() {
  if (pimpl_->hwnd_) {
    // Simulate dragging by sending WM_NCLBUTTONDOWN with HTCAPTION
    PostMessage(pimpl_->hwnd_, WM_NCLBUTTONDOWN, HTCAPTION, 0);
  }
}

void Window::StartResizing() {
  // Windows doesn't have a direct API to start resizing programmatically
  // This would require more complex implementation
}

WindowID Window::GetId() const {
  return pimpl_ && pimpl_->hwnd_ ? reinterpret_cast<WindowID>(pimpl_->hwnd_) : -1;
}

void* Window::GetNativeObjectInternal() const {
  return pimpl_ ? reinterpret_cast<void*>(pimpl_->hwnd_) : nullptr;
}

}  // namespace nativeapi
