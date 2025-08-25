#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "../../tray.h"
#include "../../menu.h"

namespace nativeapi {

// Window class name for tray window
static const wchar_t* TRAY_WINDOW_CLASS = L"NativeAPITrayWindow";
static UINT g_tray_message = WM_USER + 1;
static UINT g_next_tray_uid = 1000;

// Helper function to convert string to wide string
std::wstring StringToWideString(const std::string& str) {
    if (str.empty()) return std::wstring();
    
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), nullptr, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

// Helper function to convert base64 to HICON
HICON Base64ToIcon(const std::string& base64Data) {
    // This is a placeholder - in a full implementation you would:
    // 1. Decode the base64 string
    // 2. Create an HICON from the decoded image data
    // For now, return nullptr
    return nullptr;
}

// Window procedure for tray messages
LRESULT CALLBACK TrayWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == g_tray_message) {
        UINT tray_id = LOWORD(wParam);
        UINT mouse_msg = LOWORD(lParam);
        
        // Find the tray instance
        Tray* tray = reinterpret_cast<Tray*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        
        switch (mouse_msg) {
            case WM_RBUTTONUP: {
                // Show context menu
                if (tray) {
                    Menu contextMenu = tray->GetContextMenu();
                    HMENU hmenu = static_cast<HMENU>(contextMenu.GetNativeMenu());
                    if (hmenu) {
                        POINT pt;
                        GetCursorPos(&pt);
                        
                        // Required for popup menus to work correctly
                        SetForegroundWindow(hwnd);
                        
                        TrackPopupMenu(hmenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, nullptr);
                        
                        // Required after TrackPopupMenu
                        PostMessage(hwnd, WM_NULL, 0, 0);
                    }
                }
                break;
            }
            case WM_LBUTTONUP:
                // Handle left click
                break;
            case WM_LBUTTONDBLCLK:
                // Handle double click
                break;
        }
        return 0;
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// Private implementation class
class Tray::Impl {
public:
    Impl() : hwnd_(nullptr), tray_uid_(g_next_tray_uid++), icon_(nullptr) {
        CreateTrayWindow();
        InitializeTrayIcon();
    }
    
    ~Impl() {
        RemoveTrayIcon();
        if (hwnd_) {
            DestroyWindow(hwnd_);
        }
        if (icon_) {
            DestroyIcon(icon_);
        }
    }
    
    void CreateTrayWindow() {
        // Register window class if not already registered
        static bool class_registered = false;
        if (!class_registered) {
            WNDCLASS wc = {};
            wc.lpfnWndProc = TrayWindowProc;
            wc.hInstance = GetModuleHandle(nullptr);
            wc.lpszClassName = TRAY_WINDOW_CLASS;
            RegisterClass(&wc);
            class_registered = true;
        }
        
        // Create hidden window for tray messages
        hwnd_ = CreateWindow(
            TRAY_WINDOW_CLASS,
            L"TrayWindow",
            WS_OVERLAPPED,
            0, 0, 0, 0,
            nullptr, nullptr,
            GetModuleHandle(nullptr),
            nullptr
        );
    }
    
    void InitializeTrayIcon() {
        if (!hwnd_) return;
        
        nid_.cbSize = sizeof(NOTIFYICONDATA);
        nid_.hWnd = hwnd_;
        nid_.uID = tray_uid_;
        nid_.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid_.uCallbackMessage = g_tray_message;
        nid_.hIcon = LoadIcon(nullptr, IDI_APPLICATION); // Default icon
        wcscpy_s(nid_.szTip, L"Tray Icon");
        
        // Store tray pointer in window
        SetWindowLongPtr(hwnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(tray_ptr_));
        
        Shell_NotifyIcon(NIM_ADD, &nid_);
    }
    
    void RemoveTrayIcon() {
        if (hwnd_) {
            Shell_NotifyIcon(NIM_DELETE, &nid_);
        }
    }
    
    void UpdateIcon() {
        if (hwnd_ && icon_) {
            nid_.hIcon = icon_;
            Shell_NotifyIcon(NIM_MODIFY, &nid_);
        }
    }
    
    void UpdateTooltip(const std::string& tooltip) {
        if (!hwnd_) return;
        
        std::wstring wtooltip = StringToWideString(tooltip);
        wcsncpy_s(nid_.szTip, wtooltip.c_str(), sizeof(nid_.szTip) / sizeof(wchar_t) - 1);
        nid_.szTip[sizeof(nid_.szTip) / sizeof(wchar_t) - 1] = L'\0';
        
        Shell_NotifyIcon(NIM_MODIFY, &nid_);
    }

    HWND hwnd_;
    UINT tray_uid_;
    NOTIFYICONDATA nid_;
    HICON icon_;
    std::string title_;
    std::string tooltip_;
    Menu context_menu_;
    Tray* tray_ptr_ = nullptr;
};

Tray::Tray() : pimpl_(new Impl()) {
    id = -1;
    pimpl_->tray_ptr_ = this;
}

Tray::Tray(void* tray) : pimpl_(new Impl()) {
    id = -1; // Will be set by TrayManager when created
    pimpl_->tray_ptr_ = this;
}

Tray::~Tray() {
    delete pimpl_;
}

void Tray::SetIcon(std::string icon) {
    if (pimpl_->icon_) {
        DestroyIcon(pimpl_->icon_);
        pimpl_->icon_ = nullptr;
    }

    // Check if the icon is a base64 string
    if (icon.find("data:image") != std::string::npos) {
        // Extract the base64 part
        size_t pos = icon.find("base64,");
        if (pos != std::string::npos) {
            std::string base64Icon = icon.substr(pos + 7);
            pimpl_->icon_ = Base64ToIcon(base64Icon);
        }
    } else if (!icon.empty()) {
        // Load icon from file path
        std::wstring wicon = StringToWideString(icon);
        pimpl_->icon_ = (HICON)LoadImage(
            nullptr,
            wicon.c_str(),
            IMAGE_ICON,
            0, 0, // Use default size
            LR_LOADFROMFILE | LR_DEFAULTSIZE
        );
    }

    if (pimpl_->icon_) {
        pimpl_->UpdateIcon();
    }
}

void Tray::SetTitle(std::string title) {
    pimpl_->title_ = title;
    // Note: Windows tray icons don't display titles like macOS status items
    // The title could be used as part of the tooltip or for identification
}

std::string Tray::GetTitle() {
    return pimpl_->title_;
}

void Tray::SetTooltip(std::string tooltip) {
    pimpl_->tooltip_ = tooltip;
    pimpl_->UpdateTooltip(tooltip);
}

std::string Tray::GetTooltip() {
    return pimpl_->tooltip_;
}

void Tray::SetContextMenu(Menu menu) {
    pimpl_->context_menu_ = menu;
}

Menu Tray::GetContextMenu() {
    return pimpl_->context_menu_;
}

Rectangle Tray::GetBounds() {
    Rectangle bounds = {0, 0, 0, 0};
    
    if (pimpl_->hwnd_) {
        // Get the position of the notification area
        HWND hTray = FindWindow(L"Shell_TrayWnd", nullptr);
        if (hTray) {
            HWND hNotify = FindWindowEx(hTray, nullptr, L"TrayNotifyWnd", nullptr);
            if (hNotify) {
                RECT rect;
                GetWindowRect(hNotify, &rect);
                
                // This is approximate - getting exact icon position is complex
                bounds.x = static_cast<double>(rect.left);
                bounds.y = static_cast<double>(rect.top);
                bounds.width = static_cast<double>(rect.right - rect.left);
                bounds.height = static_cast<double>(rect.bottom - rect.top);
            }
        }
    }
    
    return bounds;
}

}  // namespace nativeapi