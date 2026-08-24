#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#include <sys/sysctl.h>
#include <sys/utsname.h>

#include "../../device_info.h"

namespace nativeapi {
namespace {

std::string ToStdString(NSString* value) {
  if (!value) {
    return "";
  }
  const char* utf8 = [value UTF8String];
  return utf8 ? utf8 : "";
}

std::string SysctlString(const char* name) {
  size_t size = 0;
  if (sysctlbyname(name, nullptr, &size, nullptr, 0) != 0 || size == 0) {
    return "";
  }
  std::string value(size, '\0');
  if (sysctlbyname(name, value.data(), &size, nullptr, 0) != 0) {
    return "";
  }
  while (!value.empty() && value.back() == '\0') {
    value.pop_back();
  }
  return value;
}

class IosDeviceInfoImpl final : public DeviceInfo::Impl {
 public:
  std::string GetName() const override {
    @autoreleasepool {
      return ToStdString([[UIDevice currentDevice] name]);
    }
  }

  std::string GetModel() const override {
    // "iPhone15,2"; on iOS hw.machine carries the model identifier.
    std::string model = SysctlString("hw.machine");
    if (!model.empty()) {
      return model;
    }
    @autoreleasepool {
      return ToStdString([[UIDevice currentDevice] model]);
    }
  }

  std::string GetManufacturer() const override { return "Apple"; }

  std::string GetOsName() const override {
    @autoreleasepool {
      return ToStdString([[UIDevice currentDevice] systemName]);
    }
  }

  std::string GetOsVersion() const override {
    @autoreleasepool {
      return ToStdString([[UIDevice currentDevice] systemVersion]);
    }
  }

  std::string GetKernelVersion() const override {
    utsname info{};
    if (uname(&info) != 0) {
      return "";
    }
    return info.release;
  }

  std::string GetArchitecture() const override {
    // uname's machine field is the model identifier on iOS, so report the
    // compile-time process architecture instead.
#if defined(__arm64__) || defined(__aarch64__)
    return "arm64";
#elif defined(__x86_64__)
    return "x86_64";
#else
    return "";
#endif
  }
};

}  // namespace

DeviceInfo::DeviceInfo() : pimpl_(std::make_unique<IosDeviceInfoImpl>()) {}

DeviceInfo::~DeviceInfo() = default;

}  // namespace nativeapi
