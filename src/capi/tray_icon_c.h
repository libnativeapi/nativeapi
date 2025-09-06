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

#include "geometry_c.h"
#include "menu_c.h"

/**
 * Opaque handle for tray icon objects
 */
typedef void* native_tray_icon_t;

/**
 * Tray icon identifier
 */
typedef long native_tray_icon_id_t;

/**
 * Tray icon clicked event
 */
typedef struct {
  native_tray_icon_id_t tray_icon_id;
  char button[16];  // "left", "right", etc.
} native_tray_icon_clicked_event_t;

/**
 * Tray icon right-clicked event
 */
typedef struct {
  native_tray_icon_id_t tray_icon_id;
} native_tray_icon_right_clicked_event_t;

/**
 * Tray icon double-clicked event
 */
typedef struct {
  native_tray_icon_id_t tray_icon_id;
} native_tray_icon_double_clicked_event_t;

/**
 * Event types for tray icon events
 */
typedef enum {
  NATIVE_TRAY_ICON_EVENT_CLICKED = 0,
  NATIVE_TRAY_ICON_EVENT_RIGHT_CLICKED = 1,
  NATIVE_TRAY_ICON_EVENT_DOUBLE_CLICKED = 2
} native_tray_icon_event_type_t;

/**
 * Event callback function type
 */
typedef void (*native_tray_icon_event_callback_t)(const void* event, void* user_data);

/**
 * TrayIcon operations
 */

/**
 * Create a new tray icon
 * @return Tray icon handle, or NULL if creation failed
 */
FFI_PLUGIN_EXPORT
native_tray_icon_t native_tray_icon_create(void);

/**
 * Create a tray icon from a native platform object
 * @param native_tray Pointer to platform-specific tray icon object
 * @return Tray icon handle, or NULL if creation failed
 */
FFI_PLUGIN_EXPORT
native_tray_icon_t native_tray_icon_create_from_native(void* native_tray);

/**
 * Destroy a tray icon and release its resources
 * @param tray_icon The tray icon to destroy
 */
FFI_PLUGIN_EXPORT
void native_tray_icon_destroy(native_tray_icon_t tray_icon);

/**
 * Get the ID of a tray icon
 * @param tray_icon The tray icon
 * @return The tray icon ID
 */
FFI_PLUGIN_EXPORT
native_tray_icon_id_t native_tray_icon_get_id(native_tray_icon_t tray_icon);

/**
 * Set the icon image for the tray icon
 * @param tray_icon The tray icon
 * @param icon Path to icon file or base64 encoded image data
 */
FFI_PLUGIN_EXPORT
void native_tray_icon_set_icon(native_tray_icon_t tray_icon, const char* icon);

/**
 * Set the title text for the tray icon
 * @param tray_icon The tray icon
 * @param title The title text to set
 */
FFI_PLUGIN_EXPORT
void native_tray_icon_set_title(native_tray_icon_t tray_icon, const char* title);

/**
 * Get the title text of the tray icon
 * @param tray_icon The tray icon
 * @param buffer Buffer to store the title (caller allocated)
 * @param buffer_size Size of the buffer
 * @return Length of the title, or -1 if buffer too small
 */
FFI_PLUGIN_EXPORT
int native_tray_icon_get_title(native_tray_icon_t tray_icon, char* buffer, size_t buffer_size);

/**
 * Set the tooltip text for the tray icon
 * @param tray_icon The tray icon
 * @param tooltip The tooltip text to set
 */
FFI_PLUGIN_EXPORT
void native_tray_icon_set_tooltip(native_tray_icon_t tray_icon, const char* tooltip);

/**
 * Get the tooltip text of the tray icon
 * @param tray_icon The tray icon
 * @param buffer Buffer to store the tooltip (caller allocated)
 * @param buffer_size Size of the buffer
 * @return Length of the tooltip, or -1 if buffer too small
 */
FFI_PLUGIN_EXPORT
int native_tray_icon_get_tooltip(native_tray_icon_t tray_icon, char* buffer, size_t buffer_size);

/**
 * Set the context menu for the tray icon
 * @param tray_icon The tray icon
 * @param menu The context menu to set
 */
FFI_PLUGIN_EXPORT
void native_tray_icon_set_context_menu(native_tray_icon_t tray_icon, native_menu_t menu);

/**
 * Get the context menu of the tray icon
 * @param tray_icon The tray icon
 * @return The context menu handle, or NULL if no menu set
 */
FFI_PLUGIN_EXPORT
native_menu_t native_tray_icon_get_context_menu(native_tray_icon_t tray_icon);

/**
 * Get the screen bounds of the tray icon
 * @param tray_icon The tray icon
 * @param bounds Pointer to store the bounds (caller allocated)
 * @return true if bounds were retrieved successfully, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_tray_icon_get_bounds(native_tray_icon_t tray_icon, native_rectangle_t* bounds);

/**
 * Show the tray icon in the system tray
 * @param tray_icon The tray icon
 * @return true if shown successfully, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_tray_icon_show(native_tray_icon_t tray_icon);

/**
 * Hide the tray icon from the system tray
 * @param tray_icon The tray icon
 * @return true if hidden successfully, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_tray_icon_hide(native_tray_icon_t tray_icon);

/**
 * Check if the tray icon is currently visible
 * @param tray_icon The tray icon
 * @return true if visible, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_tray_icon_is_visible(native_tray_icon_t tray_icon);

/**
 * Add an event listener for tray icon events
 * @param tray_icon The tray icon
 * @param event_type The type of event to listen for
 * @param callback The callback function
 * @param user_data User data to pass to callback
 * @return Listener ID that can be used to remove the listener, or -1 on error
 */
FFI_PLUGIN_EXPORT
int native_tray_icon_add_listener(native_tray_icon_t tray_icon, native_tray_icon_event_type_t event_type, native_tray_icon_event_callback_t callback, void* user_data);

/**
 * Remove an event listener
 * @param tray_icon The tray icon
 * @param listener_id The listener ID returned by add_listener
 * @return true if the listener was found and removed, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_tray_icon_remove_listener(native_tray_icon_t tray_icon, int listener_id);

/**
 * Show the context menu at specified coordinates
 * @param tray_icon The tray icon
 * @param x The x-coordinate in screen coordinates
 * @param y The y-coordinate in screen coordinates
 * @return true if menu was shown successfully, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_tray_icon_show_context_menu(native_tray_icon_t tray_icon, double x, double y);

/**
 * Show the context menu at default location
 * @param tray_icon The tray icon
 * @return true if menu was shown successfully, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_tray_icon_show_context_menu_default(native_tray_icon_t tray_icon);

#ifdef __cplusplus
}
#endif