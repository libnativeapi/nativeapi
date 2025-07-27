#include "keyboard_monitor.h"

namespace nativeapi {

KeyboardEventHandler::KeyboardEventHandler(
    std::function<void(int)> onKeyPressedCallback,
    std::function<void(int)> onKeyReleasedCallback,
    std::function<void(uint32_t)> onModifierKeysChangedCallback)
    : onKeyPressedCallback_(onKeyPressedCallback),
      onKeyReleasedCallback_(onKeyReleasedCallback),
      onModifierKeysChangedCallback_(onModifierKeysChangedCallback) {}

void KeyboardEventHandler::OnKeyPressed(int keycode) {
  if (onKeyPressedCallback_) {
    onKeyPressedCallback_(keycode);
  }
}

void KeyboardEventHandler::OnKeyReleased(int keycode) {
  if (onKeyReleasedCallback_) {
    onKeyReleasedCallback_(keycode);
  }
}

void KeyboardEventHandler::OnModifierKeysChanged(uint32_t modifier_keys) {
  if (onModifierKeysChangedCallback_) {
    onModifierKeysChangedCallback_(modifier_keys);
  }
}

void KeyboardMonitor::SetEventHandler(KeyboardEventHandler* event_handler) {
  event_handler_ = event_handler;
}

}  // namespace nativeapi
