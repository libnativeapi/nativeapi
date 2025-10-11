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
  NATIVE_WINDOW_EVENT_CREATED = 0,
  NATIVE_WINDOW_EVENT_CLOSED = 1,
  NATIVE_WINDOW_EVENT_FOCUSED = 2,
  NATIVE_WINDOW_EVENT_BLURRED = 3,
  NATIVE_WINDOW_EVENT_MINIMIZED = 4,
  NATIVE_WINDOW_EVENT_MAXIMIZED = 5,
  NATIVE_WINDOW_EVENT_RESTORED = 6,
  NATIVE_WINDOW_EVENT_MOVED = 7,
  NATIVE_WINDOW_EVENT_RESIZED = 8
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
 * Create a new window with the specified options
 * @param options Window creation options
 * @return Window handle, or NULL if creation failed
 */
FFI_PLUGIN_EXPORT
native_window_t native_window_manager_create(const native_window_options_t* options);

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
 * Initialize the window manager (called automatically on first use)
 * @return true on success, false on failure
 */
FFI_PLUGIN_EXPORT
bool native_window_manager_initialize(void);

/**
 * Shutdown the window manager and cleanup resources
 */
FFI_PLUGIN_EXPORT
void native_window_manager_shutdown(void);

#ifdef __cplusplus
}
#endif