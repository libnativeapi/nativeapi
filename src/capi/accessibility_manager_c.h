#pragma once

#include <stdbool.h>

#if _WIN32
#define FFI_PLUGIN_EXPORT __declspec(dllexport)
#else
#define FFI_PLUGIN_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enables system accessibility features
 *
 * This function activates accessibility functionality across the system.
 * The operation is idempotent - calling it multiple times has the same
 * effect as calling it once.
 *
 * @note This operation may require system permissions depending on the
 *       platform implementation.
 */
FFI_PLUGIN_EXPORT
void native_accessibility_manager_enable();

/**
 * @brief Checks if accessibility features are currently enabled
 *
 * @return true if accessibility is enabled, false otherwise
 *
 * This function provides a quick way to query the current state of
 * accessibility features without modifying the system state.
 */
FFI_PLUGIN_EXPORT
bool native_accessibility_manager_is_enabled();

#ifdef __cplusplus
}
#endif
