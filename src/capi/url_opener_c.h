#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

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
 * @brief Check whether URL opening is supported on this platform.
 */
bool native_url_opener_is_supported(void);

/**
 * @brief Attempt to open URL with the system default browser.
 *
 * Caller must release result.error_message via native_url_open_result_free().
 */
native_url_open_result_t native_url_opener_open(const char* url);

/**
 * @brief Free owned memory inside a native_url_open_result_t.
 */
void native_url_open_result_free(native_url_open_result_t* result);

#ifdef __cplusplus
}
#endif
