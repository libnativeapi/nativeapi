#pragma once

#include <vector>

namespace nativeapi {

// WindowManager is a singleton that manages all windows on the system.
class WindowManager {
 public:
  WindowManager();
  virtual ~WindowManager();

  Window Create();

  // Get the current window.
  Window GetCurrent();

  // Get all windows.
  std::vector<Window> GetAll();
};

}  // namespace nativeapi