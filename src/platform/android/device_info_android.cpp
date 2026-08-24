#include "../../device_info.h"

#include <sys/system_properties.h>
#include <sys/utsname.h>

namespace nativeapi {
namespace {

std::string SystemProperty(const char* name) {
  char value[PROP_VALUE_MAX] = {0};
  int length = __system_property_get(name, value);
  return length > 0 ? std::string(value, static_cast<size_t>(length)) : "";
}

std::string UnameRelease() {
  utsname info{};
  return uname(&info) == 0 ? info.release : "";
}

std::string UnameMachine() {
  utsname info{};
  return uname(&info) == 0 ? info.machine : "";
}

class AndroidDeviceInfoImpl final : public DeviceInfo::Impl {
 public:
  std::string GetName() const override {
    // The user-visible device name lives in Settings.Global, which this
    // native layer cannot reach without a JNI context.
    return "";
  }

  std::string GetModel() const override { return SystemProperty("ro.product.model"); }

  std::string GetManufacturer() const override {
    return SystemProperty("ro.product.manufacturer");
  }

  std::string GetOsName() const override { return "Android"; }

  std::string GetOsVersion() const override {
    return SystemProperty("ro.build.version.release");
  }

  std::string GetKernelVersion() const override { return UnameRelease(); }

  std::string GetArchitecture() const override { return UnameMachine(); }
};

}  // namespace

DeviceInfo::DeviceInfo() : pimpl_(std::make_unique<AndroidDeviceInfoImpl>()) {}

DeviceInfo::~DeviceInfo() = default;

}  // namespace nativeapi
