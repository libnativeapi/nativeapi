#import <Foundation/Foundation.h>

#include <sys/sysctl.h>
#include <sys/utsname.h>

#include "../../device_info.h"
#include "string_utils_macos.h"

namespace nativeapi {
namespace {

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

std::string UnameRelease() {
  utsname info{};
  return uname(&info) == 0 ? info.release : "";
}

std::string UnameMachine() {
  utsname info{};
  return uname(&info) == 0 ? info.machine : "";
}

class MacosDeviceInfoImpl final : public DeviceInfo::Impl {
 public:
  std::string GetName() const override {
    @autoreleasepool {
      NSString* name = [[NSHost currentHost] localizedName];
      if (!name) {
        name = [[NSProcessInfo processInfo] hostName];
      }
      return ToStdString(name);
    }
  }

  std::string GetModel() const override { return SysctlString("hw.model"); }

  std::string GetManufacturer() const override { return "Apple"; }

  std::string GetOsName() const override { return "macOS"; }

  std::string GetOsVersion() const override {
    @autoreleasepool {
      NSOperatingSystemVersion version = [[NSProcessInfo processInfo] operatingSystemVersion];
      return std::to_string(version.majorVersion) + "." + std::to_string(version.minorVersion) +
             "." + std::to_string(version.patchVersion);
    }
  }

  std::string GetKernelVersion() const override { return UnameRelease(); }

  std::string GetArchitecture() const override { return UnameMachine(); }
};

}  // namespace

DeviceInfo::DeviceInfo() : pimpl_(std::make_unique<MacosDeviceInfoImpl>()) {}

DeviceInfo::~DeviceInfo() = default;

}  // namespace nativeapi
