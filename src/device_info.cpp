#include "device_info.h"

namespace nativeapi {

DeviceInfo& DeviceInfo::GetInstance() {
  static DeviceInfo instance;
  return instance;
}

std::string DeviceInfo::GetName() const {
  return pimpl_->GetName();
}

std::string DeviceInfo::GetModel() const {
  return pimpl_->GetModel();
}

std::string DeviceInfo::GetManufacturer() const {
  return pimpl_->GetManufacturer();
}

std::string DeviceInfo::GetOsName() const {
  return pimpl_->GetOsName();
}

std::string DeviceInfo::GetOsVersion() const {
  return pimpl_->GetOsVersion();
}

std::string DeviceInfo::GetKernelVersion() const {
  return pimpl_->GetKernelVersion();
}

std::string DeviceInfo::GetArchitecture() const {
  return pimpl_->GetArchitecture();
}

}  // namespace nativeapi
