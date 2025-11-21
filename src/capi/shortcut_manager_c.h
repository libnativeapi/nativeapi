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

#include "shortcut_c.h"

/**
 * Shortcut event types
 */
typedef enum {
  NATIVE_SHORTCUT_EVENT_ACTIVATED = 0,
  NATIVE_SHORTCUT_EVENT_REGISTERED = 1,
  NATIVE_SHORTCUT_EVENT_UNREGISTERED = 2,
  NATIVE_SHORTCUT_EVENT_REGISTRATION_FAILED = 3
} native_shortcut_event_type_t;

/**
 * Shortcut event structure
 */
typedef struct {
  native_shortcut_event_type_t type;
  native_shortcut_id_t shortcut_id;
  const char* accelerator;
  union {
    struct {
      const char* error_message;
    } registration_failed;
  } data;
} native_shortcut_event_t;

/**
 * Shortcut event callback function type
 */
typedef void (*native_shortcut_event_callback_t)(const native_shortcut_event_t* event,
                                                 void* user_data);

/**
 * Shortcut list structure
 */
typedef struct {
  native_shortcut_t* shortcuts;
  size_t count;
} native_shortcut_list_t;

/**
 * Free a shortcut list
 * @param list The list to free
 */
FFI_PLUGIN_EXPORT
void native_shortcut_list_free(native_shortcut_list_t list);

/**
 * Check if global shortcuts are supported on the current platform
 * @return true if supported, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_shortcut_manager_is_supported(void);

/**
 * Register a new keyboard shortcut with a simple accelerator string
 * @param accelerator The keyboard shortcut string (e.g., "Ctrl+Shift+A")
 * @param callback The callback function to invoke when the shortcut is activated
 * @param user_data User data to pass to the callback
 * @return Shortcut handle, or NULL if registration failed
 */
FFI_PLUGIN_EXPORT
native_shortcut_t native_shortcut_manager_register(const char* accelerator,
                                                   native_shortcut_callback_t callback,
                                                   void* user_data);

/**
 * Register a new keyboard shortcut with detailed options
 * @param options The shortcut options
 * @param callback The callback function to invoke when the shortcut is activated
 * @param user_data User data to pass to the callback
 * @return Shortcut handle, or NULL if registration failed
 */
FFI_PLUGIN_EXPORT
native_shortcut_t native_shortcut_manager_register_with_options(
    const native_shortcut_options_t* options,
    native_shortcut_callback_t callback,
    void* user_data);

/**
 * Unregister a keyboard shortcut by its ID
 * @param shortcut_id The shortcut ID
 * @return true if unregistered successfully, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_shortcut_manager_unregister_by_id(native_shortcut_id_t shortcut_id);

/**
 * Unregister a keyboard shortcut by its accelerator string
 * @param accelerator The keyboard shortcut string
 * @return true if unregistered successfully, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_shortcut_manager_unregister_by_accelerator(const char* accelerator);

/**
 * Unregister all keyboard shortcuts
 * @return Number of shortcuts that were unregistered
 */
FFI_PLUGIN_EXPORT
int native_shortcut_manager_unregister_all(void);

/**
 * Get a shortcut by its ID
 * @param shortcut_id The shortcut ID
 * @return Shortcut handle, or NULL if not found
 */
FFI_PLUGIN_EXPORT
native_shortcut_t native_shortcut_manager_get_by_id(native_shortcut_id_t shortcut_id);

/**
 * Get a shortcut by its accelerator string
 * @param accelerator The keyboard shortcut string
 * @return Shortcut handle, or NULL if not found
 */
FFI_PLUGIN_EXPORT
native_shortcut_t native_shortcut_manager_get_by_accelerator(const char* accelerator);

/**
 * Get all registered shortcuts
 * @return List of all shortcuts (caller must free with native_shortcut_list_free)
 */
FFI_PLUGIN_EXPORT
native_shortcut_list_t native_shortcut_manager_get_all(void);

/**
 * Get shortcuts filtered by scope
 * @param scope The shortcut scope to filter by
 * @return List of matching shortcuts (caller must free with native_shortcut_list_free)
 */
FFI_PLUGIN_EXPORT
native_shortcut_list_t native_shortcut_manager_get_by_scope(native_shortcut_scope_t scope);

/**
 * Check if a specific accelerator is available for registration
 * @param accelerator The keyboard shortcut string to check
 * @return true if available, false if already in use
 */
FFI_PLUGIN_EXPORT
bool native_shortcut_manager_is_available(const char* accelerator);

/**
 * Validate an accelerator string format
 * @param accelerator The keyboard shortcut string to validate
 * @return true if the format is valid, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_shortcut_manager_is_valid_accelerator(const char* accelerator);

/**
 * Enable or disable shortcut processing
 * @param enabled true to enable, false to disable
 */
FFI_PLUGIN_EXPORT
void native_shortcut_manager_set_enabled(bool enabled);

/**
 * Check if shortcut processing is enabled
 * @return true if enabled, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_shortcut_manager_is_enabled(void);

/**
 * Register a callback for shortcut events
 * @param callback The callback function to register
 * @param user_data User data to pass to the callback
 * @return Registration ID, or -1 on failure
 */
FFI_PLUGIN_EXPORT
int native_shortcut_manager_register_event_callback(native_shortcut_event_callback_t callback,
                                                    void* user_data);

/**
 * Unregister a shortcut event callback
 * @param registration_id The registration ID returned by register_event_callback
 * @return true if callback was found and unregistered, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_shortcut_manager_unregister_event_callback(int registration_id);

#ifdef __cplusplus
}
#endif
