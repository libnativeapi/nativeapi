#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

/**
 * @file autostart_c.h
 * @brief C API for managing application auto-start at user login.
 *
 * This API provides a C interface around the cross-platform AutoStart facility.
 * It enables enabling/disabling auto-start behavior, configuring the target
 * executable and arguments, and querying state.
 *
 * Platform behavior:
 * - Windows: HKCU\Software\Microsoft\Windows\CurrentVersion\Run registry value
 * - macOS:   Launch Agents in ~/Library/LaunchAgents (plist with ProgramArguments)
 * - Linux:   XDG autostart in ~/.config/autostart (Desktop Entry Exec line)
 * - Mobile (Android/iOS/OHOS): Typically unsupported (functions return false)
 *
 * Notes:
 * - For string-returning functions (char*), use free_c_str() from string_utils_c.h to free.
 * - For string-array-returning functions, free each string with free_c_str() and the array with
 * delete[].
 */

/**
 * Opaque handle type for AutoStart instance.
 */
typedef void* native_autostart_t;

/**
 * Create an AutoStart manager with defaults derived from the current application.
 *
 * @return Handle to an AutoStart instance, or NULL on failure.
 */
native_autostart_t native_autostart_create(void);

/**
 * Create an AutoStart manager with a custom identifier.
 *
 * @param id A stable, unique identifier for your app (e.g., "com.example.myapp").
 * @return Handle to an AutoStart instance, or NULL on failure.
 */
native_autostart_t native_autostart_create_with_id(const char* id);

/**
 * Create an AutoStart manager with a custom identifier and display name.
 *
 * @param id           Stable, unique identifier (e.g., "com.example.myapp").
 * @param display_name Human-readable application name (e.g., "MyApp").
 * @return Handle to an AutoStart instance, or NULL on failure.
 */
native_autostart_t native_autostart_create_with_id_and_name(const char* id,
                                                            const char* display_name);

/**
 * Destroy an AutoStart instance and release resources.
 *
 * @param autostart Handle returned by a native_autostart_create* function.
 */
void native_autostart_destroy(native_autostart_t autostart);

/**
 * Check if auto-start management is supported on the current platform.
 *
 * This is a static check that does not require an AutoStart instance.
 *
 * @return true if supported, false if unsupported.
 */
bool native_autostart_is_supported(void);

/**
 * Get the identifier associated with this AutoStart instance.
 *
 * @param autostart AutoStart handle.
 * @return Newly allocated string with the identifier, or NULL on error.
 *         Caller must free with free_c_str().
 */
char* native_autostart_get_id(native_autostart_t autostart);

/**
 * Get the human-readable display name associated with this AutoStart instance.
 *
 * @param autostart AutoStart handle.
 * @return Newly allocated string with the display name, or NULL on error.
 *         Caller must free with free_c_str().
 */
char* native_autostart_get_display_name(native_autostart_t autostart);

/**
 * Set the human-readable display name used where applicable.
 *
 * @param autostart    AutoStart handle.
 * @param display_name The display name to set.
 * @return true if stored successfully; does not change OS registration until Enable().
 */
bool native_autostart_set_display_name(native_autostart_t autostart, const char* display_name);

/**
 * Set the program (executable) path and optional arguments to use for auto-start.
 *
 * If not set, the implementation attempts to use the current process executable.
 * Pass NULL for arguments or argument_count == 0 when no arguments are needed.
 *
 * @param autostart       AutoStart handle.
 * @param executable_path Absolute path to the executable to launch on login.
 * @param arguments       Array of argument strings (can be NULL if count is 0).
 * @param argument_count  Number of strings in the arguments array.
 * @return true if stored successfully; does not change OS registration until Enable().
 */
bool native_autostart_set_program(native_autostart_t autostart,
                                  const char* executable_path,
                                  const char* const* arguments,
                                  size_t argument_count);

/**
 * Get the currently configured executable path.
 *
 * @param autostart AutoStart handle.
 * @return Newly allocated string with the executable path, or NULL on error.
 *         Caller must free with free_c_str().
 */
char* native_autostart_get_executable_path(native_autostart_t autostart);

/**
 * Get the currently configured arguments for auto-start.
 *
 * @param autostart     AutoStart handle.
 * @param out_arguments Output pointer to an array of newly allocated strings.
 * @param out_count     Output pointer to receive the number of arguments.
 * @return true on success; false on error. On success, free each string with
 *         free_c_str() and the array with delete[].
 */
bool native_autostart_get_arguments(native_autostart_t autostart,
                                    char*** out_arguments,
                                    size_t* out_count);

/**
 * Enable auto-start at user login for the configured program and arguments.
 *
 * @param autostart AutoStart handle.
 * @return true on success, false on failure or if unsupported.
 */
bool native_autostart_enable(native_autostart_t autostart);

/**
 * Disable auto-start at user login.
 *
 * @param autostart AutoStart handle.
 * @return true on success, false on failure or if unsupported.
 */
bool native_autostart_disable(native_autostart_t autostart);

/**
 * Check whether auto-start is currently enabled for this identifier.
 *
 * @param autostart AutoStart handle.
 * @return true if enabled, false otherwise or on error.
 */
bool native_autostart_is_enabled(native_autostart_t autostart);

#ifdef __cplusplus
}
#endif