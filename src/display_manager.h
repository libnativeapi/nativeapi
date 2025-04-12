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

  // Get all displays information
  std::vector<Display> GetAll();

  // Get the primary display information
  Display GetPrimary();

  // Get the current cursor position
  Point GetCursorPosition();

  // Add a listener to the display manager
  void AddListener(DisplayListener* listener);

  // Remove a listener from the display manager
  void RemoveListener(DisplayListener* listener);

 private:
  std::vector<Display> displays_;
  std::vector<DisplayListener*> listeners_;
  void NotifyListeners(std::function<void(DisplayListener*)> callback);
};

// DisplayEventHandler is an implementation of DisplayListener that uses
// callbacks to handle display events.
class DisplayEventHandler : public DisplayListener {
 public:
  // Constructor that takes callbacks for display events
  DisplayEventHandler(
      std::function<void(const Display&)> onDisplayAddedCallback,
      std::function<void(const Display&)> onDisplayRemovedCallback);

  // Implementation of OnDisplayAdded from DisplayListener interface
  void OnDisplayAdded(const Display& display) override;

  // Implementation of OnDisplayRemoved from DisplayListener interface
  void OnDisplayRemoved(const Display& display) override;

 private:
  std::function<void(const Display&)> onDisplayAddedCallback_;
  std::function<void(const Display&)> onDisplayRemovedCallback_;
};

}  // namespace nativeapi
