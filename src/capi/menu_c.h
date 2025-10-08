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

/**
 * Opaque handles for menu objects
 */
typedef void* native_menu_t;
typedef void* native_menu_item_t;

/**
 * Menu and menu item identifiers
 */
typedef long native_menu_id_t;
typedef long native_menu_item_id_t;

/**
 * Menu item types
 */
typedef enum {
  NATIVE_MENU_ITEM_TYPE_NORMAL = 0,
  NATIVE_MENU_ITEM_TYPE_CHECKBOX = 1,
  NATIVE_MENU_ITEM_TYPE_RADIO = 2,
  NATIVE_MENU_ITEM_TYPE_SEPARATOR = 3,
  NATIVE_MENU_ITEM_TYPE_SUBMENU = 4
} native_menu_item_type_t;

/**
 * Menu item states
 */
typedef enum {
  NATIVE_MENU_ITEM_STATE_UNCHECKED = 0,
  NATIVE_MENU_ITEM_STATE_CHECKED = 1,
  NATIVE_MENU_ITEM_STATE_MIXED = 2
} native_menu_item_state_t;

/**
 * Keyboard accelerator modifier flags
 */
typedef enum {
  NATIVE_ACCELERATOR_MODIFIER_NONE = 0,
  NATIVE_ACCELERATOR_MODIFIER_CTRL = 1 << 0,
  NATIVE_ACCELERATOR_MODIFIER_ALT = 1 << 1,
  NATIVE_ACCELERATOR_MODIFIER_SHIFT = 1 << 2,
  NATIVE_ACCELERATOR_MODIFIER_META = 1 << 3
} native_accelerator_modifier_t;

/**
 * Keyboard accelerator structure
 */
typedef struct {
  int modifiers;
  char key[64];
} native_keyboard_accelerator_t;

/**
 * Menu item clicked event
 */
typedef struct {
  native_menu_item_id_t item_id;
  char item_text[256];
} native_menu_item_clicked_event_t;

/**
 * Menu item submenu opened event
 */
typedef struct {
  native_menu_item_id_t item_id;
} native_menu_item_submenu_opened_event_t;

/**
 * Menu item submenu closed event
 */
typedef struct {
  native_menu_item_id_t item_id;
} native_menu_item_submenu_closed_event_t;


/**
 * Menu item list structure
 */
typedef struct {
  native_menu_item_t* items;
  size_t count;
} native_menu_item_list_t;

/**
 * Menu opened event
 */
typedef struct {
  native_menu_id_t menu_id;
} native_menu_opened_event_t;

/**
 * Menu closed event
 */
typedef struct {
  native_menu_id_t menu_id;
} native_menu_closed_event_t;


/**
 * Event listener registration function types
 */
typedef void (*native_menu_item_event_callback_t)(const void* event, void* user_data);
typedef void (*native_menu_event_callback_t)(const void* event, void* user_data);

/**
 * Event types for menu item events
 */
typedef enum {
  NATIVE_MENU_ITEM_EVENT_CLICKED = 0,
  NATIVE_MENU_ITEM_EVENT_SUBMENU_OPENED = 1,
  NATIVE_MENU_ITEM_EVENT_SUBMENU_CLOSED = 2
} native_menu_item_event_type_t;

/**
 * Event types for menu events
 */
typedef enum {
  NATIVE_MENU_EVENT_OPENED = 0,
  NATIVE_MENU_EVENT_CLOSED = 1
} native_menu_event_type_t;

/**
 * MenuItem operations
 */

/**
 * Create a new menu item
 * @param text The display text for the menu item
 * @param type The type of menu item to create
 * @return Menu item handle, or NULL if creation failed
 */
FFI_PLUGIN_EXPORT
native_menu_item_t native_menu_item_create(const char* text, native_menu_item_type_t type);

/**
 * Create a separator menu item
 * @return Menu item handle, or NULL if creation failed
 */
FFI_PLUGIN_EXPORT
native_menu_item_t native_menu_item_create_separator(void);

/**
 * Destroy a menu item and release its resources
 * @param item The menu item to destroy
 */
FFI_PLUGIN_EXPORT
void native_menu_item_destroy(native_menu_item_t item);

/**
 * Get the ID of a menu item
 * @param item The menu item
 * @return The menu item ID
 */
FFI_PLUGIN_EXPORT
native_menu_item_id_t native_menu_item_get_id(native_menu_item_t item);

/**
 * Get the type of a menu item
 * @param item The menu item
 * @return The menu item type
 */
FFI_PLUGIN_EXPORT
native_menu_item_type_t native_menu_item_get_type(native_menu_item_t item);

/**
 * Set the label of a menu item
 * @param item The menu item
 * @param label The label to set
 */
FFI_PLUGIN_EXPORT
void native_menu_item_set_label(native_menu_item_t item, const char* label);

/**
 * Get the label of a menu item
 * @param item The menu item
 * @return The label string (caller must free), or NULL if item is invalid
 */
FFI_PLUGIN_EXPORT
char* native_menu_item_get_label(native_menu_item_t item);

/**
 * Set the icon of a menu item
 * @param item The menu item
 * @param icon Path to icon file or base64 data
 */
FFI_PLUGIN_EXPORT
void native_menu_item_set_icon(native_menu_item_t item, const char* icon);

/**
 * Get the icon of a menu item
 * @param item The menu item
 * @return The icon path/data string (caller must free), or NULL if item is invalid or no icon set
 */
FFI_PLUGIN_EXPORT
char* native_menu_item_get_icon(native_menu_item_t item);

/**
 * Set the tooltip of a menu item
 * @param item The menu item
 * @param tooltip The tooltip text to set
 */
FFI_PLUGIN_EXPORT
void native_menu_item_set_tooltip(native_menu_item_t item, const char* tooltip);

/**
 * Get the tooltip of a menu item
 * @param item The menu item
 * @return The tooltip string (caller must free), or NULL if item is invalid or no tooltip set
 */
FFI_PLUGIN_EXPORT
char* native_menu_item_get_tooltip(native_menu_item_t item);

/**
 * Set the keyboard accelerator for a menu item
 * @param item The menu item
 * @param accelerator The keyboard accelerator to set
 */
FFI_PLUGIN_EXPORT
void native_menu_item_set_accelerator(native_menu_item_t item, const native_keyboard_accelerator_t* accelerator);

/**
 * Get the keyboard accelerator of a menu item
 * @param item The menu item
 * @param accelerator Pointer to store the accelerator (caller allocated)
 * @return true if accelerator exists, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_menu_item_get_accelerator(native_menu_item_t item, native_keyboard_accelerator_t* accelerator);

/**
 * Remove the keyboard accelerator from a menu item
 * @param item The menu item
 */
FFI_PLUGIN_EXPORT
void native_menu_item_remove_accelerator(native_menu_item_t item);

/**
 * Set the enabled state of a menu item
 * @param item The menu item
 * @param enabled true to enable, false to disable
 */
FFI_PLUGIN_EXPORT
void native_menu_item_set_enabled(native_menu_item_t item, bool enabled);

/**
 * Check if a menu item is enabled
 * @param item The menu item
 * @return true if enabled, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_menu_item_is_enabled(native_menu_item_t item);

/**
 * Set the visibility of a menu item
 * @param item The menu item
 * @param visible true to show, false to hide
 */
FFI_PLUGIN_EXPORT
void native_menu_item_set_visible(native_menu_item_t item, bool visible);

/**
 * Check if a menu item is visible
 * @param item The menu item
 * @return true if visible, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_menu_item_is_visible(native_menu_item_t item);

/**
 * Set the state of a checkbox/radio menu item
 * @param item The menu item
 * @param state The state to set (unchecked, checked, or mixed)
 */
FFI_PLUGIN_EXPORT
void native_menu_item_set_state(native_menu_item_t item, native_menu_item_state_t state);

/**
 * Get the state of a menu item
 * @param item The menu item
 * @return The current state of the menu item
 */
FFI_PLUGIN_EXPORT
native_menu_item_state_t native_menu_item_get_state(native_menu_item_t item);

/**
 * Set the radio group ID for a radio menu item
 * @param item The menu item
 * @param group_id The radio group identifier
 */
FFI_PLUGIN_EXPORT
void native_menu_item_set_radio_group(native_menu_item_t item, int group_id);

/**
 * Get the radio group ID of a menu item
 * @param item The menu item
 * @return The radio group ID, or -1 if not set
 */
FFI_PLUGIN_EXPORT
int native_menu_item_get_radio_group(native_menu_item_t item);

/**
 * Set the submenu for a menu item
 * @param item The menu item
 * @param submenu The submenu to attach
 */
FFI_PLUGIN_EXPORT
void native_menu_item_set_submenu(native_menu_item_t item, native_menu_t submenu);

/**
 * Get the submenu of a menu item
 * @param item The menu item
 * @return The submenu handle, or NULL if no submenu
 */
FFI_PLUGIN_EXPORT
native_menu_t native_menu_item_get_submenu(native_menu_item_t item);

/**
 * Remove the submenu from a menu item
 * @param item The menu item
 */
FFI_PLUGIN_EXPORT
void native_menu_item_remove_submenu(native_menu_item_t item);

/**
 * Add event listener for a menu item
 * @param item The menu item
 * @param event_type The type of event to listen for
 * @param callback The callback function
 * @param user_data User data passed to callback
 * @return A listener ID that can be used to remove the listener, or -1 on error
 */
FFI_PLUGIN_EXPORT
int native_menu_item_add_listener(native_menu_item_t item, native_menu_item_event_type_t event_type, native_menu_item_event_callback_t callback, void* user_data);

/**
 * Remove event listener from a menu item
 * @param item The menu item
 * @param listener_id The listener ID returned by native_menu_item_add_listener
 * @return true if removed successfully, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_menu_item_remove_listener(native_menu_item_t item, int listener_id);


/**
 * Programmatically trigger a menu item
 * @param item The menu item
 * @return true if triggered successfully, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_menu_item_trigger(native_menu_item_t item);

/**
 * Menu operations
 */

/**
 * Create a new menu
 * @return Menu handle, or NULL if creation failed
 */
FFI_PLUGIN_EXPORT
native_menu_t native_menu_create(void);

/**
 * Destroy a menu and release its resources
 * @param menu The menu to destroy
 */
FFI_PLUGIN_EXPORT
void native_menu_destroy(native_menu_t menu);

/**
 * Get the ID of a menu
 * @param menu The menu
 * @return The menu ID
 */
FFI_PLUGIN_EXPORT
native_menu_id_t native_menu_get_id(native_menu_t menu);

/**
 * Add a menu item to the end of the menu
 * @param menu The menu
 * @param item The menu item to add
 */
FFI_PLUGIN_EXPORT
void native_menu_add_item(native_menu_t menu, native_menu_item_t item);

/**
 * Insert a menu item at a specific position
 * @param menu The menu
 * @param item The menu item to insert
 * @param index The position to insert at (0-based)
 */
FFI_PLUGIN_EXPORT
void native_menu_insert_item(native_menu_t menu, native_menu_item_t item, size_t index);

/**
 * Remove a menu item from the menu
 * @param menu The menu
 * @param item The menu item to remove
 * @return true if item was found and removed, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_menu_remove_item(native_menu_t menu, native_menu_item_t item);

/**
 * Remove a menu item by its ID
 * @param menu The menu
 * @param item_id The ID of the item to remove
 * @return true if item was found and removed, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_menu_remove_item_by_id(native_menu_t menu, native_menu_item_id_t item_id);

/**
 * Remove a menu item at a specific position
 * @param menu The menu
 * @param index The position of the item to remove (0-based)
 * @return true if item was removed, false if index out of bounds
 */
FFI_PLUGIN_EXPORT
bool native_menu_remove_item_at(native_menu_t menu, size_t index);

/**
 * Remove all items from the menu
 * @param menu The menu
 */
FFI_PLUGIN_EXPORT
void native_menu_clear(native_menu_t menu);

/**
 * Add a separator to the end of the menu
 * @param menu The menu
 */
FFI_PLUGIN_EXPORT
void native_menu_add_separator(native_menu_t menu);

/**
 * Insert a separator at a specific position
 * @param menu The menu
 * @param index The position to insert the separator at (0-based)
 */
FFI_PLUGIN_EXPORT
void native_menu_insert_separator(native_menu_t menu, size_t index);

/**
 * Get the number of items in the menu
 * @param menu The menu
 * @return The number of items
 */
FFI_PLUGIN_EXPORT
size_t native_menu_get_item_count(native_menu_t menu);

/**
 * Get a menu item at a specific position
 * @param menu The menu
 * @param index The position of the item (0-based)
 * @return The menu item handle, or NULL if index out of bounds
 */
FFI_PLUGIN_EXPORT
native_menu_item_t native_menu_get_item_at(native_menu_t menu, size_t index);

/**
 * Get a menu item by its ID
 * @param menu The menu
 * @param item_id The ID of the item to find
 * @return The menu item handle, or NULL if not found
 */
FFI_PLUGIN_EXPORT
native_menu_item_t native_menu_get_item_by_id(native_menu_t menu, native_menu_item_id_t item_id);

/**
 * Get all menu items
 * @param menu The menu
 * @return List of menu items (caller must free with native_menu_item_list_free)
 */
FFI_PLUGIN_EXPORT
native_menu_item_list_t native_menu_get_all_items(native_menu_t menu);

/**
 * Find a menu item by text
 * @param menu The menu
 * @param text The text to search for
 * @return The first menu item with matching text, or NULL if not found
 */
FFI_PLUGIN_EXPORT
native_menu_item_t native_menu_find_item_by_text(native_menu_t menu, const char* text);

/**
 * Show the menu as a context menu at specified coordinates
 * @param menu The menu
 * @param x The x-coordinate in screen coordinates
 * @param y The y-coordinate in screen coordinates
 * @return true if menu was shown successfully, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_menu_show_as_context_menu(native_menu_t menu, double x, double y);

/**
 * Show the menu as a context menu at default location
 * @param menu The menu
 * @return true if menu was shown successfully, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_menu_show_as_context_menu_default(native_menu_t menu);

/**
 * Close the menu if it's currently showing
 * @param menu The menu
 * @return true if menu was closed, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_menu_close(native_menu_t menu);

/**
 * Check if the menu is currently visible
 * @param menu The menu
 * @return true if visible, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_menu_is_visible(native_menu_t menu);

/**
 * Set the enabled state of the menu
 * @param menu The menu
 * @param enabled true to enable, false to disable
 */
FFI_PLUGIN_EXPORT
void native_menu_set_enabled(native_menu_t menu, bool enabled);

/**
 * Check if the menu is enabled
 * @param menu The menu
 * @return true if enabled, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_menu_is_enabled(native_menu_t menu);

/**
 * Add event listener for a menu
 * @param menu The menu
 * @param event_type The type of event to listen for
 * @param callback The callback function
 * @param user_data User data passed to callback
 * @return A listener ID that can be used to remove the listener, or -1 on error
 */
FFI_PLUGIN_EXPORT
int native_menu_add_listener(native_menu_t menu, native_menu_event_type_t event_type, native_menu_event_callback_t callback, void* user_data);

/**
 * Remove event listener from a menu
 * @param menu The menu
 * @param listener_id The listener ID returned by native_menu_add_listener
 * @return true if removed successfully, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_menu_remove_listener(native_menu_t menu, int listener_id);


/**
 * Create and add a menu item in one operation
 * @param menu The menu
 * @param text The item text
 * @param type The item type
 * @return The created menu item handle
 */
FFI_PLUGIN_EXPORT
native_menu_item_t native_menu_create_and_add_item(native_menu_t menu, const char* text, native_menu_item_type_t type);

/**
 * Create and add a submenu item in one operation
 * @param menu The menu
 * @param text The item text
 * @param submenu The submenu to attach
 * @return The created menu item handle
 */
FFI_PLUGIN_EXPORT
native_menu_item_t native_menu_create_and_add_submenu(native_menu_t menu, const char* text, native_menu_t submenu);

/**
 * Utility functions
 */

/**
 * Free a menu item list
 * @param list The list to free
 */
FFI_PLUGIN_EXPORT
void native_menu_item_list_free(native_menu_item_list_t list);

/**
 * Convert keyboard accelerator to string representation
 * @param accelerator The accelerator
 * @return The string representation (caller must free), or NULL if accelerator is invalid
 */
FFI_PLUGIN_EXPORT
char* native_keyboard_accelerator_to_string(const native_keyboard_accelerator_t* accelerator);

#ifdef __cplusplus
}
#endif