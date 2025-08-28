#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <vector>

#include "display.h"
#include "event.h"
#include "event_emitter.h"
#include "geometry.h"

namespace nativeapi {

/**
 * Event class for display addition
 */
class DisplayAddedEvent : public TypedEvent<DisplayAddedEvent> {
 public:
  explicit DisplayAddedEvent(const Display& display) : display_(display) {}

  const Display& GetDisplay() const { return display_; }

 private:
  Display display_;
};

/**
 * Event class for display removal
 */
class DisplayRemovedEvent : public TypedEvent<DisplayRemovedEvent> {
 public:
  explicit DisplayRemovedEvent(const Display& display) : display_(display) {}

  const Display& GetDisplay() const { return display_; }

 private:
  Display display_;
};

/**
 * DisplayManager is a singleton that manages all displays on the system.
 */
class DisplayManager : public EventEmitter {
 public:
  DisplayManager();
  virtual ~DisplayManager();

  // Get all displays information
  std::vector<Display> GetAll();

  // Get the primary display information
  Display GetPrimary();

  // Get the current cursor position
  Point GetCursorPosition();

 private:
  std::vector<Display> displays_;
};

}  // namespace nativeapi
