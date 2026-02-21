#include <gio/gio.h>

#include "../../url_opener.h"
#include "../../url_opener_internal.h"

namespace nativeapi {
namespace {

UrlLaunchOutcome LaunchUrl(const std::string& url) {
  GError* error = nullptr;
  const gboolean ok = g_app_info_launch_default_for_uri(url.c_str(), nullptr, &error);
  if (!ok) {
    std::string message = "Failed to launch URL via desktop defaults.";
    if (error && error->message) {
      message = error->message;
    }
    if (error) {
      g_error_free(error);
    }
    return {false, message};
  }
  return {true, ""};
}

}  // namespace

bool UrlOpener::IsSupported() {
  return true;
}

UrlOpenResult UrlOpener::Open(const std::string& url) const {
  return OpenUrlWithLauncher(url, LaunchUrl);
}

}  // namespace nativeapi
