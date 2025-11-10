#pragma once

#include <stdbool.h>
#include <stdint.h>

#if _WIN32
#define FFI_PLUGIN_EXPORT __declspec(dllexport)
#else
#define FFI_PLUGIN_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "window_c.h"

/**
 * Window event types
 */
typedef enum {
  NATIVE_WINDOW_EVENT_FOCUSED = 0,
  NATIVE_WINDOW_EVENT_BLURRED = 1,
  NATIVE_WINDOW_EVENT_MINIMIZED = 2,
  NATIVE_WINDOW_EVENT_MAXIMIZED = 3,
  NATIVE_WINDOW_EVENT_RESTORED = 4,
  NATIVE_WINDOW_EVENT_MOVED = 5,
  NATIVE_WINDOW_EVENT_RESIZED = 6
} native_window_event_type_t;

/**
 * Window event structure
 */
typedef struct {
  native_window_event_type_t type;
  native_window_id_t window_id;
  union {
    struct {
      native_point_t position;
    } moved;
    struct {
      native_size_t size;
    } resized;
  } data;
} native_window_event_t;

/**
 * Window event callback function type
 */
typedef void (*native_window_event_callback_t)(const native_window_event_t* event, void* user_data);

/**
 * Window manager singleton operations
 */

/**
 * Create a new window with default settings
 * @return Window handle, or NULL if creation failed
 */
FFI_PLUGIN_EXPORT
native_window_t native_window_manager_create(void);

/**
 * Get a window by its ID
 * @param window_id The window ID
 * @return Window handle, or NULL if not found
 */
FFI_PLUGIN_EXPORT
native_window_t native_window_manager_get(native_window_id_t window_id);

/**
 * Get all managed windows
 * @return List of all windows (caller must free with native_window_list_free)
 */
FFI_PLUGIN_EXPORT
native_window_list_t native_window_manager_get_all(void);

/**
 * Get the currently active/focused window
 * @return Current window handle, or NULL if no window is active
 */
FFI_PLUGIN_EXPORT
native_window_t native_window_manager_get_current(void);

/**
 * Destroy a window by its ID
 * @param window_id The window ID to destroy
 * @return true if window was found and destroyed, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_window_manager_destroy(native_window_id_t window_id);

/**
 * Register a callback for window events
 * @param callback The callback function to register
 * @param user_data User data to pass to the callback
 * @return Registration ID, or -1 on failure
 */
FFI_PLUGIN_EXPORT
int native_window_manager_register_event_callback(native_window_event_callback_t callback,
                                                  void* user_data);

/**
 * Unregister a window event callback
 * @param registration_id The registration ID returned by register_event_callback
 * @return true if callback was found and unregistered, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_window_manager_unregister_event_callback(int registration_id);

/**
 * Shutdown the window manager and cleanup resources
 */
FFI_PLUGIN_EXPORT
void native_window_manager_shutdown(void);

/**
 * Hooks called BEFORE a native window is shown/hidden.
 * Passing NULL clears the corresponding hook.
 */
typedef void (*native_window_will_show_callback_t)(native_window_id_t window_id, void* user_data);
typedef void (*native_window_will_hide_callback_t)(native_window_id_t window_id, void* user_data);

/**
 * Set (or clear) the "will show" hook.
 * @param callback Function called before window is shown (e.g., makeKeyAndOrderFront: on macOS).
 * NULL to clear.
 * @param user_data Opaque pointer passed back to callback.
 */
FFI_PLUGIN_EXPORT
void native_window_manager_set_will_show_hook(native_window_will_show_callback_t callback,
                                              void* user_data);

/**
 * Set (or clear) the "will hide" hook.
 * @param callback Function called before window is hidden (e.g., orderOut: on macOS). NULL to
 * clear.
 * @param user_data Opaque pointer passed back to callback.
 */
FFI_PLUGIN_EXPORT
void native_window_manager_set_will_hide_hook(native_window_will_hide_callback_t callback,
                                              void* user_data);

#ifdef __cplusplus
}
#endif