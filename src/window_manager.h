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

  // Create a new window with the given options.
  std::shared_ptr<Window> Create(const WindowOptions& options);

  // Get a window by its ID. Returns nullptr if window not found.
  std::shared_ptr<Window> Get(WindowID id);

  // Get all windows.
  std::vector<std::shared_ptr<Window>> GetAll();

  // Get the current window. Returns nullptr if no window is active.
  std::shared_ptr<Window> GetCurrent();

  // Destroy a window by its ID. Returns true if window was destroyed.
  bool Destroy(WindowID id);

 private:
  // Store window instances
  std::unordered_map<WindowID, std::shared_ptr<Window>> windows_;
};

}  // namespace nativeapi
