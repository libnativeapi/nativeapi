#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

/**
 * @file launch_at_login_c.h
 * @brief C API for managing launching the application at user login.
 *
 * This API provides a C interface around the cross-platform LaunchAtLogin facility.
 * It enables enabling/disabling launch-at-login behavior, configuring the target
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
 * Opaque handle type for LaunchAtLogin instance.
 */
typedef void* native_launch_at_login_t;

/**
 * Create a LaunchAtLogin manager with defaults derived from the current application.
 *
 * @return Handle to a LaunchAtLogin instance, or NULL on failure.
 */
native_launch_at_login_t native_launch_at_login_create(void);

/**
 * Create a LaunchAtLogin manager with a custom identifier.
 *
 * @param id A stable, unique identifier for your app (e.g., "com.example.myapp").
 * @return Handle to a LaunchAtLogin instance, or NULL on failure.
 */
native_launch_at_login_t native_launch_at_login_create_with_id(const char* id);

/**
 * Create a LaunchAtLogin manager with a custom identifier and display name.
 *
 * @param id           Stable, unique identifier (e.g., "com.example.myapp").
 * @param display_name Human-readable application name (e.g., "MyApp").
 * @return Handle to a LaunchAtLogin instance, or NULL on failure.
 */
native_launch_at_login_t native_launch_at_login_create_with_id_and_name(const char* id,
                                                                        const char* display_name);

/**
 * Destroy a LaunchAtLogin instance and release resources.
 *
 * @param launch_at_login Handle returned by a native_launch_at_login_create* function.
 */
void native_launch_at_login_destroy(native_launch_at_login_t launch_at_login);

/**
 * Check if launch-at-login management is supported on the current platform.
 *
 * This is a static check that does not require a LaunchAtLogin instance.
 *
 * @return true if supported, false if unsupported.
 */
bool native_launch_at_login_is_supported(void);

/**
 * Get the identifier associated with this LaunchAtLogin instance.
 *
 * @param launch_at_login LaunchAtLogin handle.
 * @return Newly allocated string with the identifier, or NULL on error.
 *         Caller must free with free_c_str().
 */
char* native_launch_at_login_get_id(native_launch_at_login_t launch_at_login);

/**
 * Get the human-readable display name associated with this LaunchAtLogin instance.
 *
 * @param launch_at_login LaunchAtLogin handle.
 * @return Newly allocated string with the display name, or NULL on error.
 *         Caller must free with free_c_str().
 */
char* native_launch_at_login_get_display_name(native_launch_at_login_t launch_at_login);

/**
 * Set the human-readable display name used where applicable.
 *
 * @param launch_at_login    LaunchAtLogin handle.
 * @param display_name The display name to set.
 * @return true if stored successfully; does not change OS registration until Enable().
 */
bool native_launch_at_login_set_display_name(native_launch_at_login_t launch_at_login,
                                             const char* display_name);

/**
 * Set the program (executable) path and optional arguments to use when launching at login.
 *
 * If not set, the implementation attempts to use the current process executable.
 * Pass NULL for arguments or argument_count == 0 when no arguments are needed.
 *
 * @param launch_at_login       LaunchAtLogin handle.
 * @param executable_path Absolute path to the executable to launch on login.
 * @param arguments       Array of argument strings (can be NULL if count is 0).
 * @param argument_count  Number of strings in the arguments array.
 * @return true if stored successfully; does not change OS registration until Enable().
 */
bool native_launch_at_login_set_program(native_launch_at_login_t launch_at_login,
                                        const char* executable_path,
                                        const char* const* arguments,
                                        size_t argument_count);

/**
 * Get the currently configured executable path.
 *
 * @param launch_at_login LaunchAtLogin handle.
 * @return Newly allocated string with the executable path, or NULL on error.
 *         Caller must free with free_c_str().
 */
char* native_launch_at_login_get_executable_path(native_launch_at_login_t launch_at_login);

/**
 * Get the currently configured arguments for launch-at-login.
 *
 * @param launch_at_login     LaunchAtLogin handle.
 * @param out_arguments Output pointer to an array of newly allocated strings.
 * @param out_count     Output pointer to receive the number of arguments.
 * @return true on success; false on error. On success, free each string with
 *         free_c_str() and the array with delete[].
 */
bool native_launch_at_login_get_arguments(native_launch_at_login_t launch_at_login,
                                          char*** out_arguments,
                                          size_t* out_count);

/**
 * Enable launch-at-login for the configured program and arguments.
 *
 * @param launch_at_login LaunchAtLogin handle.
 * @return true on success, false on failure or if unsupported.
 */
bool native_launch_at_login_enable(native_launch_at_login_t launch_at_login);

/**
 * Disable launch-at-login.
 *
 * @param launch_at_login LaunchAtLogin handle.
 * @return true on success, false on failure or if unsupported.
 */
bool native_launch_at_login_disable(native_launch_at_login_t launch_at_login);

/**
 * Check whether launch-at-login is currently enabled for this identifier.
 *
 * @param launch_at_login LaunchAtLogin handle.
 * @return true if enabled, false otherwise or on error.
 */
bool native_launch_at_login_is_enabled(native_launch_at_login_t launch_at_login);

#ifdef __cplusplus
}
#endif
