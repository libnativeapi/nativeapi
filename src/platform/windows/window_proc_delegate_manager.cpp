#include "window_proc_delegate_manager.h"
#include <windows.h>
#include <iostream>

namespace nativeapi {

WindowProcDelegateManager& WindowProcDelegateManager::GetInstance() {
  static WindowProcDelegateManager instance;
  return instance;
}

LRESULT CALLBACK WindowProcDelegateManager::InternalWindowProc(HWND hwnd,
                                                               UINT message,
                                                               WPARAM wparam,
                                                               LPARAM lparam) {
  WindowProcDelegateManager& manager = GetInstance();
  std::lock_guard<std::mutex> lock(manager.mutex_);

  // Dispatch message to all registered delegates
  for (const auto& pair : manager.delegates_) {
    const auto& delegate = pair.second;
    if (delegate) {
      auto result = delegate(hwnd, message, wparam, lparam);
      if (result.has_value()) {
        return result.value();
      }
    }
  }

  // If no delegate handled the message, call the original WindowProc
  if (manager.original_window_proc_) {
    return CallWindowProc(manager.original_window_proc_, hwnd, message, wparam,
                          lparam);
  }

  // Fallback to DefWindowProc if no original proc is available
  return DefWindowProc(hwnd, message, wparam, lparam);
}

HWND WindowProcDelegateManager::GetTopLevelWindow() {
  // Try to get the foreground window first
  HWND hwnd = GetForegroundWindow();
  if (hwnd && IsWindow(hwnd)) {
    return hwnd;
  }

  // If no foreground window, try to get the active window
  hwnd = GetActiveWindow();
  if (hwnd && IsWindow(hwnd)) {
    return hwnd;
  }

  // If still no window, try to find any top-level window
  hwnd = FindWindow(nullptr, nullptr);
  if (hwnd && IsWindow(hwnd)) {
    return hwnd;
  }

  return nullptr;
}

void WindowProcDelegateManager::SetupInternalWindowProc() {
  if (window_proc_hooked_) {
    return;  // Already hooked
  }

  top_level_window_ = GetTopLevelWindow();
  if (!top_level_window_) {
    std::cerr << "Failed to get top level window for WindowProc hooking"
              << std::endl;
    return;
  }

  // Store the original WindowProc
  original_window_proc_ = reinterpret_cast<WNDPROC>(
      GetWindowLongPtr(top_level_window_, GWLP_WNDPROC));
  if (!original_window_proc_) {
    std::cerr << "Failed to get original WindowProc" << std::endl;
    return;
  }

  // Set our internal WindowProc
  if (SetWindowLongPtr(top_level_window_, GWLP_WNDPROC,
                       reinterpret_cast<LONG_PTR>(InternalWindowProc)) == 0) {
    DWORD error = GetLastError();
    std::cerr << "Failed to set internal WindowProc. Error: " << error
              << std::endl;
    return;
  }

  window_proc_hooked_ = true;
}

void WindowProcDelegateManager::RestoreOriginalWindowProc() {
  if (!window_proc_hooked_ || !top_level_window_ || !original_window_proc_) {
    return;
  }

  // Restore the original WindowProc
  if (SetWindowLongPtr(top_level_window_, GWLP_WNDPROC,
                       reinterpret_cast<LONG_PTR>(original_window_proc_)) ==
      0) {
    DWORD error = GetLastError();
    std::cerr << "Failed to restore original WindowProc. Error: " << error
              << std::endl;
  }

  // Reset state
  window_proc_hooked_ = false;
  top_level_window_ = nullptr;
  original_window_proc_ = nullptr;
}

int WindowProcDelegateManager::RegisterDelegate(WindowProcDelegate delegate) {
  if (!delegate) {
    return -1;
  }

  std::lock_guard<std::mutex> lock(mutex_);

  int id = next_id_++;
  delegates_[id] = std::move(delegate);

  // If this is the first delegate, setup the internal WindowProc
  if (delegates_.size() == 1) {
    SetupInternalWindowProc();
  }

  return id;
}

bool WindowProcDelegateManager::UnregisterDelegate(int id) {
  if (id < 0) {
    return false;
  }

  std::lock_guard<std::mutex> lock(mutex_);

  auto it = delegates_.find(id);
  if (it != delegates_.end()) {
    delegates_.erase(it);

    // If no delegates remain, restore the original WindowProc
    if (delegates_.empty()) {
      RestoreOriginalWindowProc();
    }

    return true;
  }

  return false;
}

}  // namespace nativeapi
