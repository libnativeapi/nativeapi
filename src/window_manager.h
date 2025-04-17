#pragma once

#include <vector>

#include "window.h"
namespace nativeapi {

// WindowManager is a singleton that manages all windows on the system.
class WindowManager {
 public:
  WindowManager();
  virtual ~WindowManager();

  // Get a window by its ID.
  Window Get(WindowID id);

  // Get all windows.
  std::vector<Window> GetAll();

  // Get the current window.
  Window GetCurrent();
};

}  // namespace nativeapi