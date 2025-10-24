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
#include "image_c.h"
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
 * Set the icon image for the tray icon using an Image object
 * @param tray_icon The tray icon
 * @param image The Image object to set as the icon, or NULL to clear the icon
 */
FFI_PLUGIN_EXPORT
void native_tray_icon_set_icon(native_tray_icon_t tray_icon, native_image_t image);

/**
 * Get the current icon image of the tray icon
 * @param tray_icon The tray icon
 * @return The Image object, or NULL if no icon is set. Caller must call
 *         native_image_destroy() when done.
 */
FFI_PLUGIN_EXPORT
native_image_t native_tray_icon_get_icon(native_tray_icon_t tray_icon);

/**
 * Set the title text for the tray icon
 * @param tray_icon The tray icon
 * @param title The title text to set, or NULL to clear the title
 */
FFI_PLUGIN_EXPORT
void native_tray_icon_set_title(native_tray_icon_t tray_icon, const char* title);

/**
 * Get the title text of the tray icon
 * @param tray_icon The tray icon
 * @return The title text, or NULL if no title is set or error. Caller must free
 * the returned string.
 */
FFI_PLUGIN_EXPORT
char* native_tray_icon_get_title(native_tray_icon_t tray_icon);

/**
 * Set the tooltip text for the tray icon
 * @param tray_icon The tray icon
 * @param tooltip The tooltip text to set, or NULL to clear the tooltip
 */
FFI_PLUGIN_EXPORT
void native_tray_icon_set_tooltip(native_tray_icon_t tray_icon, const char* tooltip);

/**
 * Get the tooltip text of the tray icon
 * @param tray_icon The tray icon
 * @return The tooltip text, or NULL if no tooltip is set or error. Caller must
 * free the returned string.
 */
FFI_PLUGIN_EXPORT
char* native_tray_icon_get_tooltip(native_tray_icon_t tray_icon);

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
 * Set the visibility of the tray icon in the system tray
 * @param tray_icon The tray icon
 * @param visible true to show the icon, false to hide it
 * @return true if visibility was changed successfully, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_tray_icon_set_visible(native_tray_icon_t tray_icon, bool visible);

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
int native_tray_icon_add_listener(native_tray_icon_t tray_icon,
                                  native_tray_icon_event_type_t event_type,
                                  native_tray_icon_event_callback_t callback,
                                  void* user_data);

/**
 * Remove an event listener
 * @param tray_icon The tray icon
 * @param listener_id The listener ID returned by add_listener
 * @return true if the listener was found and removed, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_tray_icon_remove_listener(native_tray_icon_t tray_icon, int listener_id);

/**
 * Open the context menu at default location
 * @param tray_icon The tray icon
 * @return true if menu was opened successfully, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_tray_icon_open_context_menu(native_tray_icon_t tray_icon);

/**
 * Close the currently displayed context menu
 * @param tray_icon The tray icon
 * @return true if menu was closed successfully or wasn't visible, false on
 * error
 */
FFI_PLUGIN_EXPORT
bool native_tray_icon_close_context_menu(native_tray_icon_t tray_icon);

#ifdef __cplusplus
}
#endif
