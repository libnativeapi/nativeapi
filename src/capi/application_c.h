#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if _WIN32
#define FFI_PLUGIN_EXPORT __declspec(dllexport)
#else
#define FFI_PLUGIN_EXPORT
#endif

#include "window_c.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque handle for the Application instance
 */
typedef void* native_application_t;

/**
 * @brief Application event types
 */
typedef enum {
  NATIVE_APPLICATION_EVENT_STARTED = 0,
  NATIVE_APPLICATION_EVENT_EXITING = 1,
  NATIVE_APPLICATION_EVENT_ACTIVATED = 2,
  NATIVE_APPLICATION_EVENT_DEACTIVATED = 3,
  NATIVE_APPLICATION_EVENT_QUIT_REQUESTED = 4
} native_application_event_type_t;

/**
 * @brief Application event structure
 */
typedef struct {
  native_application_event_type_t type;
  int exit_code;  // For EXITING event
} native_application_event_t;

/**
 * @brief Application event callback function type
 */
typedef void (*native_application_event_callback_t)(const native_application_event_t* event);

/**
 * @brief Get the singleton instance of Application
 *
 * @return Handle to the singleton Application instance
 */
FFI_PLUGIN_EXPORT
native_application_t native_application_get_instance(void);

/**
 * @brief Run the application main event loop
 *
 * @param app Application handle (must not be NULL)
 * @return Exit code of the application (0 for success)
 */
FFI_PLUGIN_EXPORT
int native_application_run(native_application_t app);

/**
 * @brief Run the application with the specified window
 *
 * @param app Application handle (must not be NULL)
 * @param window Window handle (must not be NULL)
 * @return Exit code of the application (0 for success)
 */
FFI_PLUGIN_EXPORT
int native_application_run_with_window(native_application_t app, native_window_t window);

/**
 * @brief Request the application to quit
 *
 * @param app Application handle (must not be NULL)
 * @param exit_code The exit code to use when quitting (default: 0)
 */
FFI_PLUGIN_EXPORT
void native_application_quit(native_application_t app, int exit_code);

/**
 * @brief Check if the application is currently running
 *
 * @param app Application handle (must not be NULL)
 * @return true if the application is running, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_application_is_running(native_application_t app);

/**
 * @brief Check if this is a single instance application
 *
 * @param app Application handle (must not be NULL)
 * @return true if only one instance is allowed, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_application_is_single_instance(native_application_t app);

/**
 * @brief Set the application icon
 *
 * @param app Application handle (must not be NULL)
 * @param icon_path Path to the icon file (must not be NULL)
 * @return true if the icon was set successfully, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_application_set_icon(native_application_t app, const char* icon_path);

/**
 * @brief Show or hide the dock icon (macOS only)
 *
 * @param app Application handle (must not be NULL)
 * @param visible true to show the dock icon, false to hide it
 * @return true if the operation succeeded, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_application_set_dock_icon_visible(native_application_t app, bool visible);

/**
 * @brief Add an event listener for application events
 *
 * @param app Application handle (must not be NULL)
 * @param callback Event callback function (must not be NULL)
 * @return Listener ID that can be used to remove the listener, or 0 on failure
 */
FFI_PLUGIN_EXPORT
size_t native_application_add_event_listener(native_application_t app,
                                             native_application_event_callback_t callback);

/**
 * @brief Remove an event listener by ID
 *
 * @param app Application handle (must not be NULL)
 * @param listener_id The ID returned by native_application_add_event_listener
 * @return true if the listener was found and removed, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_application_remove_event_listener(native_application_t app, size_t listener_id);

/**
 * @brief Convenience function to run the application with the specified window
 *
 * This is equivalent to calling native_application_run_with_window with the singleton instance.
 * This function provides a simple way to run an application without explicitly
 * accessing the singleton.
 *
 * @param window Window handle (must not be NULL)
 * @return Exit code of the application (0 for success)
 */
FFI_PLUGIN_EXPORT
int native_run_app(native_window_t window);

#ifdef __cplusplus
}
#endif
