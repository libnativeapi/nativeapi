#pragma once

#include "event.h"
#include "display.h"
#include "geometry.h"

namespace nativeapi {

/**
 * Example events that demonstrate how to integrate the generic event system
 * with existing components like display management and keyboard monitoring.
 */

/**
 * Display-related events
 */
class DisplayAddedEvent : public TypedEvent<DisplayAddedEvent> {
 public:
  explicit DisplayAddedEvent(const Display& display) : display_(display) {}
  
  const Display& GetDisplay() const { return display_; }
  
 private:
  Display display_;
};

class DisplayRemovedEvent : public TypedEvent<DisplayRemovedEvent> {
 public:
  explicit DisplayRemovedEvent(const Display& display) : display_(display) {}
  
  const Display& GetDisplay() const { return display_; }
  
 private:
  Display display_;
};

class DisplayChangedEvent : public TypedEvent<DisplayChangedEvent> {
 public:
  DisplayChangedEvent(const Display& old_display, const Display& new_display)
      : old_display_(old_display), new_display_(new_display) {}
  
  const Display& GetOldDisplay() const { return old_display_; }
  const Display& GetNewDisplay() const { return new_display_; }
  
 private:
  Display old_display_;
  Display new_display_;
};

/**
 * Keyboard-related events
 */
class KeyPressedEvent : public TypedEvent<KeyPressedEvent> {
 public:
  explicit KeyPressedEvent(int keycode) : keycode_(keycode) {}
  
  int GetKeycode() const { return keycode_; }
  
 private:
  int keycode_;
};

class KeyReleasedEvent : public TypedEvent<KeyReleasedEvent> {
 public:
  explicit KeyReleasedEvent(int keycode) : keycode_(keycode) {}
  
  int GetKeycode() const { return keycode_; }
  
 private:
  int keycode_;
};

class ModifierKeysChangedEvent : public TypedEvent<ModifierKeysChangedEvent> {
 public:
  explicit ModifierKeysChangedEvent(uint32_t modifier_keys) 
      : modifier_keys_(modifier_keys) {}
  
  uint32_t GetModifierKeys() const { return modifier_keys_; }
  
 private:
  uint32_t modifier_keys_;
};

/**
 * Mouse-related events
 */
class MouseMovedEvent : public TypedEvent<MouseMovedEvent> {
 public:
  MouseMovedEvent(const Point& old_position, const Point& new_position)
      : old_position_(old_position), new_position_(new_position) {}
  
  const Point& GetOldPosition() const { return old_position_; }
  const Point& GetNewPosition() const { return new_position_; }
  
 private:
  Point old_position_;
  Point new_position_;
};

class MouseClickedEvent : public TypedEvent<MouseClickedEvent> {
 public:
  enum class Button { Left, Right, Middle, X1, X2 };
  
  MouseClickedEvent(Button button, const Point& position)
      : button_(button), position_(position) {}
  
  Button GetButton() const { return button_; }
  const Point& GetPosition() const { return position_; }
  
 private:
  Button button_;
  Point position_;
};

/**
 * Window-related events
 */
class WindowCreatedEvent : public TypedEvent<WindowCreatedEvent> {
 public:
  explicit WindowCreatedEvent(void* window_handle) 
      : window_handle_(window_handle) {}
  
  void* GetWindowHandle() const { return window_handle_; }
  
 private:
  void* window_handle_;
};

class WindowClosedEvent : public TypedEvent<WindowClosedEvent> {
 public:
  explicit WindowClosedEvent(void* window_handle) 
      : window_handle_(window_handle) {}
  
  void* GetWindowHandle() const { return window_handle_; }
  
 private:
  void* window_handle_;
};

/**
 * Application-related events
 */
class ApplicationStartedEvent : public TypedEvent<ApplicationStartedEvent> {
 public:
  ApplicationStartedEvent() = default;
};

class ApplicationExitingEvent : public TypedEvent<ApplicationExitingEvent> {
 public:
  explicit ApplicationExitingEvent(int exit_code = 0) : exit_code_(exit_code) {}
  
  int GetExitCode() const { return exit_code_; }
  
 private:
  int exit_code_;
};

}  // namespace nativeapi