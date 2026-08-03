// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

#include "keyboard_c.h"

#include <cstdio>
#include <memory>
#include <new>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "string_utils_c.h"
#include "../foundation/handle_table.h"
#include "../foundation/keyboard.h"

namespace {

native_modifier_key_t ToCModifierKey(nativeapi::ModifierKey value) {
  switch (value) {
    case nativeapi::ModifierKey::None:
      return NATIVE_MODIFIER_KEY_NONE;
    case nativeapi::ModifierKey::Shift:
      return NATIVE_MODIFIER_KEY_SHIFT;
    case nativeapi::ModifierKey::Ctrl:
      return NATIVE_MODIFIER_KEY_CTRL;
    case nativeapi::ModifierKey::Alt:
      return NATIVE_MODIFIER_KEY_ALT;
    case nativeapi::ModifierKey::Meta:
      return NATIVE_MODIFIER_KEY_META;
    case nativeapi::ModifierKey::Fn:
      return NATIVE_MODIFIER_KEY_FN;
    case nativeapi::ModifierKey::CapsLock:
      return NATIVE_MODIFIER_KEY_CAPS_LOCK;
    case nativeapi::ModifierKey::NumLock:
      return NATIVE_MODIFIER_KEY_NUM_LOCK;
    case nativeapi::ModifierKey::ScrollLock:
      return NATIVE_MODIFIER_KEY_SCROLL_LOCK;
    default:
      return NATIVE_MODIFIER_KEY_NONE;
  }
}

nativeapi::ModifierKey ToCppModifierKey(native_modifier_key_t value) {
  switch (value) {
    case NATIVE_MODIFIER_KEY_NONE:
      return nativeapi::ModifierKey::None;
    case NATIVE_MODIFIER_KEY_SHIFT:
      return nativeapi::ModifierKey::Shift;
    case NATIVE_MODIFIER_KEY_CTRL:
      return nativeapi::ModifierKey::Ctrl;
    case NATIVE_MODIFIER_KEY_ALT:
      return nativeapi::ModifierKey::Alt;
    case NATIVE_MODIFIER_KEY_META:
      return nativeapi::ModifierKey::Meta;
    case NATIVE_MODIFIER_KEY_FN:
      return nativeapi::ModifierKey::Fn;
    case NATIVE_MODIFIER_KEY_CAPS_LOCK:
      return nativeapi::ModifierKey::CapsLock;
    case NATIVE_MODIFIER_KEY_NUM_LOCK:
      return nativeapi::ModifierKey::NumLock;
    case NATIVE_MODIFIER_KEY_SCROLL_LOCK:
      return nativeapi::ModifierKey::ScrollLock;
    default:
      return nativeapi::ModifierKey::None;
  }
}

native_keyboard_accelerator_t ToCKeyboardAccelerator(const nativeapi::KeyboardAccelerator& value) {
  native_keyboard_accelerator_t result = {};
  result.modifiers = ToCModifierKey(value.modifiers);
  result.key = to_c_str(value.key);
  return result;
}

nativeapi::KeyboardAccelerator ToCppKeyboardAccelerator(const native_keyboard_accelerator_t& value) {
  nativeapi::KeyboardAccelerator result = {};
  result.modifiers = ToCppModifierKey(value.modifiers);
  result.key = value.key ? value.key : "";
  return result;
}

}  // namespace

char* native_keyboard_accelerator_to_string(native_keyboard_accelerator_t keyboard_accelerator) {
  try {
    const auto self = ToCppKeyboardAccelerator(keyboard_accelerator);
    return to_c_str(self.ToString());
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_keyboard_accelerator_to_string");
    return nullptr;
  }
}

bool native_keyboard_accelerator_is_empty(native_keyboard_accelerator_t keyboard_accelerator) {
  try {
    const auto self = ToCppKeyboardAccelerator(keyboard_accelerator);
    return self.IsEmpty();
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_keyboard_accelerator_is_empty");
    return false;
  }
}

void native_keyboard_accelerator_free(native_keyboard_accelerator_t* value) {
  if (!value) {
    return;
  }
  free_c_str(value->key);
  value->key = nullptr;
}

bool ToCKeyboardEvent(const nativeapi::KeyboardEvent& event, native_keyboard_event_t* out) {
  if (!out) {
    return false;
  }
  *out = native_keyboard_event_t{};
  out->keycode = event.GetKeycode();
  if (const auto* typed = dynamic_cast<const nativeapi::KeyPressedEvent*>(&event)) {
    out->type = NATIVE_KEYBOARD_EVENT_TYPE_KEY_PRESSED;
    (void)typed;
    return true;
  }
  if (const auto* typed = dynamic_cast<const nativeapi::KeyReleasedEvent*>(&event)) {
    out->type = NATIVE_KEYBOARD_EVENT_TYPE_KEY_RELEASED;
    (void)typed;
    return true;
  }
  if (const auto* typed = dynamic_cast<const nativeapi::ModifierKeysChangedEvent*>(&event)) {
    out->type = NATIVE_KEYBOARD_EVENT_TYPE_MODIFIER_KEYS_CHANGED;
    out->data.modifier_keys_changed.modifier_keys = typed->GetModifierKeys();
    return true;
  }
  return false;
}

void FreeCKeyboardEvent(native_keyboard_event_t* value) {
  if (!value) {
    return;
  }
}

