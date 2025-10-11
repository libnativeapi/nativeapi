#include <shellapi.h>
#include <windows.h>
#include <iostream>
#include <mutex>
#include <string>

#include "../../tray_icon.h"
#include "../../tray_manager.h"

namespace nativeapi {

// Hidden window class name for tray icon messages
static const char* TRAY_WINDOW_CLASS_NAME = "NativeAPITrayWindow";

// Hidden window procedure for handling tray icon messages
static LRESULT CALLBACK TrayWindowProc(HWND hwnd,
                                       UINT message,
                                       WPARAM wParam,
                                       LPARAM lParam) {
  // Try to find the TrayIcon instance associated with this message
  // In a real implementation, you'd have a better way to map messages to
  // instances
  return DefWindowProc(hwnd, message, wParam, lParam);
}

class TrayManager::Impl {
 public:
  Impl() : hwnd_(nullptr), next_icon_id_(1) { CreateHiddenWindow(); }

  ~Impl() {
    if (hwnd_) {
      DestroyWindow(hwnd_);
    }
    UnregisterClass(TRAY_WINDOW_CLASS_NAME, GetModuleHandle(nullptr));
  }

  void CreateHiddenWindow() {
    HINSTANCE hInstance = GetModuleHandle(nullptr);

    // Register window class
    WNDCLASS wc = {};
    wc.lpfnWndProc = TrayWindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = TRAY_WINDOW_CLASS_NAME;

    if (!RegisterClass(&wc)) {
      DWORD error = GetLastError();
      if (error != ERROR_CLASS_ALREADY_EXISTS) {
        std::cerr << "Failed to register tray window class. Error: " << error
                  << std::endl;
        return;
      }
    }

    // Create hidden window
    hwnd_ = CreateWindow(TRAY_WINDOW_CLASS_NAME, "NativeAPI Tray Window", 0, 0,
                         0, 0, 0,
                         HWND_MESSAGE,  // Message-only window
                         nullptr, hInstance, nullptr);

    if (!hwnd_) {
      std::cerr << "Failed to create tray window. Error: " << GetLastError()
                << std::endl;
    }
  }

  HWND hwnd_;
  UINT next_icon_id_;
};

TrayManager::TrayManager()
    : next_tray_id_(1), pimpl_(std::make_unique<Impl>()) {}

TrayManager::~TrayManager() {
  std::lock_guard<std::mutex> lock(mutex_);
  // Clean up all managed tray icons
  for (auto& pair : trays_) {
    auto tray = pair.second;
    if (tray) {
      // The TrayIcon destructor will handle cleanup of the tray icon
    }
  }
  trays_.clear();
}

bool TrayManager::IsSupported() {
  return true;  // Windows always supports system tray
}

std::shared_ptr<TrayIcon> TrayManager::Create() {
  std::lock_guard<std::mutex> lock(mutex_);

  if (!pimpl_->hwnd_) {
    std::cerr << "TrayManager: Hidden window not available" << std::endl;
    return nullptr;
  }

  auto tray = std::make_shared<TrayIcon>();
  tray->id = next_tray_id_++;

  // Windows-specific initialization is now handled internally by the TrayIcon
  // implementation The platform-specific details are encapsulated within the
  // PIMPL pattern
  UINT icon_id = pimpl_->next_icon_id_++;
  // Platform-specific setup is handled in the TrayIcon constructor and methods

  trays_[tray->id] = tray;

  return tray;
}

std::shared_ptr<TrayIcon> TrayManager::Get(TrayIconID id) {
  std::lock_guard<std::mutex> lock(mutex_);

  auto it = trays_.find(id);
  if (it != trays_.end()) {
    return it->second;
  }
  return nullptr;
}

std::vector<std::shared_ptr<TrayIcon>> TrayManager::GetAll() {
  std::lock_guard<std::mutex> lock(mutex_);

  std::vector<std::shared_ptr<TrayIcon>> trays;
  for (const auto& pair : trays_) {
    trays.push_back(pair.second);
  }
  return trays;
}

bool TrayManager::Destroy(TrayIconID id) {
  std::lock_guard<std::mutex> lock(mutex_);

  auto it = trays_.find(id);
  if (it != trays_.end()) {
    // Remove the tray icon from our container
    // The shared_ptr will automatically clean up when the last reference is
    // released
    trays_.erase(it);
    return true;
  }
  return false;
}

}  // namespace nativeapi
