#pragma once

#include <functional>
#include <string>

#include "url_opener.h"

namespace nativeapi {

struct UrlLaunchOutcome {
  bool success = false;
  std::string message;
};

using UrlLauncher = std::function<UrlLaunchOutcome(const std::string&)>;

UrlOpenResult OpenUrlWithLauncher(const std::string& url, const UrlLauncher& launcher);

}  // namespace nativeapi
