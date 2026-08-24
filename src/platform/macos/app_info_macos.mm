#import <Foundation/Foundation.h>

#include "../../app_info.h"

namespace nativeapi {
namespace {

std::string ToStdString(NSString* value) {
  if (!value) {
    return "";
  }
  const char* utf8 = [value UTF8String];
  return utf8 ? utf8 : "";
}

NSString* InfoPlistString(NSString* key) {
  id value = [[NSBundle mainBundle] objectForInfoDictionaryKey:key];
  return [value isKindOfClass:[NSString class]] ? (NSString*)value : nil;
}

class MacosAppInfoImpl final : public AppInfo::Impl {
 public:
  std::string GetName() const override {
    @autoreleasepool {
      NSString* name = InfoPlistString(@"CFBundleDisplayName");
      if (!name) {
        name = InfoPlistString(@"CFBundleName");
      }
      if (!name) {
        name = [[NSProcessInfo processInfo] processName];
      }
      return ToStdString(name);
    }
  }

  std::string GetIdentifier() const override {
    @autoreleasepool {
      return ToStdString([[NSBundle mainBundle] bundleIdentifier]);
    }
  }

  std::string GetVersion() const override {
    @autoreleasepool {
      return ToStdString(InfoPlistString(@"CFBundleShortVersionString"));
    }
  }

  std::string GetBuildNumber() const override {
    @autoreleasepool {
      return ToStdString(InfoPlistString(@"CFBundleVersion"));
    }
  }
};

}  // namespace

AppInfo::AppInfo() : pimpl_(std::make_unique<MacosAppInfoImpl>()) {}

AppInfo::~AppInfo() = default;

}  // namespace nativeapi
