#include "url_opener_internal.h"

#include <algorithm>
#include <cctype>

namespace nativeapi {
namespace {

std::string Trim(const std::string& value) {
  size_t start = 0;
  while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start]))) {
    ++start;
  }

  size_t end = value.size();
  while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
    --end;
  }

  return value.substr(start, end - start);
}

std::string ToLower(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  return value;
}

UrlOpenResult MakeFailure(UrlOpenErrorCode code, const std::string& message) {
  UrlOpenResult result;
  result.success = false;
  result.error_code = code;
  result.error_message = message;
  return result;
}

UrlOpenResult ValidateUrl(const std::string& raw_url) {
  const std::string url = Trim(raw_url);
  if (url.empty()) {
    return MakeFailure(UrlOpenErrorCode::kInvalidUrlEmpty, "URL is empty.");
  }

  const size_t scheme_separator = url.find(':');
  if (scheme_separator == std::string::npos || scheme_separator == 0) {
    return MakeFailure(UrlOpenErrorCode::kInvalidUrlMissingScheme,
                       "URL must include an explicit scheme (http or https).");
  }

  const std::string scheme = ToLower(url.substr(0, scheme_separator));
  if (scheme != "http" && scheme != "https") {
    return MakeFailure(UrlOpenErrorCode::kInvalidUrlUnsupportedScheme,
                       "Only http and https URLs are supported.");
  }

  UrlOpenResult ok;
  ok.success = true;
  ok.error_code = UrlOpenErrorCode::kNone;
  return ok;
}

}  // namespace

UrlOpenResult OpenUrlWithLauncher(const std::string& url, const UrlLauncher& launcher) {
  UrlOpenResult validation = ValidateUrl(url);
  if (!validation.success) {
    return validation;
  }

  const UrlLaunchOutcome launch = launcher(url);
  if (!launch.success) {
    const std::string message =
        launch.message.empty() ? "Failed to invoke the system browser." : launch.message;
    return MakeFailure(UrlOpenErrorCode::kInvocationFailed, message);
  }

  UrlOpenResult ok;
  ok.success = true;
  ok.error_code = UrlOpenErrorCode::kNone;
  return ok;
}

}  // namespace nativeapi
