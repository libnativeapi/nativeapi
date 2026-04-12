#pragma once

#include <stdbool.h>

#if _WIN32
#define FFI_PLUGIN_EXPORT __declspec(dllexport)
#else
#define FFI_PLUGIN_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque handle for a URL opener instance.
 */
typedef void* native_url_opener_t;

/**
 * @brief Error codes returned by URL opening APIs.
 */
typedef enum {
  NATIVE_URL_OPEN_ERROR_NONE = 0,
  NATIVE_URL_OPEN_ERROR_INVALID_URL_EMPTY = 1,
  NATIVE_URL_OPEN_ERROR_INVALID_URL_MISSING_SCHEME = 2,
  NATIVE_URL_OPEN_ERROR_INVALID_URL_UNSUPPORTED_SCHEME = 3,
  NATIVE_URL_OPEN_ERROR_UNSUPPORTED_PLATFORM = 4,
  NATIVE_URL_OPEN_ERROR_INVOCATION_FAILED = 5,
} native_url_open_error_code_t;

/**
 * @brief Result payload for URL open attempts.
 */
typedef struct {
  bool success;
  native_url_open_error_code_t error_code;
  char* error_message;
} native_url_open_result_t;

/**
 * @brief Create a URL opener instance.
 */
FFI_PLUGIN_EXPORT
native_url_opener_t native_url_opener_create(void);

/**
 * @brief Destroy a URL opener instance.
 */
FFI_PLUGIN_EXPORT
void native_url_opener_destroy(native_url_opener_t opener);

/**
 * @brief Check whether URL opening is supported on this platform.
 */
FFI_PLUGIN_EXPORT
bool native_url_opener_is_supported(native_url_opener_t opener);

/**
 * @brief Attempt to open URL with the system default browser.
 *
 * Caller must release result.error_message via native_url_open_result_free().
 */
FFI_PLUGIN_EXPORT
native_url_open_result_t native_url_opener_open(native_url_opener_t opener, const char* url);

/**
 * @brief Free owned memory inside a native_url_open_result_t.
 */
FFI_PLUGIN_EXPORT
void native_url_open_result_free(native_url_open_result_t* result);

#ifdef __cplusplus
}
#endif
