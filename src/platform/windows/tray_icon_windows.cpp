#include <shellapi.h>
#include <windows.h>
#include <memory>
#include <string>

#include "../../foundation/geometry.h"
#include "../../foundation/id_allocator.h"
#include "../../menu.h"
#include "../../tray_icon.h"
#include "../../tray_icon_event.h"
#include "string_utils_windows.h"

namespace nativeapi {

// Private implementation class
class TrayIcon::Impl {
 public:
  Impl() : hwnd_(nullptr), icon_id_(0), icon_handle_(nullptr) {
    id_ = IdAllocator::Allocate<TrayIcon>();
  }

  Impl(HWND hwnd, UINT icon_id)
      : hwnd_(hwnd), icon_id_(icon_id), icon_handle_(nullptr) {
    id_ = IdAllocator::Allocate<TrayIcon>();
    // Initialize NOTIFYICONDATA structure
    ZeroMemory(&nid_, sizeof(NOTIFYICONDATAW));
    nid_.cbSize = sizeof(NOTIFYICONDATAW);
    nid_.hWnd = hwnd_;
    nid_.uID = icon_id_;
    nid_.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid_.uCallbackMessage = WM_USER + 1;  // Custom message for tray icon events
  }

  // Windows-specific method to set internal data
  void SetWindowsData(HWND hwnd, UINT icon_id) {
    hwnd_ = hwnd;
    icon_id_ = icon_id;

    // Re-initialize NOTIFYICONDATA structure
    ZeroMemory(&nid_, sizeof(NOTIFYICONDATAW));
    nid_.cbSize = sizeof(NOTIFYICONDATAW);
    nid_.hWnd = hwnd_;
    nid_.uID = icon_id_;
    nid_.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid_.uCallbackMessage = WM_USER + 1;  // Custom message for tray icon events
  }

  // Windows-specific message handler
  LRESULT HandleWindowsMessage(UINT message,
                               WPARAM wParam,
                               LPARAM lParam,
                               TrayIcon* tray_icon) {
    if (message == WM_USER + 1 && wParam == icon_id_) {
      switch (lParam) {
        case WM_LBUTTONUP:
          try {
            tray_icon->EmitSync<TrayIconClickedEvent>(tray_icon->GetId(),
                                                      "left");
          } catch (...) {
            // Protect against event emission exceptions
          }
          break;
        case WM_RBUTTONUP:
          try {
            tray_icon->EmitSync<TrayIconRightClickedEvent>(tray_icon->GetId());
          } catch (...) {
            // Protect against event emission exceptions
          }
          if (tray_icon->GetContextMenu()) {
            tray_icon->OpenContextMenu();
          }
          break;
        case WM_LBUTTONDBLCLK:
          try {
            tray_icon->EmitSync<TrayIconDoubleClickedEvent>(tray_icon->GetId());
          } catch (...) {
            // Protect against event emission exceptions
          }
          break;
      }
      return 0;
    }
    return DefWindowProc(hwnd_, message, wParam, lParam);
  }

  ~Impl() {
    if (hwnd_) {
      Shell_NotifyIconW(NIM_DELETE, &nid_);
    }
    if (icon_handle_) {
      DestroyIcon(icon_handle_);
    }
  }

  HWND hwnd_;
  UINT icon_id_;
  NOTIFYICONDATAW nid_;
  std::shared_ptr<Menu> context_menu_;
  HICON icon_handle_;
  TrayIconId id_;
};

TrayIcon::TrayIcon() : pimpl_(std::make_unique<Impl>()) {
  // Create a hidden window for this tray icon
  HINSTANCE hInstance = GetModuleHandle(nullptr);

  // Register window class for this tray icon
  static UINT next_class_id = 1;
  std::string class_name =
      "NativeAPITrayIcon_" + std::to_string(next_class_id++);
  std::wstring wclass_name = StringToWString(class_name);

  WNDCLASSW wc = {};
  wc.lpfnWndProc = DefWindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = wclass_name.c_str();

  if (RegisterClassW(&wc)) {
    // Create hidden message-only window
    std::wstring wtitle = StringToWString("NativeAPI Tray Icon");
    HWND hwnd =
        CreateWindowW(wclass_name.c_str(), wtitle.c_str(), 0, 0, 0, 0, 0,
                      HWND_MESSAGE,  // Message-only window
                      nullptr, hInstance, nullptr);

    if (hwnd) {
      // Generate unique icon ID
      static UINT next_icon_id = 1;
      UINT icon_id = next_icon_id++;

      // Reinitialize the Impl with the created window and icon ID
      pimpl_ = std::make_unique<Impl>(hwnd, icon_id);
    }
  }
}

TrayIcon::TrayIcon(void* tray) : pimpl_(std::make_unique<Impl>()) {
  // In a real implementation, you'd extract HWND and icon ID from the tray
  // parameter For now, this constructor is mainly used by TrayManager for
  // creating uninitialized icons
}

TrayIcon::~TrayIcon() {}

TrayIconId TrayIcon::GetId() {
  return pimpl_->id_;
}

void TrayIcon::SetIcon(std::string icon) {
  if (!pimpl_->hwnd_) {
    return;
  }

  HICON hIcon = nullptr;

  // Check if the icon is a base64 string
  if (icon.find("data:image") == 0) {
    // For base64 icons, we'd need to decode and create HICON
    // This is a placeholder implementation
    hIcon = LoadIcon(nullptr, IDI_APPLICATION);
  } else if (!icon.empty()) {
    // Try to load as file path first
    std::wstring wicon = StringToWString(icon);
    hIcon = (HICON)LoadImageW(nullptr, wicon.c_str(), IMAGE_ICON, 16, 16,
                              LR_LOADFROMFILE);

    // If file path failed, try as resource
    if (!hIcon) {
      hIcon = LoadIconW(GetModuleHandle(nullptr), wicon.c_str());
    }

    // If still failed, use default application icon
    if (!hIcon) {
      hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    }
  } else {
    // Use default application icon
    hIcon = LoadIcon(nullptr, IDI_APPLICATION);
  }

  if (hIcon) {
    // Clean up previous icon
    if (pimpl_->icon_handle_) {
      DestroyIcon(pimpl_->icon_handle_);
    }

    pimpl_->icon_handle_ = hIcon;
    pimpl_->nid_.hIcon = hIcon;

    // Update the icon if it's currently visible
    if (IsVisible()) {
      Shell_NotifyIconW(NIM_MODIFY, &pimpl_->nid_);
    }
  }
}

void TrayIcon::SetTitle(std::optional<std::string> title) {
  (void)title;  // Unused on Windows
  // Windows tray icons don't support title
}

std::optional<std::string> TrayIcon::GetTitle() {
  // Windows tray icons don't support title
  return std::nullopt;
}

void TrayIcon::SetTooltip(std::optional<std::string> tooltip) {
  if (pimpl_->hwnd_) {
    std::string tooltip_str = tooltip.has_value() ? *tooltip : "";
    std::wstring wtooltip = StringToWString(tooltip_str);
    wcsncpy_s(pimpl_->nid_.szTip, _countof(pimpl_->nid_.szTip),
              wtooltip.c_str(), _TRUNCATE);

    // Update if icon is visible (check if hIcon is set as indicator)
    if (pimpl_->nid_.hIcon) {
      Shell_NotifyIconW(NIM_MODIFY, &pimpl_->nid_);
    }
  }
}

std::optional<std::string> TrayIcon::GetTooltip() {
  if (pimpl_->hwnd_ && pimpl_->nid_.szTip[0] != L'\0') {
    return WCharArrayToString(pimpl_->nid_.szTip);
  }
  return std::nullopt;
}

void TrayIcon::SetContextMenu(std::shared_ptr<Menu> menu) {
  pimpl_->context_menu_ = menu;
}

std::shared_ptr<Menu> TrayIcon::GetContextMenu() {
  return pimpl_->context_menu_;
}

Rectangle TrayIcon::GetBounds() {
  Rectangle bounds = {0, 0, 0, 0};

  if (pimpl_->hwnd_ && IsVisible()) {
    RECT rect;
    NOTIFYICONIDENTIFIER niid = {};
    niid.cbSize = sizeof(NOTIFYICONIDENTIFIER);
    niid.hWnd = pimpl_->hwnd_;
    niid.uID = pimpl_->icon_id_;

    // Get the rectangle of the notification icon
    if (Shell_NotifyIconGetRect(&niid, &rect) == S_OK) {
      bounds.x = rect.left;
      bounds.y = rect.top;
      bounds.width = rect.right - rect.left;
      bounds.height = rect.bottom - rect.top;
    }
  }

  return bounds;
}

bool TrayIcon::SetVisible(bool visible) {
  if (!pimpl_->hwnd_) {
    return false;
  }

  bool currently_visible = IsVisible();

  if (visible && !currently_visible) {
    // Show the tray icon
    return Shell_NotifyIconW(NIM_ADD, &pimpl_->nid_) == TRUE;
  } else if (!visible && currently_visible) {
    // Hide the tray icon
    return Shell_NotifyIconW(NIM_DELETE, &pimpl_->nid_) == TRUE;
  } else {
    // Already in the desired state
    return true;
  }
}

bool TrayIcon::IsVisible() {
  if (!pimpl_->hwnd_) {
    return false;
  }

  // Check if the tray icon is visible by querying its bounds
  NOTIFYICONIDENTIFIER niid = {};
  niid.cbSize = sizeof(NOTIFYICONIDENTIFIER);
  niid.hWnd = pimpl_->hwnd_;
  niid.uID = pimpl_->icon_id_;

  RECT rect;
  return Shell_NotifyIconGetRect(&niid, &rect) == S_OK;
}

bool TrayIcon::OpenContextMenu(double x, double y) {
  if (!pimpl_->context_menu_) {
    return false;
  }

  // Open the context menu at the specified coordinates
  return pimpl_->context_menu_->Open(x, y);
}

bool TrayIcon::OpenContextMenu() {
  if (!pimpl_->context_menu_) {
    return false;
  }

  // Get the bounds of the tray icon to show menu near it
  Rectangle bounds = GetBounds();
  if (bounds.width > 0 && bounds.height > 0) {
    // Open menu below the tray icon
    return pimpl_->context_menu_->Open(bounds.x, bounds.y + bounds.height);
  } else {
    // Fall back to showing at mouse location
    return pimpl_->context_menu_->Open();
  }
}

bool TrayIcon::CloseContextMenu() {
  if (!pimpl_->context_menu_) {
    return true;  // No menu to close, consider success
  }

  // Close the context menu
  return pimpl_->context_menu_->Close();
}

void* TrayIcon::GetNativeObjectInternal() const {
  return reinterpret_cast<void*>(static_cast<uintptr_t>(pimpl_->icon_id_));
}

// Note: Windows-specific functionality is now handled internally by the Impl
// class The SetWindowsData and HandleWindowsMessage methods have been moved to
// the Impl class to maintain proper encapsulation and avoid exposing
// platform-specific details in the public API

}  // namespace nativeapi
