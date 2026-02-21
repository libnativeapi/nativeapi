#pragma once

#include <string>

namespace nativeapi {

enum class UrlOpenErrorCode {
  kNone = 0,
  kInvalidUrlEmpty,
  kInvalidUrlMissingScheme,
  kInvalidUrlUnsupportedScheme,
  kUnsupportedPlatform,
  kInvocationFailed,
};

struct UrlOpenResult {
  bool success = false;
  UrlOpenErrorCode error_code = UrlOpenErrorCode::kNone;
  std::string error_message;
};

class UrlOpener {
 public:
  static bool IsSupported();

  UrlOpenResult Open(const std::string& url) const;
};

}  // namespace nativeapi
