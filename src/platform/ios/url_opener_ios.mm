#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#include <dispatch/dispatch.h>

#include "../../url_opener.h"
#include "../../url_opener_internal.h"

namespace nativeapi {
namespace {

UrlLaunchOutcome LaunchUrlOnMainThread(const std::string& url) {
  @autoreleasepool {
    UIApplication* app = [UIApplication sharedApplication];
    if (!app) {
      return {false, "UIApplication is unavailable."};
    }

    NSString* ns_url = [NSString stringWithUTF8String:url.c_str()];
    if (!ns_url) {
      return {false, "Failed to build NSURL from UTF-8 input."};
    }

    NSURL* target = [NSURL URLWithString:ns_url];
    if (!target) {
      return {false, "Failed to parse URL."};
    }

    if (![app canOpenURL:target]) {
      return {false, "No handler available for URL."};
    }

    if (@available(iOS 10.0, *)) {
      __block BOOL open_result = NO;
      dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
      [app openURL:target
          options:@{}
completionHandler:^(BOOL success) {
  open_result = success;
  dispatch_semaphore_signal(semaphore);
}];

      const long wait_result =
          dispatch_semaphore_wait(semaphore, dispatch_time(DISPATCH_TIME_NOW, 3 * NSEC_PER_SEC));
      if (wait_result != 0) {
        return {false, "Timed out waiting for openURL completion."};
      }
      if (!open_result) {
        return {false, "UIApplication failed to open URL."};
      }
      return {true, ""};
    }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    const BOOL opened = [app openURL:target];
#pragma clang diagnostic pop
    if (!opened) {
      return {false, "UIApplication failed to open URL."};
    }
    return {true, ""};
  }
}

UrlLaunchOutcome LaunchUrl(const std::string& url) {
  if ([NSThread isMainThread]) {
    return LaunchUrlOnMainThread(url);
  }

  __block UrlLaunchOutcome outcome;
  dispatch_sync(dispatch_get_main_queue(), ^{
    outcome = LaunchUrlOnMainThread(url);
  });
  return outcome;
}

}  // namespace

bool UrlOpener::IsSupported() {
  return true;
}

UrlOpenResult UrlOpener::Open(const std::string& url) const {
  return OpenUrlWithLauncher(url, LaunchUrl);
}

}  // namespace nativeapi
