// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "common_c.h"

#if _WIN32
#define FFI_PLUGIN_EXPORT __declspec(dllexport)
#else
#define FFI_PLUGIN_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  NATIVE_URL_OPEN_ERROR_CODE_NONE = 0,
  NATIVE_URL_OPEN_ERROR_CODE_INVALID_URL_EMPTY = 1,
  NATIVE_URL_OPEN_ERROR_CODE_INVALID_URL_MISSING_SCHEME = 2,
  NATIVE_URL_OPEN_ERROR_CODE_INVALID_URL_UNSUPPORTED_SCHEME = 3,
  NATIVE_URL_OPEN_ERROR_CODE_UNSUPPORTED_PLATFORM = 4,
  NATIVE_URL_OPEN_ERROR_CODE_INVOCATION_FAILED = 5,
} native_url_open_error_code_t;

typedef struct {
  bool success;
  native_url_open_error_code_t error_code;
  char* error_message;
} native_url_open_result_t;

/// Frees everything the struct owns.
FFI_PLUGIN_EXPORT
void native_url_open_result_free(native_url_open_result_t* value);

FFI_PLUGIN_EXPORT
bool native_url_opener_is_supported(void);

FFI_PLUGIN_EXPORT
bool native_url_opener_can_open(const char* url);

FFI_PLUGIN_EXPORT
native_url_open_result_t native_url_opener_open(const char* url);

#ifdef __cplusplus
}
#endif
