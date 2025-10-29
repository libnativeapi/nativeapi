#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

// Opaque handle type
typedef void* native_preferences_t;

// Preferences API

/**
 * @brief Create a preferences storage with default scope.
 * @return Handle to preferences storage, or NULL on failure
 */
native_preferences_t native_preferences_create(void);

/**
 * @brief Create a preferences storage with custom scope.
 * @param scope Scope for isolating preferences
 * @return Handle to preferences storage, or NULL on failure
 */
native_preferences_t native_preferences_create_with_scope(const char* scope);

/**
 * @brief Destroy a preferences storage instance.
 * @param prefs Handle to preferences storage
 */
void native_preferences_destroy(native_preferences_t prefs);

/**
 * @brief Set a key-value pair.
 * @param prefs Handle to preferences storage
 * @param key The key to set
 * @param value The value to store
 * @return true if successful, false otherwise
 */
bool native_preferences_set(native_preferences_t prefs, const char* key, const char* value);

/**
 * @brief Get the value for a given key.
 * @param prefs Handle to preferences storage
 * @param key The key to retrieve
 * @param default_value Default value if key doesn't exist
 * @return The stored value or default_value. Caller must free the returned string.
 */
char* native_preferences_get(native_preferences_t prefs,
                             const char* key,
                             const char* default_value);

/**
 * @brief Remove a key-value pair.
 * @param prefs Handle to preferences storage
 * @param key The key to remove
 * @return true if successful, false if key doesn't exist
 */
bool native_preferences_remove(native_preferences_t prefs, const char* key);

/**
 * @brief Clear all key-value pairs.
 * @param prefs Handle to preferences storage
 * @return true if successful, false otherwise
 */
bool native_preferences_clear(native_preferences_t prefs);

/**
 * @brief Check if a key exists.
 * @param prefs Handle to preferences storage
 * @param key The key to check
 * @return true if key exists, false otherwise
 */
bool native_preferences_contains(native_preferences_t prefs, const char* key);

/**
 * @brief Get all keys.
 * @param prefs Handle to preferences storage
 * @param out_keys Pointer to array of keys (allocated by function)
 * @param out_count Pointer to receive number of keys
 * @return true if successful, false otherwise. Caller must free each key and the array.
 */
bool native_preferences_get_keys(native_preferences_t prefs, char*** out_keys, size_t* out_count);

/**
 * @brief Get the number of stored items.
 * @param prefs Handle to preferences storage
 * @return Number of key-value pairs
 */
size_t native_preferences_get_size(native_preferences_t prefs);

/**
 * @brief Get the scope.
 * @param prefs Handle to preferences storage
 * @return The scope. Caller must free the returned string.
 */
char* native_preferences_get_scope(native_preferences_t prefs);

/**
 * @brief Free a string returned by preferences functions.
 * @param str String to free
 */
void native_preferences_free_string(char* str);

/**
 * @brief Free a string array returned by preferences functions.
 * @param strings Array of strings to free
 * @param count Number of strings in the array
 */
void native_preferences_free_string_array(char** strings, size_t count);

#ifdef __cplusplus
}
#endif
