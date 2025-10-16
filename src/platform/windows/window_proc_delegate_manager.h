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

 private:
  mutable std::mutex mutex_;
  std::unordered_map<int, WindowProcDelegate> delegates_;
  int next_id_ = 1;
};

}  // namespace nativeapi
