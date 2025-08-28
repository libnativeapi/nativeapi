#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Modifier key enumeration for C API
 */
typedef enum {
  NATIVE_MODIFIER_KEY_NONE = 0,
  NATIVE_MODIFIER_KEY_SHIFT = 1 << 0,
  NATIVE_MODIFIER_KEY_CTRL = 1 << 1,
  NATIVE_MODIFIER_KEY_ALT = 1 << 2,
  NATIVE_MODIFIER_KEY_META = 1 << 3,  // Windows key or Cmd key
  NATIVE_MODIFIER_KEY_FN = 1 << 4,
  NATIVE_MODIFIER_KEY_CAPS_LOCK = 1 << 5,
  NATIVE_MODIFIER_KEY_NUM_LOCK = 1 << 6,
  NATIVE_MODIFIER_KEY_SCROLL_LOCK = 1 << 7
} native_modifier_key_t;

/**
 * Callback function type for key pressed events
 * @param keycode The key code that was pressed
 * @param user_data User-provided data passed to the callback
 */
typedef void (*native_key_pressed_callback_t)(int keycode, void* user_data);

/**
 * Callback function type for key released events
 * @param keycode The key code that was released
 * @param user_data User-provided data passed to the callback
 */
typedef void (*native_key_released_callback_t)(int keycode, void* user_data);

/**
 * Callback function type for modifier keys changed events
 * @param modifier_keys Bitwise OR of active modifier keys
 * @param user_data User-provided data passed to the callback
 */
typedef void (*native_modifier_keys_changed_callback_t)(uint32_t modifier_keys,
                                                        void* user_data);

/**
 * Opaque handle to keyboard monitor instance
 */
typedef struct native_keyboard_monitor_t native_keyboard_monitor_t;

/**
 * Create a new keyboard monitor instance
 * @return Pointer to keyboard monitor instance, or NULL on failure
 */
native_keyboard_monitor_t* native_keyboard_monitor_create(void);

/**
 * Destroy a keyboard monitor instance
 * @param monitor Pointer to keyboard monitor instance to destroy
 */
void native_keyboard_monitor_destroy(native_keyboard_monitor_t* monitor);

/**
 * Set callback functions for keyboard events
 * @param monitor Pointer to keyboard monitor instance
 * @param on_key_pressed Callback for key pressed events (can be NULL)
 * @param on_key_released Callback for key released events (can be NULL)
 * @param on_modifier_keys_changed Callback for modifier keys changed events
 * (can be NULL)
 * @param user_data User data to pass to callbacks
 * @return true on success, false on failure
 */
bool native_keyboard_monitor_set_callbacks(
    native_keyboard_monitor_t* monitor,
    native_key_pressed_callback_t on_key_pressed,
    native_key_released_callback_t on_key_released,
    native_modifier_keys_changed_callback_t on_modifier_keys_changed,
    void* user_data);

/**
 * Start keyboard monitoring
 * @param monitor Pointer to keyboard monitor instance
 * @return true on success, false on failure
 */
bool native_keyboard_monitor_start(native_keyboard_monitor_t* monitor);

/**
 * Stop keyboard monitoring
 * @param monitor Pointer to keyboard monitor instance
 * @return true on success, false on failure
 */
bool native_keyboard_monitor_stop(native_keyboard_monitor_t* monitor);

/**
 * Check if keyboard monitoring is active
 * @param monitor Pointer to keyboard monitor instance
 * @return true if monitoring is active, false otherwise
 */
bool native_keyboard_monitor_is_monitoring(
    const native_keyboard_monitor_t* monitor);

#ifdef __cplusplus
}
#endif