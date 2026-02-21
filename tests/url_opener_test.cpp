#include <cstdlib>
#include <iostream>

#include "../src/url_opener.h"
#include "../src/url_opener_internal.h"

namespace {

int RunTests() {
  using namespace nativeapi;

  {
    UrlOpenResult result = OpenUrlWithLauncher("", [](const std::string&) {
      return UrlLaunchOutcome{true, ""};
    });
    if (result.success || result.error_code != UrlOpenErrorCode::kInvalidUrlEmpty) {
      std::cerr << "Expected empty URL to fail with kInvalidUrlEmpty." << std::endl;
      return 1;
    }
  }

  {
    UrlOpenResult result = OpenUrlWithLauncher("example.com", [](const std::string&) {
      return UrlLaunchOutcome{true, ""};
    });
    if (result.success || result.error_code != UrlOpenErrorCode::kInvalidUrlMissingScheme) {
      std::cerr << "Expected missing scheme to fail with kInvalidUrlMissingScheme." << std::endl;
      return 1;
    }
  }

  {
    UrlOpenResult result = OpenUrlWithLauncher("mailto:test@example.com", [](const std::string&) {
      return UrlLaunchOutcome{true, ""};
    });
    if (result.success || result.error_code != UrlOpenErrorCode::kInvalidUrlUnsupportedScheme) {
      std::cerr << "Expected non-http(s) scheme to fail with kInvalidUrlUnsupportedScheme."
                << std::endl;
      return 1;
    }
  }

  {
    bool launched = false;
    UrlOpenResult result = OpenUrlWithLauncher("https://example.com", [&](const std::string& url) {
      launched = (url == "https://example.com");
      return UrlLaunchOutcome{true, ""};
    });
    if (!launched || !result.success || result.error_code != UrlOpenErrorCode::kNone) {
      std::cerr << "Expected valid URL success path." << std::endl;
      return 1;
    }
  }

  {
    UrlOpenResult result = OpenUrlWithLauncher("https://example.com", [](const std::string&) {
      return UrlLaunchOutcome{false, "launcher failed"};
    });
    if (result.success || result.error_code != UrlOpenErrorCode::kInvocationFailed ||
        result.error_message != "launcher failed") {
      std::cerr << "Expected invocation failure with propagated message." << std::endl;
      return 1;
    }
  }

  return 0;
}

}  // namespace

int main() {
  return RunTests();
}
