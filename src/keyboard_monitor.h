#pragma once

#include <functional>
#include <memory>
#include <string>

namespace nativeapi {

// KeyboardEventHandler uses callbacks to handle keyboard events.
class KeyboardEventHandler {
 public:
  // Constructor that takes callbacks for keyboard events
  KeyboardEventHandler(std::function<void(int)> onKeyPressedCallback,
                       std::function<void(int)> onKeyReleasedCallback);

  // Handle key pressed event
  void OnKeyPressed(int keycode);

  // Handle key released event
  void OnKeyReleased(int keycode);

 private:
  std::function<void(int)> onKeyPressedCallback_;
  std::function<void(int)> onKeyReleasedCallback_;
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
  std::unique_ptr<Impl> impl_;
  KeyboardEventHandler* event_handler_;
};

}  // namespace nativeapi
