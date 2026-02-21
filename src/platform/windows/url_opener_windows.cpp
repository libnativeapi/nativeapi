#include <windows.h>
#include <shellapi.h>

#include <sstream>

#include "../../url_opener.h"
#include "../../url_opener_internal.h"

namespace nativeapi {
namespace {

UrlLaunchOutcome LaunchUrl(const std::string& url) {
  HINSTANCE launch_result =
      ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
  const auto code = reinterpret_cast<intptr_t>(launch_result);
  if (code <= 32) {
    std::ostringstream oss;
    oss << "ShellExecute failed with code " << code << ".";
    return {false, oss.str()};
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
