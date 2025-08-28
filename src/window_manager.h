#pragma once

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#include "event.h"
#include "event_emitter.h"
#include "geometry.h"
#include "window.h"

namespace nativeapi {

/**
 * Event class for window creation
 */
class WindowCreatedEvent : public TypedEvent<WindowCreatedEvent> {
 public:
  explicit WindowCreatedEvent(WindowID window_id) : window_id_(window_id) {}

  WindowID GetWindowId() const { return window_id_; }

 private:
  WindowID window_id_;
};

/**
 * Event class for window closure
 */
class WindowClosedEvent : public TypedEvent<WindowClosedEvent> {
 public:
  explicit WindowClosedEvent(WindowID window_id) : window_id_(window_id) {}

  WindowID GetWindowId() const { return window_id_; }

 private:
  WindowID window_id_;
};

/**
 * Event class for window focus gained
 */
class WindowFocusedEvent : public TypedEvent<WindowFocusedEvent> {
 public:
  explicit WindowFocusedEvent(WindowID window_id) : window_id_(window_id) {}

  WindowID GetWindowId() const { return window_id_; }

 private:
  WindowID window_id_;
};

/**
 * Event class for window focus lost
 */
class WindowBlurredEvent : public TypedEvent<WindowBlurredEvent> {
 public:
  explicit WindowBlurredEvent(WindowID window_id) : window_id_(window_id) {}

  WindowID GetWindowId() const { return window_id_; }

 private:
  WindowID window_id_;
};

/**
 * Event class for window minimized
 */
class WindowMinimizedEvent : public TypedEvent<WindowMinimizedEvent> {
 public:
  explicit WindowMinimizedEvent(WindowID window_id) : window_id_(window_id) {}

  WindowID GetWindowId() const { return window_id_; }

 private:
  WindowID window_id_;
};

/**
 * Event class for window maximized
 */
class WindowMaximizedEvent : public TypedEvent<WindowMaximizedEvent> {
 public:
  explicit WindowMaximizedEvent(WindowID window_id) : window_id_(window_id) {}

  WindowID GetWindowId() const { return window_id_; }

 private:
  WindowID window_id_;
};

/**
 * Event class for window restored
 */
class WindowRestoredEvent : public TypedEvent<WindowRestoredEvent> {
 public:
  explicit WindowRestoredEvent(WindowID window_id) : window_id_(window_id) {}

  WindowID GetWindowId() const { return window_id_; }

 private:
  WindowID window_id_;
};

/**
 * Event class for window moved
 */
class WindowMovedEvent : public TypedEvent<WindowMovedEvent> {
 public:
  WindowMovedEvent(WindowID window_id, Point new_position)
      : window_id_(window_id), new_position_(new_position) {}

  WindowID GetWindowId() const { return window_id_; }
  Point GetNewPosition() const { return new_position_; }

 private:
  WindowID window_id_;
  Point new_position_;
};

/**
 * Event class for window resized
 */
class WindowResizedEvent : public TypedEvent<WindowResizedEvent> {
 public:
  WindowResizedEvent(WindowID window_id, Size new_size)
      : window_id_(window_id), new_size_(new_size) {}

  WindowID GetWindowId() const { return window_id_; }
  Size GetNewSize() const { return new_size_; }

 private:
  WindowID window_id_;
  Size new_size_;
};

// WindowManager is a singleton that manages all windows on the system.
class WindowManager: public EventEmitter {
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
  class Impl;  // Forward declaration only
  std::unique_ptr<Impl> pimpl_;
  
  // Store window instances
  std::unordered_map<WindowID, std::shared_ptr<Window>> windows_;
  
  // Platform-specific method to set up event monitoring
  void SetupEventMonitoring();
  
  // Platform-specific method to clean up event monitoring
  void CleanupEventMonitoring();
  
  // Internal method to dispatch window events
  void DispatchWindowEvent(const Event& event);
};

}  // namespace nativeapi
