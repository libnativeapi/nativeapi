#pragma once
#include <string>
#include "foundation/event.h"
#include "foundation/geometry.h"
#include "window.h"

namespace nativeapi {

/**
 * Base class for all window-related events
 *
 * This class provides common functionality for window events,
 * including access to the window ID that triggered the event.
 */
class WindowEvent : public Event {
 public:
  /**
   * Constructor for WindowEvent
   * @param window_id The window ID associated with this event
   */
  explicit WindowEvent(WindowId window_id) : window_id_(window_id) {}

  /**
   * Virtual destructor
   */
  virtual ~WindowEvent() = default;

  /**
   * Get the window ID associated with this event
   * @return The window ID
   */
  WindowId GetWindowId() const { return window_id_; }

  /**
   * Get a string representation of the event type (for debugging)
   * Default implementation returns "WindowEvent"
   */
  std::string GetTypeName() const override { return "WindowEvent"; }

 private:
  WindowId window_id_;
};

/**
 * Event class for window creation
 *
 * This event is emitted when a new window is successfully created.
 */
class WindowCreatedEvent : public WindowEvent {
 public:
  explicit WindowCreatedEvent(WindowId window_id) : WindowEvent(window_id) {}

  /**
   * Get a string representation of the event type
   */
  std::string GetTypeName() const override { return "WindowCreatedEvent"; }
};

/**
 * Event class for window closure
 *
 * This event is emitted when a window is closed or destroyed.
 */
class WindowClosedEvent : public WindowEvent {
 public:
  explicit WindowClosedEvent(WindowId window_id) : WindowEvent(window_id) {}

  /**
   * Get a string representation of the event type
   */
  std::string GetTypeName() const override { return "WindowClosedEvent"; }

  /**
   * Get the static type index for this event type
   */
};

/**
 * Event class for window focus gained
 *
 * This event is emitted when a window gains focus and becomes the active window.
 */
class WindowFocusedEvent : public WindowEvent {
 public:
  explicit WindowFocusedEvent(WindowId window_id) : WindowEvent(window_id) {}

  /**
   * Get a string representation of the event type
   */
  std::string GetTypeName() const override { return "WindowFocusedEvent"; }

  /**
   * Get the static type index for this event type
   */
};

/**
 * Event class for window focus lost
 *
 * This event is emitted when a window loses focus and is no longer the active window.
 */
class WindowBlurredEvent : public WindowEvent {
 public:
  explicit WindowBlurredEvent(WindowId window_id) : WindowEvent(window_id) {}

  /**
   * Get a string representation of the event type
   */
  std::string GetTypeName() const override { return "WindowBlurredEvent"; }

  /**
   * Get the static type index for this event type
   */
};

/**
 * Event class for window minimized
 *
 * This event is emitted when a window is minimized to the taskbar or dock.
 */
class WindowMinimizedEvent : public WindowEvent {
 public:
  explicit WindowMinimizedEvent(WindowId window_id) : WindowEvent(window_id) {}

  /**
   * Get a string representation of the event type
   */
  std::string GetTypeName() const override { return "WindowMinimizedEvent"; }

  /**
   * Get the static type index for this event type
   */
};

/**
 * Event class for window maximized
 *
 * This event is emitted when a window is maximized to fill the entire screen.
 */
class WindowMaximizedEvent : public WindowEvent {
 public:
  explicit WindowMaximizedEvent(WindowId window_id) : WindowEvent(window_id) {}

  /**
   * Get a string representation of the event type
   */
  std::string GetTypeName() const override { return "WindowMaximizedEvent"; }

  /**
   * Get the static type index for this event type
   */
};

/**
 * Event class for window restored
 *
 * This event is emitted when a window is restored from minimized or maximized state
 * to its normal windowed state.
 */
class WindowRestoredEvent : public WindowEvent {
 public:
  explicit WindowRestoredEvent(WindowId window_id) : WindowEvent(window_id) {}

  /**
   * Get a string representation of the event type
   */
  std::string GetTypeName() const override { return "WindowRestoredEvent"; }

  /**
   * Get the static type index for this event type
   */
};

/**
 * Event class for window moved
 *
 * This event is emitted when a window is moved to a new position on the screen.
 */
class WindowMovedEvent : public WindowEvent {
 public:
  WindowMovedEvent(WindowId window_id, Point new_position)
      : WindowEvent(window_id), new_position_(new_position) {}

  /**
   * Get the new position of the window
   * @return The new position as a Point
   */
  Point GetNewPosition() const { return new_position_; }

  /**
   * Get a string representation of the event type
   */
  std::string GetTypeName() const override { return "WindowMovedEvent"; }

  /**
   * Get the static type index for this event type
   */

 private:
  Point new_position_;
};

/**
 * Event class for window resized
 *
 * This event is emitted when a window is resized to a new size.
 */
class WindowResizedEvent : public WindowEvent {
 public:
  WindowResizedEvent(WindowId window_id, Size new_size)
      : WindowEvent(window_id), new_size_(new_size) {}

  /**
   * Get the new size of the window
   * @return The new size as a Size object
   */
  Size GetNewSize() const { return new_size_; }

  /**
   * Get a string representation of the event type
   */
  std::string GetTypeName() const override { return "WindowResizedEvent"; }

  /**
   * Get the static type index for this event type
   */

 private:
  Size new_size_;
};

}  // namespace nativeapi
