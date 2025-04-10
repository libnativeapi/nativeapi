#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <vector>

#include "display.h"
#include "geometry.h"

namespace nativeapi {

// DisplayListener is an interface that can be implemented by classes that want
// to listen for display events.
class DisplayListener {
 public:
  virtual void OnDisplayAdded(const Display& display) = 0;
  virtual void OnDisplayRemoved(const Display& display) = 0;
};

// DisplayManager is a singleton that manages all displays on the system.
class DisplayManager {
 public:
  DisplayManager();
  virtual ~DisplayManager();

  // Get the current cursor position
  Point GetCursorPosition();

  // Get all displays information
  std::vector<Display> GetAll();

  // Get the primary display information
  Display GetPrimary();

  // Add a listener to the display manager
  void AddListener(DisplayListener* listener);

  // Remove a listener from the display manager
  void RemoveListener(DisplayListener* listener);

 private:
  std::vector<Display> displays_;
  std::vector<DisplayListener*> listeners_;
  void NotifyDisplayAdded(const Display& display);
  void NotifyDisplayRemoved(const Display& display);
};

}  // namespace nativeapi
