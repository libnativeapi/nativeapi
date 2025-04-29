#include "keyboard_monitor.h"

namespace nativeapi {

KeyboardEventHandler::KeyboardEventHandler(
    std::function<void(const std::string&)> onKeyPressedCallback,
    std::function<void(const std::string&)> onKeyReleasedCallback)
    : onKeyPressedCallback_(onKeyPressedCallback),
      onKeyReleasedCallback_(onKeyReleasedCallback) {}

void KeyboardEventHandler::OnKeyPressed(const std::string& key) {
  if (onKeyPressedCallback_) {
    onKeyPressedCallback_(key);
  }
}

void KeyboardEventHandler::OnKeyReleased(const std::string& key) {
  if (onKeyReleasedCallback_) {
    onKeyReleasedCallback_(key);
  }
}

void KeyboardMonitor::SetEventHandler(KeyboardEventHandler* event_handler) {
  event_handler_ = event_handler;
}

}  // namespace nativeapi
