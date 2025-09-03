#include "keyboard_monitor_c.h"
#include "../keyboard_monitor.h"
#include "../keyboard_event.h"
#include <memory>

// Internal structure to hold C API state
struct native_keyboard_monitor_t {
  std::unique_ptr<nativeapi::KeyboardMonitor> cpp_monitor;
  
  // Store C callback functions and user data
  native_key_pressed_callback_t on_key_pressed;
  native_key_released_callback_t on_key_released;
  native_modifier_keys_changed_callback_t on_modifier_keys_changed;
  void* user_data;
  
  // Store listener IDs for cleanup
  size_t key_pressed_listener_id;
  size_t key_released_listener_id;
  size_t modifier_keys_listener_id;
  
  native_keyboard_monitor_t() 
    : cpp_monitor(nullptr), 
      on_key_pressed(nullptr),
      on_key_released(nullptr),
      on_modifier_keys_changed(nullptr),
      user_data(nullptr),
      key_pressed_listener_id(0),
      key_released_listener_id(0),
      modifier_keys_listener_id(0) {}
};

// Helper function to convert C++ ModifierKey enum to C enum
static uint32_t convert_modifier_keys_to_c(uint32_t cpp_modifier_keys) {
  uint32_t c_modifier_keys = 0;
  
  if (cpp_modifier_keys & static_cast<uint32_t>(nativeapi::ModifierKey::Shift)) {
    c_modifier_keys |= NATIVE_MODIFIER_KEY_SHIFT;
  }
  if (cpp_modifier_keys & static_cast<uint32_t>(nativeapi::ModifierKey::Ctrl)) {
    c_modifier_keys |= NATIVE_MODIFIER_KEY_CTRL;
  }
  if (cpp_modifier_keys & static_cast<uint32_t>(nativeapi::ModifierKey::Alt)) {
    c_modifier_keys |= NATIVE_MODIFIER_KEY_ALT;
  }
  if (cpp_modifier_keys & static_cast<uint32_t>(nativeapi::ModifierKey::Meta)) {
    c_modifier_keys |= NATIVE_MODIFIER_KEY_META;
  }
  if (cpp_modifier_keys & static_cast<uint32_t>(nativeapi::ModifierKey::Fn)) {
    c_modifier_keys |= NATIVE_MODIFIER_KEY_FN;
  }
  if (cpp_modifier_keys & static_cast<uint32_t>(nativeapi::ModifierKey::CapsLock)) {
    c_modifier_keys |= NATIVE_MODIFIER_KEY_CAPS_LOCK;
  }
  if (cpp_modifier_keys & static_cast<uint32_t>(nativeapi::ModifierKey::NumLock)) {
    c_modifier_keys |= NATIVE_MODIFIER_KEY_NUM_LOCK;
  }
  if (cpp_modifier_keys & static_cast<uint32_t>(nativeapi::ModifierKey::ScrollLock)) {
    c_modifier_keys |= NATIVE_MODIFIER_KEY_SCROLL_LOCK;
  }
  
  return c_modifier_keys;
}

native_keyboard_monitor_t* native_keyboard_monitor_create(void) {
  try {
    native_keyboard_monitor_t* monitor = new native_keyboard_monitor_t();
    monitor->cpp_monitor = std::make_unique<nativeapi::KeyboardMonitor>();
    return monitor;
  } catch (...) {
    return nullptr;
  }
}

void native_keyboard_monitor_destroy(native_keyboard_monitor_t* monitor) {
  if (!monitor) {
    return;
  }
  
  // Stop monitoring first if it's active
  if (monitor->cpp_monitor && monitor->cpp_monitor->IsMonitoring()) {
    monitor->cpp_monitor->Stop();
  }
  
  delete monitor;
}

bool native_keyboard_monitor_set_callbacks(
    native_keyboard_monitor_t* monitor,
    native_key_pressed_callback_t on_key_pressed,
    native_key_released_callback_t on_key_released,
    native_modifier_keys_changed_callback_t on_modifier_keys_changed,
    void* user_data) {
  
  if (!monitor || !monitor->cpp_monitor) {
    return false;
  }
  
  // Remove existing listeners if any
  if (monitor->key_pressed_listener_id > 0) {
    monitor->cpp_monitor->RemoveListener(monitor->key_pressed_listener_id);
  }
  if (monitor->key_released_listener_id > 0) {
    monitor->cpp_monitor->RemoveListener(monitor->key_released_listener_id);
  }
  if (monitor->modifier_keys_listener_id > 0) {
    monitor->cpp_monitor->RemoveListener(monitor->modifier_keys_listener_id);
  }
  
  // Store the C callbacks and user data
  monitor->on_key_pressed = on_key_pressed;
  monitor->on_key_released = on_key_released;
  monitor->on_modifier_keys_changed = on_modifier_keys_changed;
  monitor->user_data = user_data;
  
  try {
    // Add event listeners with callback lambdas
    if (on_key_pressed) {
      monitor->key_pressed_listener_id = monitor->cpp_monitor->AddListener<nativeapi::KeyPressedEvent>(
        [monitor](const nativeapi::KeyPressedEvent& event) {
          monitor->on_key_pressed(event.GetKeycode(), monitor->user_data);
        }
      );
    }
    
    if (on_key_released) {
      monitor->key_released_listener_id = monitor->cpp_monitor->AddListener<nativeapi::KeyReleasedEvent>(
        [monitor](const nativeapi::KeyReleasedEvent& event) {
          monitor->on_key_released(event.GetKeycode(), monitor->user_data);
        }
      );
    }
    
    if (on_modifier_keys_changed) {
      monitor->modifier_keys_listener_id = monitor->cpp_monitor->AddListener<nativeapi::ModifierKeysChangedEvent>(
        [monitor](const nativeapi::ModifierKeysChangedEvent& event) {
          uint32_t c_modifier_keys = convert_modifier_keys_to_c(event.GetModifierKeys());
          monitor->on_modifier_keys_changed(c_modifier_keys, monitor->user_data);
        }
      );
    }
    
    return true;
  } catch (...) {
    return false;
  }
}

bool native_keyboard_monitor_start(native_keyboard_monitor_t* monitor) {
  if (!monitor || !monitor->cpp_monitor) {
    return false;
  }
  
  try {
    monitor->cpp_monitor->Start();
    return true;
  } catch (...) {
    return false;
  }
}

bool native_keyboard_monitor_stop(native_keyboard_monitor_t* monitor) {
  if (!monitor || !monitor->cpp_monitor) {
    return false;
  }
  
  try {
    monitor->cpp_monitor->Stop();
    return true;
  } catch (...) {
    return false;
  }
}

bool native_keyboard_monitor_is_monitoring(const native_keyboard_monitor_t* monitor) {
  if (!monitor || !monitor->cpp_monitor) {
    return false;
  }
  
  try {
    return monitor->cpp_monitor->IsMonitoring();
  } catch (...) {
    return false;
  }
}