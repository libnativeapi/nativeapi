#pragma once

#include <functional>
#include <memory>
#include <string>

namespace nativeapi {

enum class ModifierKey : uint32_t {
  None = 0,
  Shift = 1 << 0,
  Ctrl = 1 << 1,
  Alt = 1 << 2,
  Meta = 1 << 3,  // Windows key or Cmd key
  Fn = 1 << 4,
  CapsLock = 1 << 5,
  NumLock = 1 << 6,
  ScrollLock = 1 << 7
};

// KeyboardEventHandler uses callbacks to handle keyboard events.
class KeyboardEventHandler {
 public:
  // Constructor that takes callbacks for keyboard events
  KeyboardEventHandler(
      std::function<void(int)> onKeyPressedCallback,
      std::function<void(int)> onKeyReleasedCallback,
      std::function<void(uint32_t)> onModifierKeysChangedCallback);

  // Handle key pressed event
  void OnKeyPressed(int keycode);

  // Handle key released event
  void OnKeyReleased(int keycode);

  // Handle modifier keys changed event
  void OnModifierKeysChanged(uint32_t modifier_keys);

 private:
  std::function<void(int)> onKeyPressedCallback_;
  std::function<void(int)> onKeyReleasedCallback_;
  std::function<void(uint32_t)> onModifierKeysChangedCallback_;
};

class KeyboardMonitor {
 public:
  KeyboardMonitor();
  virtual ~KeyboardMonitor();

  // Set the event handler for the keyboard monitor
  void SetEventHandler(KeyboardEventHandler* event_handler);

  // Start the keyboard monitor
  void Start();

  // Stop the keyboard monitor
  void Stop();

  // Check if the keyboard monitor is monitoring
  bool IsMonitoring() const;

  // Get the event handler
  KeyboardEventHandler* GetEventHandler() const { return event_handler_; }

 private:
  class Impl;
  friend class Impl;  // Allow Impl class to access private members
  std::unique_ptr<Impl> impl_;
  KeyboardEventHandler* event_handler_;
};

}  // namespace nativeapi
