#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <vector>

#include "display.h"
#include "display_event.h"
#include "event.h"
#include "event_emitter.h"
#include "geometry.h"

namespace nativeapi {



/**
 * DisplayManager is a singleton that manages all displays on the system.
 *
 * This class provides functionality to:
 * - Query all connected displays
 * - Get primary display information
 * - Monitor display changes (addition/removal)
 * - Get cursor position across displays
 *
 * The DisplayManager uses the singleton pattern to ensure there's only one
 * instance managing the display system throughout the application lifecycle.
 *
 * Thread Safety: This class is not thread-safe. External synchronization
 * is required if accessed from multiple threads.
 *
 * Example usage:
 * @code
 * DisplayManager& manager = DisplayManager::GetInstance();
 * std::vector<Display> displays = manager.GetAll();
 * Display primary = manager.GetPrimary();
 * @endcode
 */
class DisplayManager : public EventEmitter {
 public:
  /**
   * Get the singleton instance of DisplayManager
   * @return Reference to the singleton DisplayManager instance
   */
  static DisplayManager& GetInstance();

  /**
   * @brief Destructor for DisplayManager.
   *
   * Cleans up all resources, stops event monitoring.
   * This is automatically called when the application terminates.
   */
  virtual ~DisplayManager();

  /**
   * Get all connected displays information
   *
   * @return Vector containing all Display objects representing connected
   * displays. The vector may be empty if no displays are detected.
   * @note The returned vector is a snapshot of current displays at the time of
   * call
   */
  std::vector<Display> GetAll();

  /**
   * Get the primary display information
   *
   * The primary display is typically the main screen where the desktop
   * environment displays its primary interface elements.
   *
   * @return Display object representing the primary display
   * @throws std::runtime_error if no primary display is available
   */
  Display GetPrimary();

  /**
   * Get the current cursor position in screen coordinates
   *
   * The coordinates are relative to the top-left corner of the primary display,
   * with positive X extending right and positive Y extending down.
   *
   * @return Point containing the current cursor coordinates (x, y)
   * @note The position is captured at the time of the function call
   */
  Point GetCursorPosition();

  // Prevent copy construction and assignment to maintain singleton property
  DisplayManager(const DisplayManager&) = delete;
  DisplayManager& operator=(const DisplayManager&) = delete;
  DisplayManager(DisplayManager&&) = delete;
  DisplayManager& operator=(DisplayManager&&) = delete;

 private:
  /**
   * @brief Private constructor to enforce singleton pattern.
   *
   * Initializes the DisplayManager instance and sets up initial state.
   */
  DisplayManager();

  /**
   * Cached list of displays
   * Updated when display configuration changes are detected.
   */
  std::vector<Display> displays_;

  /**
   * Static instance holder for singleton pattern
   */
  static DisplayManager* instance_;
};

}  // namespace nativeapi
