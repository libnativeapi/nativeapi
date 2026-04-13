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
  static UrlOpener& GetInstance();

  bool IsSupported() const;

  UrlOpenResult Open(const std::string& url) const;

  UrlOpener(const UrlOpener&) = delete;
  UrlOpener& operator=(const UrlOpener&) = delete;
  UrlOpener(UrlOpener&&) = delete;
  UrlOpener& operator=(UrlOpener&&) = delete;

 private:
  UrlOpener() = default;
};

}  // namespace nativeapi
