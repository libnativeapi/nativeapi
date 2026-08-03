// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

#include "url_opener_c.h"

#include <cstdio>
#include <memory>
#include <new>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "string_utils_c.h"
#include "../foundation/handle_table.h"
#include "../url_opener.h"

namespace {

native_url_open_error_code_t ToCUrlOpenErrorCode(nativeapi::UrlOpenErrorCode value) {
  switch (value) {
    case nativeapi::UrlOpenErrorCode::kNone:
      return NATIVE_URL_OPEN_ERROR_CODE_NONE;
    case nativeapi::UrlOpenErrorCode::kInvalidUrlEmpty:
      return NATIVE_URL_OPEN_ERROR_CODE_INVALID_URL_EMPTY;
    case nativeapi::UrlOpenErrorCode::kInvalidUrlMissingScheme:
      return NATIVE_URL_OPEN_ERROR_CODE_INVALID_URL_MISSING_SCHEME;
    case nativeapi::UrlOpenErrorCode::kInvalidUrlUnsupportedScheme:
      return NATIVE_URL_OPEN_ERROR_CODE_INVALID_URL_UNSUPPORTED_SCHEME;
    case nativeapi::UrlOpenErrorCode::kUnsupportedPlatform:
      return NATIVE_URL_OPEN_ERROR_CODE_UNSUPPORTED_PLATFORM;
    case nativeapi::UrlOpenErrorCode::kInvocationFailed:
      return NATIVE_URL_OPEN_ERROR_CODE_INVOCATION_FAILED;
    default:
      return NATIVE_URL_OPEN_ERROR_CODE_NONE;
  }
}

nativeapi::UrlOpenErrorCode ToCppUrlOpenErrorCode(native_url_open_error_code_t value) {
  switch (value) {
    case NATIVE_URL_OPEN_ERROR_CODE_NONE:
      return nativeapi::UrlOpenErrorCode::kNone;
    case NATIVE_URL_OPEN_ERROR_CODE_INVALID_URL_EMPTY:
      return nativeapi::UrlOpenErrorCode::kInvalidUrlEmpty;
    case NATIVE_URL_OPEN_ERROR_CODE_INVALID_URL_MISSING_SCHEME:
      return nativeapi::UrlOpenErrorCode::kInvalidUrlMissingScheme;
    case NATIVE_URL_OPEN_ERROR_CODE_INVALID_URL_UNSUPPORTED_SCHEME:
      return nativeapi::UrlOpenErrorCode::kInvalidUrlUnsupportedScheme;
    case NATIVE_URL_OPEN_ERROR_CODE_UNSUPPORTED_PLATFORM:
      return nativeapi::UrlOpenErrorCode::kUnsupportedPlatform;
    case NATIVE_URL_OPEN_ERROR_CODE_INVOCATION_FAILED:
      return nativeapi::UrlOpenErrorCode::kInvocationFailed;
    default:
      return nativeapi::UrlOpenErrorCode::kNone;
  }
}

native_url_open_result_t ToCUrlOpenResult(const nativeapi::UrlOpenResult& value) {
  native_url_open_result_t result = {};
  result.success = value.success;
  result.error_code = ToCUrlOpenErrorCode(value.error_code);
  result.error_message = to_c_str(value.error_message);
  return result;
}

nativeapi::UrlOpenResult ToCppUrlOpenResult(const native_url_open_result_t& value) {
  nativeapi::UrlOpenResult result = {};
  result.success = value.success;
  result.error_code = ToCppUrlOpenErrorCode(value.error_code);
  result.error_message = value.error_message ? value.error_message : "";
  return result;
}

}  // namespace

void native_url_open_result_free(native_url_open_result_t* value) {
  if (!value) {
    return;
  }
  free_c_str(value->error_message);
  value->error_message = nullptr;
}

bool native_url_opener_is_supported(void) {
  try {
    return nativeapi::UrlOpener::GetInstance().IsSupported();
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_url_opener_is_supported");
    return false;
  }
}

bool native_url_opener_can_open(const char* url) {
  try {
    return nativeapi::UrlOpener::GetInstance().CanOpen(std::string(url ? url : ""));
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_url_opener_can_open");
    return false;
  }
}

native_url_open_result_t native_url_opener_open(const char* url) {
  try {
    const auto cpp_result = nativeapi::UrlOpener::GetInstance().Open(std::string(url ? url : ""));
    return ToCUrlOpenResult(cpp_result);
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_url_opener_open");
    native_url_open_result_t result = {};
    result.success = false;
    return result;
  }
}

