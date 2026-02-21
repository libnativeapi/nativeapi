#include "url_opener_c.h"

#include <exception>
#include <string>

#include "../url_opener.h"
#include "string_utils_c.h"

using namespace nativeapi;

namespace {

native_url_open_error_code_t ToCErrorCode(UrlOpenErrorCode code) {
  switch (code) {
    case UrlOpenErrorCode::kNone:
      return NATIVE_URL_OPEN_ERROR_NONE;
    case UrlOpenErrorCode::kInvalidUrlEmpty:
      return NATIVE_URL_OPEN_ERROR_INVALID_URL_EMPTY;
    case UrlOpenErrorCode::kInvalidUrlMissingScheme:
      return NATIVE_URL_OPEN_ERROR_INVALID_URL_MISSING_SCHEME;
    case UrlOpenErrorCode::kInvalidUrlUnsupportedScheme:
      return NATIVE_URL_OPEN_ERROR_INVALID_URL_UNSUPPORTED_SCHEME;
    case UrlOpenErrorCode::kUnsupportedPlatform:
      return NATIVE_URL_OPEN_ERROR_UNSUPPORTED_PLATFORM;
    case UrlOpenErrorCode::kInvocationFailed:
      return NATIVE_URL_OPEN_ERROR_INVOCATION_FAILED;
    default:
      return NATIVE_URL_OPEN_ERROR_INVOCATION_FAILED;
  }
}

native_url_open_result_t MakeResult(bool success,
                                    native_url_open_error_code_t error_code,
                                    const std::string& message) {
  native_url_open_result_t result = {};
  result.success = success;
  result.error_code = error_code;
  result.error_message = message.empty() ? nullptr : to_c_str(message);
  return result;
}

}  // namespace

bool native_url_opener_is_supported(void) {
  try {
    return UrlOpener::IsSupported();
  } catch (...) {
    return false;
  }
}

native_url_open_result_t native_url_opener_open(const char* url) {
  if (!url) {
    return MakeResult(false, NATIVE_URL_OPEN_ERROR_INVALID_URL_EMPTY, "URL is empty.");
  }

  try {
    UrlOpener opener;
    const UrlOpenResult result = opener.Open(std::string(url));
    return MakeResult(result.success, ToCErrorCode(result.error_code), result.error_message);
  } catch (const std::exception& e) {
    return MakeResult(false, NATIVE_URL_OPEN_ERROR_INVOCATION_FAILED, e.what());
  } catch (...) {
    return MakeResult(false, NATIVE_URL_OPEN_ERROR_INVOCATION_FAILED,
                      "Unexpected error while opening URL.");
  }
}

void native_url_open_result_free(native_url_open_result_t* result) {
  if (!result) {
    return;
  }

  if (result->error_message) {
    free_c_str(result->error_message);
    result->error_message = nullptr;
  }
  result->success = false;
  result->error_code = NATIVE_URL_OPEN_ERROR_NONE;
}
