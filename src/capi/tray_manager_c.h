#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if _WIN32
#define FFI_PLUGIN_EXPORT __declspec(dllexport)
#else
#define FFI_PLUGIN_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "tray_icon_c.h"

/**
 * Tray icon list structure
 */
typedef struct {
  native_tray_icon_t* tray_icons;
  size_t count;
} native_tray_icon_list_t;

/**
 * TrayManager singleton operations
 */

/**
 * Check if system tray is supported on the current platform
 * @return true if system tray is supported, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_tray_manager_is_supported(void);

/**
 * Create a new tray icon
 * @return Tray icon handle, or NULL if creation failed
 */
FFI_PLUGIN_EXPORT
native_tray_icon_t native_tray_manager_create(void);

/**
 * Get a tray icon by its ID
 * @param tray_icon_id The tray icon ID
 * @return Tray icon handle, or NULL if not found
 */
FFI_PLUGIN_EXPORT
native_tray_icon_t native_tray_manager_get(native_tray_icon_id_t tray_icon_id);

/**
 * Get all managed tray icons
 * @return List of all tray icons (caller must free with native_tray_icon_list_free)
 */
FFI_PLUGIN_EXPORT
native_tray_icon_list_t native_tray_manager_get_all(void);

/**
 * Destroy a tray icon by its ID
 * @param tray_icon_id The tray icon ID to destroy
 * @return true if tray icon was found and destroyed, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_tray_manager_destroy(native_tray_icon_id_t tray_icon_id);

/**
 * Utility functions
 */

/**
 * Free a tray icon list
 * @param list The list to free
 */
FFI_PLUGIN_EXPORT
void native_tray_icon_list_free(native_tray_icon_list_t list);

#ifdef __cplusplus
}
#endif