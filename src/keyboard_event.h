#pragma once
#include <string>
#include <typeindex>

#include "foundation/event.h"

namespace nativeapi {

/**
 * Base class for all keyboard-related events
 *
 * This class provides common functionality for keyboard events,
 * including access to the keycode that triggered the event.
 */
class KeyboardEvent : public Event {
 public:
  /**
   * Constructor for KeyboardEvent
   * @param keycode The keycode associated with this event
   */
  explicit KeyboardEvent(int keycode) : keycode_(keycode) {}

  /**
   * Virtual destructor
   */
  virtual ~KeyboardEvent() = default;

  /**
   * Get the keycode associated with this event
   * @return The keycode value
   */
  int GetKeycode() const { return keycode_; }

  /**
   * Get a string representation of the event type (for debugging)
   * Default implementation returns "KeyboardEvent"
   */
  std::string GetTypeName() const override { return "KeyboardEvent"; }

 private:
  int keycode_;
};

/**
 * Event class for key press
 *
 * This event is emitted when a key is pressed down.
 */
class KeyPressedEvent : public KeyboardEvent {
 public:
  explicit KeyPressedEvent(int keycode) : KeyboardEvent(keycode) {}

  /**
   * Get a string representation of the event type
   */
  std::string GetTypeName() const override { return "KeyPressedEvent"; }

  /**
   * Get the static type index for this event type
   */
  static std::type_index GetStaticType() {
    return std::type_index(typeid(KeyPressedEvent));
  }

  /**
   * Get the type index for this event instance
   */
  std::type_index GetType() const {
    return GetStaticType();
  }
};

/**
 * Event class for key release
 *
 * This event is emitted when a key is released.
 */
class KeyReleasedEvent : public KeyboardEvent {
 public:
  explicit KeyReleasedEvent(int keycode) : KeyboardEvent(keycode) {}

  /**
   * Get a string representation of the event type
   */
  std::string GetTypeName() const override { return "KeyReleasedEvent"; }

  /**
   * Get the static type index for this event type
   */
  static std::type_index GetStaticType() {
    return std::type_index(typeid(KeyReleasedEvent));
  }

  /**
   * Get the type index for this event instance
   */
  std::type_index GetType() const {
    return GetStaticType();
  }
};

/**
 * Event class for modifier keys change
 *
 * This event is emitted when modifier keys (Ctrl, Alt, Shift, etc.) change state.
 */
class ModifierKeysChangedEvent : public KeyboardEvent {
 public:
  explicit ModifierKeysChangedEvent(uint32_t modifier_keys)
      : KeyboardEvent(0), modifier_keys_(modifier_keys) {}

  /**
   * Get the modifier keys state
   * @return The modifier keys bitmask
   */
  uint32_t GetModifierKeys() const { return modifier_keys_; }

  /**
   * Get a string representation of the event type
   */
  std::string GetTypeName() const override { return "ModifierKeysChangedEvent"; }

  /**
   * Get the static type index for this event type
   */
  static std::type_index GetStaticType() {
    return std::type_index(typeid(ModifierKeysChangedEvent));
  }

  /**
   * Get the type index for this event instance
   */
  std::type_index GetType() const {
    return GetStaticType();
  }

 private:
  uint32_t modifier_keys_;
};

}  // namespace nativeapi