#include "keyboard_monitor_c.h"
#include "../keyboard_monitor.h"
#include <memory>

// Internal structure to hold C API state
struct native_keyboard_monitor_t {
  std::unique_ptr<nativeapi::KeyboardMonitor> cpp_monitor;
  std::unique_ptr<nativeapi::KeyboardEventHandler> event_handler;
  
  // Store C callback functions and user data
  native_key_pressed_callback_t on_key_pressed;
  native_key_released_callback_t on_key_released;
  native_modifier_keys_changed_callback_t on_modifier_keys_changed;
  void* user_data;
  
  native_keyboard_monitor_t() 
    : cpp_monitor(nullptr), 
      event_handler(nullptr),
      on_key_pressed(nullptr),
      on_key_released(nullptr),
      on_modifier_keys_changed(nullptr),
      user_data(nullptr) {}
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
  
  // Store the C callbacks and user data
  monitor->on_key_pressed = on_key_pressed;
  monitor->on_key_released = on_key_released;
  monitor->on_modifier_keys_changed = on_modifier_keys_changed;
  monitor->user_data = user_data;
  
  try {
    // Create C++ lambda callbacks that call the C callbacks
    auto key_pressed_callback = [monitor](int keycode) {
      if (monitor->on_key_pressed) {
        monitor->on_key_pressed(keycode, monitor->user_data);
      }
    };
    
    auto key_released_callback = [monitor](int keycode) {
      if (monitor->on_key_released) {
        monitor->on_key_released(keycode, monitor->user_data);
      }
    };
    
    auto modifier_keys_changed_callback = [monitor](uint32_t modifier_keys) {
      if (monitor->on_modifier_keys_changed) {
        uint32_t c_modifier_keys = convert_modifier_keys_to_c(modifier_keys);
        monitor->on_modifier_keys_changed(c_modifier_keys, monitor->user_data);
      }
    };
    
    // Create the event handler with the lambda callbacks
    monitor->event_handler = std::make_unique<nativeapi::KeyboardEventHandler>(
        key_pressed_callback,
        key_released_callback,
        modifier_keys_changed_callback);
    
    // Set the event handler on the keyboard monitor
    monitor->cpp_monitor->SetEventHandler(monitor->event_handler.get());
    
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