#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

// Opaque handle type
typedef void* native_secure_storage_t;

// SecureStorage API

/**
 * @brief Create a secure storage with default scope.
 * @return Handle to secure storage, or NULL on failure
 */
native_secure_storage_t native_secure_storage_create(void);

/**
 * @brief Create a secure storage with custom scope.
 * @param scope Scope/application identifier
 * @return Handle to secure storage, or NULL on failure
 */
native_secure_storage_t native_secure_storage_create_with_scope(const char* scope);

/**
 * @brief Destroy a secure storage instance.
 * @param storage Handle to secure storage
 */
void native_secure_storage_destroy(native_secure_storage_t storage);

/**
 * @brief Set a key-value pair in secure storage.
 * @param storage Handle to secure storage
 * @param key The key to set
 * @param value The value to store
 * @return true if successful, false otherwise
 */
bool native_secure_storage_set(native_secure_storage_t storage, const char* key, const char* value);

/**
 * @brief Get the value for a given key from secure storage.
 * @param storage Handle to secure storage
 * @param key The key to retrieve
 * @param default_value Default value if key doesn't exist
 * @return The stored value or default_value. Caller must free the returned string.
 */
char* native_secure_storage_get(native_secure_storage_t storage,
                                const char* key,
                                const char* default_value);

/**
 * @brief Remove a key-value pair from secure storage.
 * @param storage Handle to secure storage
 * @param key The key to remove
 * @return true if successful, false if key doesn't exist
 */
bool native_secure_storage_remove(native_secure_storage_t storage, const char* key);

/**
 * @brief Clear all key-value pairs from secure storage.
 * @param storage Handle to secure storage
 * @return true if successful, false otherwise
 */
bool native_secure_storage_clear(native_secure_storage_t storage);

/**
 * @brief Check if a key exists in secure storage.
 * @param storage Handle to secure storage
 * @param key The key to check
 * @return true if key exists, false otherwise
 */
bool native_secure_storage_contains(native_secure_storage_t storage, const char* key);

/**
 * @brief Get all keys from secure storage.
 * @param storage Handle to secure storage
 * @param out_keys Pointer to array of keys (allocated by function)
 * @param out_count Pointer to receive number of keys
 * @return true if successful, false otherwise. Caller must free each key and the array.
 */
bool native_secure_storage_get_keys(native_secure_storage_t storage,
                                    char*** out_keys,
                                    size_t* out_count);

/**
 * @brief Get the number of stored items in secure storage.
 * @param storage Handle to secure storage
 * @return Number of key-value pairs
 */
size_t native_secure_storage_get_size(native_secure_storage_t storage);

/**
 * @brief Get the scope.
 * @param storage Handle to secure storage
 * @return The scope. Caller must free the returned string.
 */
char* native_secure_storage_get_scope(native_secure_storage_t storage);

/**
 * @brief Check if secure storage is available on this platform.
 * @return true if platform supports secure storage, false otherwise
 */
bool native_secure_storage_is_available(void);

/**
 * @brief Free a string returned by secure storage functions.
 * @param str String to free
 */
void native_secure_storage_free_string(char* str);

/**
 * @brief Free a string array returned by secure storage functions.
 * @param strings Array of strings to free
 * @param count Number of strings in the array
 */
void native_secure_storage_free_string_array(char** strings, size_t count);

#ifdef __cplusplus
}
#endif
