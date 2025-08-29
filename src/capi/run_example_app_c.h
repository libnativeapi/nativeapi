#pragma once

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
 * Run the application with the default window.
 * This function starts the main event loop and blocks until the application
 * exits.
 *
 * @return Exit code of the application (0 for success)
 */
FFI_PLUGIN_EXPORT
int native_run_example_app(void);

#ifdef __cplusplus
}
#endif
