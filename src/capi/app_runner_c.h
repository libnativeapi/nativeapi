#pragma once

#include <stdbool.h>
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
 * App runner exit codes
 */
typedef enum {
  NATIVE_APP_EXIT_SUCCESS = 0,
  NATIVE_APP_EXIT_FAILURE = 1,
  NATIVE_APP_EXIT_INVALID_WINDOW = 2
} native_app_exit_code_t;

/**
 * Run the application with the specified window.
 * This function starts the main event loop and blocks until the application
 * exits.
 *
 * @param window The window to run the application with (must not be NULL)
 * @return Exit code of the application (0 for success)
 */
FFI_PLUGIN_EXPORT
int native_app_runner_run(native_window_t window);

/**
 * Check if the application is currently running.
 *
 * @return true if the app is running, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_app_runner_is_running(void);

#ifdef __cplusplus
}
#endif
