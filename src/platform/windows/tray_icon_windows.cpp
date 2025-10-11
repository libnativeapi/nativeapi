#include <shellapi.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

#include "../../geometry.h"
#include "../../menu.h"
#include "../../tray_icon.h"
#include "../../tray_icon_event.h"

namespace nativeapi {

// Private implementation class
class TrayIcon::Impl {
 public:
  Impl()
      : hwnd_(nullptr), icon_id_(0), visible_(false), icon_handle_(nullptr) {}

  Impl(HWND hwnd, UINT icon_id)
      : hwnd_(hwnd), icon_id_(icon_id), visible_(false), icon_handle_(nullptr) {
    // Initialize NOTIFYICONDATA structure
    ZeroMemory(&nid_, sizeof(NOTIFYICONDATA));
    nid_.cbSize = sizeof(NOTIFYICONDATA);
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
    ZeroMemory(&nid_, sizeof(NOTIFYICONDATA));
    nid_.cbSize = sizeof(NOTIFYICONDATA);
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
          tray_icon->HandleLeftClick();
          break;
        case WM_RBUTTONUP:
          tray_icon->HandleRightClick();
          if (tray_icon->GetContextMenu()) {
            tray_icon->OpenContextMenu();
          }
          break;
        case WM_LBUTTONDBLCLK:
          tray_icon->HandleDoubleClick();
          break;
      }
      return 0;
    }
    return DefWindowProc(hwnd_, message, wParam, lParam);
  }

  ~Impl() {
    if (visible_) {
      Shell_NotifyIcon(NIM_DELETE, &nid_);
    }
    if (icon_handle_) {
      DestroyIcon(icon_handle_);
    }
  }

  HWND hwnd_;
  UINT icon_id_;
  NOTIFYICONDATA nid_;
  std::shared_ptr<Menu> context_menu_;
  std::string title_;
  std::string tooltip_;
  bool visible_;
  HICON icon_handle_;
};

TrayIcon::TrayIcon() : pimpl_(std::make_unique<Impl>()) {
  id = -1;
}

TrayIcon::TrayIcon(void* tray) : pimpl_(std::make_unique<Impl>()) {
  id = -1;  // Will be set by TrayManager when created
  // In a real implementation, you'd extract HWND and icon ID from the tray
  // parameter For now, this constructor is mainly used by TrayManager for
  // creating uninitialized icons
}

TrayIcon::~TrayIcon() {}

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
    hIcon = (HICON)LoadImage(nullptr, icon.c_str(), IMAGE_ICON, 16, 16,
                             LR_LOADFROMFILE);

    // If file path failed, try as resource
    if (!hIcon) {
      hIcon = LoadIcon(GetModuleHandle(nullptr), icon.c_str());
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

    if (pimpl_->visible_) {
      Shell_NotifyIcon(NIM_MODIFY, &pimpl_->nid_);
    }
  }
}

void TrayIcon::SetTitle(std::string title) {
  pimpl_->title_ = title;
  // Windows tray icons don't have a separate title, only tooltip
  // We'll use title as part of the tooltip
  SetTooltip(title);
}

std::string TrayIcon::GetTitle() {
  return pimpl_->title_;
}

void TrayIcon::SetTooltip(std::string tooltip) {
  pimpl_->tooltip_ = tooltip;

  if (pimpl_->hwnd_) {
    strncpy_s(pimpl_->nid_.szTip, tooltip.c_str(),
              sizeof(pimpl_->nid_.szTip) - 1);
    pimpl_->nid_.szTip[sizeof(pimpl_->nid_.szTip) - 1] = '\0';

    if (pimpl_->visible_) {
      Shell_NotifyIcon(NIM_MODIFY, &pimpl_->nid_);
    }
  }
}

std::string TrayIcon::GetTooltip() {
  return pimpl_->tooltip_;
}

void TrayIcon::SetContextMenu(std::shared_ptr<Menu> menu) {
  pimpl_->context_menu_ = menu;
}

std::shared_ptr<Menu> TrayIcon::GetContextMenu() {
  return pimpl_->context_menu_;
}

Rectangle TrayIcon::GetBounds() {
  Rectangle bounds = {0, 0, 0, 0};

  if (pimpl_->hwnd_ && pimpl_->visible_) {
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

bool TrayIcon::Show() {
  if (pimpl_->hwnd_ && !pimpl_->visible_) {
    if (Shell_NotifyIcon(NIM_ADD, &pimpl_->nid_) == TRUE) {
      pimpl_->visible_ = true;
      return true;
    }
  }
  return false;
}

bool TrayIcon::Hide() {
  if (pimpl_->hwnd_ && pimpl_->visible_) {
    if (Shell_NotifyIcon(NIM_DELETE, &pimpl_->nid_) == TRUE) {
      pimpl_->visible_ = false;
      return true;
    }
  }
  return false;
}

bool TrayIcon::IsVisible() {
  return pimpl_->visible_;
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

// Internal method to handle click events
void TrayIcon::HandleLeftClick() {
  try {
    EmitSync<TrayIconClickedEvent>(id, "left");
  } catch (...) {
    // Protect against event emission exceptions
  }
}

void TrayIcon::HandleRightClick() {
  try {
    EmitSync<TrayIconRightClickedEvent>(id);
  } catch (...) {
    // Protect against event emission exceptions
  }
}

void TrayIcon::HandleDoubleClick() {
  try {
    EmitSync<TrayIconDoubleClickedEvent>(id);
  } catch (...) {
    // Protect against event emission exceptions
  }
}

// Note: Windows-specific functionality is now handled internally by the Impl
// class The SetWindowsData and HandleWindowsMessage methods have been moved to
// the Impl class to maintain proper encapsulation and avoid exposing
// platform-specific details in the public API

}  // namespace nativeapi
