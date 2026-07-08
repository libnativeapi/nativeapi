// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

#include "url_opener_c.h"

#include <cstdlib>
#include <cstring>
#include <string>

#include "../url_opener.h"

#include <cstdio>

namespace {

char* DupString(const std::string& value) {
  if (value.empty()) {
    return nullptr;
  }
  const auto size = value.size() + 1;
  auto* buffer = static_cast<char*>(std::malloc(size));
  if (!buffer) {
    return nullptr;
  }
  std::memcpy(buffer, value.c_str(), size);
  return buffer;
}

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

native_url_open_result_t ToCUrlOpenResult(const nativeapi::UrlOpenResult& value) {
  native_url_open_result_t result = {};
  result.success = value.success;
  result.error_code = ToCUrlOpenErrorCode(value.error_code);
  result.error_message = DupString(value.error_message);
  return result;
}

}  // namespace

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

void native_url_open_result_free(native_url_open_result_t* value) {
  if (!value) {
    return;
  }
  std::free(value->error_message);
  value->error_message = nullptr;
}

