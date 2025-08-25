#include <windows.h>
#include <dwmapi.h>
#include <iostream>
#include <string>
#include "../../window.h"

#pragma comment(lib, "dwmapi.lib")

namespace nativeapi {

// Helper function to convert string to wide string
std::wstring StringToWideString(const std::string& str) {
    if (str.empty()) return std::wstring();
    
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), nullptr, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

// Helper function to convert wide string to string
std::string WideStringToString(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, nullptr, nullptr);
    return strTo;
}

// Private implementation class
class Window::Impl {
public:
    Impl(HWND hwnd) : hwnd_(hwnd) {}
    
    HWND hwnd_;
    WINDOWPLACEMENT saved_placement_ = {};
    bool placement_saved_ = false;
};

Window::Window() : pimpl_(new Impl(nullptr)) {
    id = -1;
}

Window::Window(void* window) : pimpl_(new Impl(static_cast<HWND>(window))) {
    id = reinterpret_cast<WindowID>(pimpl_->hwnd_);
}

Window::~Window() {
    delete pimpl_;
}

void Window::Focus() {
    if (pimpl_->hwnd_) {
        SetForegroundWindow(pimpl_->hwnd_);
        SetFocus(pimpl_->hwnd_);
    }
}

void Window::Blur() {
    if (pimpl_->hwnd_) {
        SetWindowPos(pimpl_->hwnd_, HWND_BOTTOM, 0, 0, 0, 0, 
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
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
    
    WINDOWPLACEMENT placement;
    placement.length = sizeof(WINDOWPLACEMENT);
    if (GetWindowPlacement(pimpl_->hwnd_, &placement)) {
        return placement.showCmd == SW_SHOWMAXIMIZED;
    }
    return false;
}

void Window::Minimize() {
    if (pimpl_->hwnd_ && !IsMinimized()) {
        ShowWindow(pimpl_->hwnd_, SW_MINIMIZE);
    }
}

void Window::Restore() {
    if (pimpl_->hwnd_) {
        ShowWindow(pimpl_->hwnd_, SW_RESTORE);
    }
}

bool Window::IsMinimized() const {
    if (!pimpl_->hwnd_) return false;
    
    WINDOWPLACEMENT placement;
    placement.length = sizeof(WINDOWPLACEMENT);
    if (GetWindowPlacement(pimpl_->hwnd_, &placement)) {
        return placement.showCmd == SW_SHOWMINIMIZED;
    }
    return false;
}

void Window::SetFullScreen(bool is_full_screen) {
    if (!pimpl_->hwnd_) return;
    
    if (is_full_screen && !IsFullScreen()) {
        // Save current window placement
        pimpl_->saved_placement_.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(pimpl_->hwnd_, &pimpl_->saved_placement_);
        pimpl_->placement_saved_ = true;
        
        // Get monitor info
        HMONITOR hmonitor = MonitorFromWindow(pimpl_->hwnd_, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi = { sizeof(mi) };
        if (GetMonitorInfo(hmonitor, &mi)) {
            // Remove window decorations
            SetWindowLong(pimpl_->hwnd_, GWL_STYLE, WS_POPUP | WS_VISIBLE);
            SetWindowPos(pimpl_->hwnd_, HWND_TOP,
                        mi.rcMonitor.left, mi.rcMonitor.top,
                        mi.rcMonitor.right - mi.rcMonitor.left,
                        mi.rcMonitor.bottom - mi.rcMonitor.top,
                        SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    } else if (!is_full_screen && IsFullScreen()) {
        // Restore window decorations
        SetWindowLong(pimpl_->hwnd_, GWL_STYLE, WS_OVERLAPPEDWINDOW);
        
        // Restore previous position and size
        if (pimpl_->placement_saved_) {
            SetWindowPlacement(pimpl_->hwnd_, &pimpl_->saved_placement_);
            pimpl_->placement_saved_ = false;
        }
        
        SetWindowPos(pimpl_->hwnd_, nullptr, 0, 0, 0, 0,
                    SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                    SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

bool Window::IsFullScreen() const {
    if (!pimpl_->hwnd_) return false;
    
    LONG style = GetWindowLong(pimpl_->hwnd_, GWL_STYLE);
    return (style & WS_POPUP) && !(style & WS_OVERLAPPEDWINDOW);
}

void Window::SetBounds(Rectangle bounds) {
    if (pimpl_->hwnd_) {
        SetWindowPos(pimpl_->hwnd_, nullptr,
                    static_cast<int>(bounds.x), static_cast<int>(bounds.y),
                    static_cast<int>(bounds.width), static_cast<int>(bounds.height),
                    SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

Rectangle Window::GetBounds() const {
    Rectangle bounds = {0, 0, 0, 0};
    
    if (pimpl_->hwnd_) {
        RECT rect;
        if (GetWindowRect(pimpl_->hwnd_, &rect)) {
            bounds.x = static_cast<double>(rect.left);
            bounds.y = static_cast<double>(rect.top);
            bounds.width = static_cast<double>(rect.right - rect.left);
            bounds.height = static_cast<double>(rect.bottom - rect.top);
        }
    }
    
    return bounds;
}

void Window::SetSize(Size size, bool animate) {
    if (pimpl_->hwnd_) {
        RECT rect;
        if (GetWindowRect(pimpl_->hwnd_, &rect)) {
            if (animate) {
                // Simple animation simulation - in practice you might want smoother animation
                UINT flags = SWP_NOZORDER | SWP_NOACTIVATE;
                SetWindowPos(pimpl_->hwnd_, nullptr, rect.left, rect.top,
                            static_cast<int>(size.width), static_cast<int>(size.height), flags);
            } else {
                SetWindowPos(pimpl_->hwnd_, nullptr, rect.left, rect.top,
                            static_cast<int>(size.width), static_cast<int>(size.height),
                            SWP_NOZORDER | SWP_NOACTIVATE);
            }
        }
    }
}

Size Window::GetSize() const {
    Size size = {0, 0};
    
    if (pimpl_->hwnd_) {
        RECT rect;
        if (GetWindowRect(pimpl_->hwnd_, &rect)) {
            size.width = static_cast<double>(rect.right - rect.left);
            size.height = static_cast<double>(rect.bottom - rect.top);
        }
    }
    
    return size;
}

void Window::SetContentSize(Size size) {
    if (pimpl_->hwnd_) {
        RECT windowRect, clientRect;
        if (GetWindowRect(pimpl_->hwnd_, &windowRect) && GetClientRect(pimpl_->hwnd_, &clientRect)) {
            int borderWidth = (windowRect.right - windowRect.left) - (clientRect.right - clientRect.left);
            int borderHeight = (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top);
            
            SetWindowPos(pimpl_->hwnd_, nullptr, windowRect.left, windowRect.top,
                        static_cast<int>(size.width) + borderWidth,
                        static_cast<int>(size.height) + borderHeight,
                        SWP_NOZORDER | SWP_NOACTIVATE);
        }
    }
}

Size Window::GetContentSize() const {
    Size size = {0, 0};
    
    if (pimpl_->hwnd_) {
        RECT rect;
        if (GetClientRect(pimpl_->hwnd_, &rect)) {
            size.width = static_cast<double>(rect.right - rect.left);
            size.height = static_cast<double>(rect.bottom - rect.top);
        }
    }
    
    return size;
}

void Window::SetMinimumSize(Size size) {
    // Windows doesn't have a direct API for this, would need to handle WM_GETMINMAXINFO
    // This is a placeholder implementation
}

Size Window::GetMinimumSize() {
    // Placeholder implementation
    return Size{0, 0};
}

void Window::SetMaximumSize(Size size) {
    // Windows doesn't have a direct API for this, would need to handle WM_GETMINMAXINFO
    // This is a placeholder implementation
}

Size Window::GetMaximumSize() {
    // Placeholder implementation
    return Size{0, 0};
}

void Window::SetResizable(bool is_resizable) {
    if (pimpl_->hwnd_) {
        LONG style = GetWindowLong(pimpl_->hwnd_, GWL_STYLE);
        if (is_resizable) {
            style |= WS_THICKFRAME;
        } else {
            style &= ~WS_THICKFRAME;
        }
        SetWindowLong(pimpl_->hwnd_, GWL_STYLE, style);
        SetWindowPos(pimpl_->hwnd_, nullptr, 0, 0, 0, 0,
                    SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }
}

bool Window::IsResizable() const {
    if (pimpl_->hwnd_) {
        LONG style = GetWindowLong(pimpl_->hwnd_, GWL_STYLE);
        return (style & WS_THICKFRAME) != 0;
    }
    return false;
}

void Window::SetMovable(bool is_movable) {
    // Windows doesn't have a direct equivalent, this would require message handling
    // Placeholder implementation
}

bool Window::IsMovable() const {
    // Most Windows windows are movable by default
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
    if (pimpl_->hwnd_) {
        LONG style = GetWindowLong(pimpl_->hwnd_, GWL_STYLE);
        return (style & WS_MINIMIZEBOX) != 0;
    }
    return false;
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
    if (pimpl_->hwnd_) {
        LONG style = GetWindowLong(pimpl_->hwnd_, GWL_STYLE);
        return (style & WS_MAXIMIZEBOX) != 0;
    }
    return false;
}

void Window::SetFullScreenable(bool is_full_screenable) {
    // Windows doesn't have a specific fullscreen capability flag
    // This is handled by the SetFullScreen method
}

bool Window::IsFullScreenable() const {
    // Most windows can be made fullscreen
    return true;
}

void Window::SetClosable(bool is_closable) {
    if (pimpl_->hwnd_) {
        HMENU hMenu = GetSystemMenu(pimpl_->hwnd_, FALSE);
        if (hMenu) {
            if (is_closable) {
                EnableMenuItem(hMenu, SC_CLOSE, MF_ENABLED);
            } else {
                EnableMenuItem(hMenu, SC_CLOSE, MF_GRAYED);
            }
        }
    }
}

bool Window::IsClosable() const {
    if (pimpl_->hwnd_) {
        HMENU hMenu = GetSystemMenu(pimpl_->hwnd_, FALSE);
        if (hMenu) {
            MENUITEMINFO mii = {};
            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_STATE;
            if (GetMenuItemInfo(hMenu, SC_CLOSE, FALSE, &mii)) {
                return !(mii.fState & MFS_GRAYED);
            }
        }
    }
    return true;
}

void Window::SetAlwaysOnTop(bool is_always_on_top) {
    if (pimpl_->hwnd_) {
        SetWindowPos(pimpl_->hwnd_, 
                    is_always_on_top ? HWND_TOPMOST : HWND_NOTOPMOST,
                    0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
}

bool Window::IsAlwaysOnTop() {
    if (pimpl_->hwnd_) {
        LONG_PTR exStyle = GetWindowLongPtr(pimpl_->hwnd_, GWL_EXSTYLE);
        return (exStyle & WS_EX_TOPMOST) != 0;
    }
    return false;
}

void Window::SetPosition(Point point) {
    if (pimpl_->hwnd_) {
        SetWindowPos(pimpl_->hwnd_, nullptr,
                    static_cast<int>(point.x), static_cast<int>(point.y),
                    0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

Point Window::GetPosition() {
    Point point = {0, 0};
    
    if (pimpl_->hwnd_) {
        RECT rect;
        if (GetWindowRect(pimpl_->hwnd_, &rect)) {
            point.x = static_cast<double>(rect.left);
            point.y = static_cast<double>(rect.top);
        }
    }
    
    return point;
}

void Window::SetTitle(std::string title) {
    if (pimpl_->hwnd_) {
        std::wstring wtitle = StringToWideString(title);
        SetWindowText(pimpl_->hwnd_, wtitle.c_str());
    }
}

std::string Window::GetTitle() {
    if (pimpl_->hwnd_) {
        int length = GetWindowTextLength(pimpl_->hwnd_);
        if (length > 0) {
            std::wstring wtitle(length + 1, L'\0');
            GetWindowText(pimpl_->hwnd_, &wtitle[0], length + 1);
            wtitle.resize(length);
            return WideStringToString(wtitle);
        }
    }
    return std::string();
}

void Window::SetHasShadow(bool has_shadow) {
    // Windows handles shadows automatically, but we can modify the window attributes
    if (pimpl_->hwnd_) {
        LONG_PTR style = GetWindowLongPtr(pimpl_->hwnd_, GWL_EXSTYLE);
        if (has_shadow) {
            style &= ~WS_EX_NOACTIVATE;  // Enable shadow
        } else {
            style |= WS_EX_NOACTIVATE;   // This can reduce shadow in some cases
        }
        SetWindowLongPtr(pimpl_->hwnd_, GWL_EXSTYLE, style);
    }
}

bool Window::HasShadow() const {
    // Windows typically shows shadows for most windows
    return true;
}

void Window::SetOpacity(float opacity) {
    if (pimpl_->hwnd_) {
        LONG_PTR exStyle = GetWindowLongPtr(pimpl_->hwnd_, GWL_EXSTYLE);
        exStyle |= WS_EX_LAYERED;
        SetWindowLongPtr(pimpl_->hwnd_, GWL_EXSTYLE, exStyle);
        
        BYTE alpha = static_cast<BYTE>(opacity * 255);
        SetLayeredWindowAttributes(pimpl_->hwnd_, 0, alpha, LWA_ALPHA);
    }
}

float Window::GetOpacity() const {
    if (pimpl_->hwnd_) {
        BYTE alpha;
        COLORREF colorKey;
        DWORD flags;
        if (GetLayeredWindowAttributes(pimpl_->hwnd_, &colorKey, &alpha, &flags)) {
            if (flags & LWA_ALPHA) {
                return static_cast<float>(alpha) / 255.0f;
            }
        }
    }
    return 1.0f;
}

void Window::SetVisibleOnAllWorkspaces(bool is_visible_on_all_workspaces) {
    // Windows doesn't have the same virtual desktop concept as macOS
    // This would require more complex implementation with Virtual Desktop APIs
}

bool Window::IsVisibleOnAllWorkspaces() const {
    // Default behavior on Windows
    return false;
}

void Window::SetIgnoreMouseEvents(bool is_ignore_mouse_events) {
    if (pimpl_->hwnd_) {
        LONG_PTR exStyle = GetWindowLongPtr(pimpl_->hwnd_, GWL_EXSTYLE);
        if (is_ignore_mouse_events) {
            exStyle |= WS_EX_TRANSPARENT;
        } else {
            exStyle &= ~WS_EX_TRANSPARENT;
        }
        SetWindowLongPtr(pimpl_->hwnd_, GWL_EXSTYLE, exStyle);
    }
}

bool Window::IsIgnoreMouseEvents() const {
    if (pimpl_->hwnd_) {
        LONG_PTR exStyle = GetWindowLongPtr(pimpl_->hwnd_, GWL_EXSTYLE);
        return (exStyle & WS_EX_TRANSPARENT) != 0;
    }
    return false;
}

void Window::SetFocusable(bool is_focusable) {
    if (pimpl_->hwnd_) {
        LONG_PTR exStyle = GetWindowLongPtr(pimpl_->hwnd_, GWL_EXSTYLE);
        if (is_focusable) {
            exStyle &= ~WS_EX_NOACTIVATE;
        } else {
            exStyle |= WS_EX_NOACTIVATE;
        }
        SetWindowLongPtr(pimpl_->hwnd_, GWL_EXSTYLE, exStyle);
    }
}

bool Window::IsFocusable() const {
    if (pimpl_->hwnd_) {
        LONG_PTR exStyle = GetWindowLongPtr(pimpl_->hwnd_, GWL_EXSTYLE);
        return (exStyle & WS_EX_NOACTIVATE) == 0;
    }
    return true;
}

void Window::StartDragging() {
    if (pimpl_->hwnd_) {
        // Simulate a title bar drag
        ReleaseCapture();
        SendMessage(pimpl_->hwnd_, WM_NCLBUTTONDOWN, HTCAPTION, 0);
    }
}

void Window::StartResizing() {
    if (pimpl_->hwnd_) {
        // Start resize from bottom-right corner
        ReleaseCapture();
        SendMessage(pimpl_->hwnd_, WM_NCLBUTTONDOWN, HTBOTTOMRIGHT, 0);
    }
}

void* Window::GetNSWindow() const {
    // This method name suggests macOS compatibility - return HWND instead
    return pimpl_->hwnd_;
}