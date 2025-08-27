#pragma once

#include "event.h"
#include "keyboard_monitor.h"

namespace nativeapi {

/**
 * Event fired when a key is pressed.
 */
class KeyPressedEvent : public TypedEvent<KeyPressedEvent> {
 public:
  explicit KeyPressedEvent(int keycode) : keycode_(keycode) {}

  int GetKeycode() const { return keycode_; }

  std::string GetTypeName() const override { return "KeyPressedEvent"; }

 private:
  int keycode_;
};

/**
 * Event fired when a key is released.
 */
class KeyReleasedEvent : public TypedEvent<KeyReleasedEvent> {
 public:
  explicit KeyReleasedEvent(int keycode) : keycode_(keycode) {}

  int GetKeycode() const { return keycode_; }

  std::string GetTypeName() const override { return "KeyReleasedEvent"; }

 private:
  int keycode_;
};

/**
 * Event fired when modifier keys change.
 */
class ModifierKeysChangedEvent : public TypedEvent<ModifierKeysChangedEvent> {
 public:
  explicit ModifierKeysChangedEvent(uint32_t modifier_keys) : modifier_keys_(modifier_keys) {}

  uint32_t GetModifierKeys() const { return modifier_keys_; }

  std::string GetTypeName() const override { return "ModifierKeysChangedEvent"; }

 private:
  uint32_t modifier_keys_;
};

}  // namespace nativeapi