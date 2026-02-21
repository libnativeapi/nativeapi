#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

#include "../../url_opener.h"
#include "../../url_opener_internal.h"

namespace nativeapi {
namespace {

UrlLaunchOutcome LaunchUrl(const std::string& url) {
  @autoreleasepool {
    NSString* ns_url = [NSString stringWithUTF8String:url.c_str()];
    if (!ns_url) {
      return {false, "Failed to build NSURL from UTF-8 input."};
    }

    NSURL* target = [NSURL URLWithString:ns_url];
    if (!target) {
      return {false, "Failed to parse URL."};
    }

    const BOOL opened = [[NSWorkspace sharedWorkspace] openURL:target];
    if (!opened) {
      return {false, "NSWorkspace could not open the URL."};
    }
    return {true, ""};
  }
}

}  // namespace

bool UrlOpener::IsSupported() {
  return true;
}

UrlOpenResult UrlOpener::Open(const std::string& url) const {
  return OpenUrlWithLauncher(url, LaunchUrl);
}

}  // namespace nativeapi
