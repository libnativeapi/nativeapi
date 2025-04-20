#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "window.h"
namespace nativeapi {

// WindowManager is a singleton that manages all windows on the system.
class WindowManager {
 public:
  WindowManager();
  virtual ~WindowManager();

  // Get a window by its ID. Returns nullptr if window not found.
  std::shared_ptr<Window> Get(WindowID id);

  // Get all windows.
  std::vector<std::shared_ptr<Window>> GetAll();

  // Get the current window. Returns nullptr if no window is active.
  std::shared_ptr<Window> GetCurrent();

 private:
  // Store window instances
  std::unordered_map<WindowID, std::shared_ptr<Window>> windows_;
};

}  // namespace nativeapi