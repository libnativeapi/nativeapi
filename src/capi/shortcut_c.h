#pragma once

#include <stdbool.h>
#include <stdint.h>

#if _WIN32
#define FFI_PLUGIN_EXPORT __declspec(dllexport)
#else
#define FFI_PLUGIN_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Shortcut ID type
 */
typedef uint64_t native_shortcut_id_t;

/**
 * Opaque handle to a Shortcut instance
 */
typedef void* native_shortcut_t;

/**
 * Shortcut scope enumeration
 */
typedef enum {
  NATIVE_SHORTCUT_SCOPE_GLOBAL = 0,
  NATIVE_SHORTCUT_SCOPE_APPLICATION = 1
} native_shortcut_scope_t;

/**
 * Shortcut options structure
 */
typedef struct {
  const char* accelerator;
  const char* description;
  native_shortcut_scope_t scope;
  bool enabled;
} native_shortcut_options_t;

/**
 * Shortcut callback function type
 */
typedef void (*native_shortcut_callback_t)(native_shortcut_id_t shortcut_id, void* user_data);

/**
 * Get the unique ID of a shortcut
 * @param shortcut The shortcut handle
 * @return The shortcut ID
 */
FFI_PLUGIN_EXPORT
native_shortcut_id_t native_shortcut_get_id(native_shortcut_t shortcut);

/**
 * Get the accelerator string of a shortcut
 * @param shortcut The shortcut handle
 * @return The accelerator string (caller must NOT free)
 */
FFI_PLUGIN_EXPORT
const char* native_shortcut_get_accelerator(native_shortcut_t shortcut);

/**
 * Get the description of a shortcut
 * @param shortcut The shortcut handle
 * @return The description string (caller must NOT free)
 */
FFI_PLUGIN_EXPORT
const char* native_shortcut_get_description(native_shortcut_t shortcut);

/**
 * Set the description of a shortcut
 * @param shortcut The shortcut handle
 * @param description The new description
 */
FFI_PLUGIN_EXPORT
void native_shortcut_set_description(native_shortcut_t shortcut, const char* description);

/**
 * Get the scope of a shortcut
 * @param shortcut The shortcut handle
 * @return The shortcut scope
 */
FFI_PLUGIN_EXPORT
native_shortcut_scope_t native_shortcut_get_scope(native_shortcut_t shortcut);

/**
 * Enable or disable a shortcut
 * @param shortcut The shortcut handle
 * @param enabled true to enable, false to disable
 */
FFI_PLUGIN_EXPORT
void native_shortcut_set_enabled(native_shortcut_t shortcut, bool enabled);

/**
 * Check if a shortcut is enabled
 * @param shortcut The shortcut handle
 * @return true if enabled, false otherwise
 */
FFI_PLUGIN_EXPORT
bool native_shortcut_is_enabled(native_shortcut_t shortcut);

/**
 * Manually invoke a shortcut's callback
 * @param shortcut The shortcut handle
 */
FFI_PLUGIN_EXPORT
void native_shortcut_invoke(native_shortcut_t shortcut);

#ifdef __cplusplus
}
#endif

