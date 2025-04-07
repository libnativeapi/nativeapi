#pragma once

#include <functional>
#include <map>
#include <vector>
#include <cstdint>

#include "display.h"
#include "geometry.h"

namespace nativeapi {

// Event types that can be listened to
enum class ScreenEventType : uint8_t {
  DisplayAdded = 0x01,    // New display connected
  DisplayRemoved = 0x02,  // Display disconnected
};

// ScreenRetriever is a singleton that manages all screens on the system.
class ScreenRetriever {
 public:
  ScreenRetriever();
  virtual ~ScreenRetriever();

  // Get the current cursor screen point
  Point GetCursorScreenPoint();

  // Get the primary display information
  Display GetPrimaryDisplay();

  // Get all displays information
  std::vector<Display> GetAllDisplays();

  // Add event listener for specific event type
  void AddEventListener(ScreenEventType event_type,
                        std::function<void(const void*)> listener);

  // Remove event listener for specific event type
  void RemoveEventListener(ScreenEventType event_type,
                           std::function<void(const void*)> listener);

 private:
  // Store current displays to detect changes
  std::vector<Display> current_displays_;

  // Event listeners storage
  std::map<ScreenEventType, std::vector<std::function<void(const void*)>>>
      listeners_;

  void HandleDisplayChange();
};

}  // namespace nativeapi