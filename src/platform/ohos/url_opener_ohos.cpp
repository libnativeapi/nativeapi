#include "../../url_opener.h"
#include "../../url_opener_internal.h"

namespace nativeapi {
namespace {

UrlLaunchOutcome LaunchUnsupported(const std::string& url) {
  (void)url;
  return {false, "URL opening is not implemented on OHOS in this native layer."};
}

}  // namespace

bool UrlOpener::IsSupported() {
  return false;
}

UrlOpenResult UrlOpener::Open(const std::string& url) const {
  UrlOpenResult result = OpenUrlWithLauncher(url, LaunchUnsupported);
  if (!result.success && result.error_code == UrlOpenErrorCode::kInvocationFailed) {
    result.error_code = UrlOpenErrorCode::kUnsupportedPlatform;
  }
  return result;
}

}  // namespace nativeapi
