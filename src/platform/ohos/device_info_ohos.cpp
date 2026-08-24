#include "../../device_info.h"

#include <sys/utsname.h>

namespace nativeapi {
namespace {

std::string UnameRelease() {
  utsname info{};
  return uname(&info) == 0 ? info.release : "";
}

std::string UnameMachine() {
  utsname info{};
  return uname(&info) == 0 ? info.machine : "";
}

class OhosDeviceInfoImpl final : public DeviceInfo::Impl {
 public:
  // Name/model/manufacturer/OS version are exposed by the deviceinfo NDK
  // (OH_GetMarketName, OH_GetProductModel, ...), which needs an extra
  // link against libdeviceinfo_ndk.z.so; not wired up yet.
  std::string GetName() const override { return ""; }

  std::string GetModel() const override { return ""; }

  std::string GetManufacturer() const override { return ""; }

  std::string GetOsName() const override { return "OpenHarmony"; }

  std::string GetOsVersion() const override { return ""; }

  std::string GetKernelVersion() const override { return UnameRelease(); }

  std::string GetArchitecture() const override { return UnameMachine(); }
};

}  // namespace

DeviceInfo::DeviceInfo() : pimpl_(std::make_unique<OhosDeviceInfoImpl>()) {}

DeviceInfo::~DeviceInfo() = default;

}  // namespace nativeapi
