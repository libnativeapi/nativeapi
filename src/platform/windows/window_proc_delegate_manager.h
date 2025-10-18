#pragma once

#include <windows.h>
#include <functional>
#include <mutex>
#include <optional>
#include <unordered_map>

namespace nativeapi {

using WindowProcDelegate =
    std::function<std::optional<LRESULT>(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)>;

class WindowProcDelegateManager {
 public:
  // Get the singleton instance
  static WindowProcDelegateManager& GetInstance();

  // Register a delegate
  // Returns a unique ID if registration was successful, -1 if failed
  int RegisterDelegate(WindowProcDelegate delegate);

  // Unregister the delegate by ID
  // Returns true if a delegate was found and removed, false if no delegate was
  // found
  bool UnregisterDelegate(int id);

  // Internal WindowProc function that dispatches messages to registered delegates
  static LRESULT CALLBACK InternalWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

 private:
  // Get the top level window handle
  HWND GetTopLevelWindow();

  // Setup internal WindowProc for the top level window
  void SetupInternalWindowProc();

  // Restore original WindowProc when no delegates are registered
  void RestoreOriginalWindowProc();

  mutable std::mutex mutex_;
  std::unordered_map<int, WindowProcDelegate> delegates_;
  int next_id_ = 1;
  
  // Store original WindowProc and window handle
  HWND top_level_window_ = nullptr;
  WNDPROC original_window_proc_ = nullptr;
  bool window_proc_hooked_ = false;
};

}  // namespace nativeapi
