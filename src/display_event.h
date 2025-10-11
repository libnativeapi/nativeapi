#pragma once
#include <string>
#include "display.h"

#include "foundation/event.h"
#include "foundation/geometry.h"

namespace nativeapi {

/**
 * Base class for all display-related events
 *
 * This class provides common functionality for display events,
 * including access to the display that triggered the event.
 */
class DisplayEvent : public Event {
 public:
  /**
   * Constructor for DisplayEvent
   * @param display The display associated with this event
   */
  explicit DisplayEvent(const Display& display) : display_(display) {}

  /**
   * Virtual destructor
   */
  virtual ~DisplayEvent() = default;

  /**
   * Get the display associated with this event
   * @return Reference to the display
   */
  const Display& GetDisplay() const { return display_; }

  /**
   * Get a string representation of the event type (for debugging)
   * Default implementation returns "DisplayEvent"
   */
  std::string GetTypeName() const override { return "DisplayEvent"; }

 private:
  Display display_;
};

/**
 * Event class for display addition
 *
 * This event is emitted when a new display is connected to the system.
 */
class DisplayAddedEvent : public DisplayEvent {
 public:
  explicit DisplayAddedEvent(const Display& display) : DisplayEvent(display) {}

  /**
   * Get a string representation of the event type
   */
  std::string GetTypeName() const override { return "DisplayAddedEvent"; }

  /**
   * Get the static type index for this event type
   */
  static std::type_index GetStaticType() {
    return std::type_index(typeid(DisplayAddedEvent));
  }

  /**
   * Get the type index for this event instance
   */
  std::type_index GetType() const {
    return GetStaticType();
  }
};

/**
 * Event class for display removal
 *
 * This event is emitted when a display is disconnected from the system.
 */
class DisplayRemovedEvent : public DisplayEvent {
 public:
  explicit DisplayRemovedEvent(const Display& display) : DisplayEvent(display) {}

  /**
   * Get a string representation of the event type
   */
  std::string GetTypeName() const override { return "DisplayRemovedEvent"; }

  /**
   * Get the static type index for this event type
   */
  static std::type_index GetStaticType() {
    return std::type_index(typeid(DisplayRemovedEvent));
  }

  /**
   * Get the type index for this event instance
   */
  std::type_index GetType() const {
    return GetStaticType();
  }
};

/**
 * Event class for display configuration changes
 *
 * This event is emitted when a display's properties change (resolution, orientation, etc.).
 */
class DisplayChangedEvent : public DisplayEvent {
 public:
  DisplayChangedEvent(const Display& old_display, const Display& new_display) 
      : DisplayEvent(new_display), old_display_(old_display) {}

  /**
   * Get the display information before the change
   * @return Reference to the old display state
   */
  const Display& GetOldDisplay() const { return old_display_; }

  /**
   * Get the display information after the change
   * @return Reference to the new display state
   */
  const Display& GetNewDisplay() const { return GetDisplay(); }

  /**
   * Get a string representation of the event type
   */
  std::string GetTypeName() const override { return "DisplayChangedEvent"; }

  /**
   * Get the static type index for this event type
   */
  static std::type_index GetStaticType() {
    return std::type_index(typeid(DisplayChangedEvent));
  }

  /**
   * Get the type index for this event instance
   */
  std::type_index GetType() const {
    return GetStaticType();
  }

 private:
  Display old_display_;
};

}  // namespace nativeapi