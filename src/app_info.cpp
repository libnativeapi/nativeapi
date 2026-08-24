#include "app_info.h"

namespace nativeapi {

AppInfo& AppInfo::GetInstance() {
  static AppInfo instance;
  return instance;
}

std::string AppInfo::GetName() const {
  return pimpl_->GetName();
}

std::string AppInfo::GetIdentifier() const {
  return pimpl_->GetIdentifier();
}

std::string AppInfo::GetVersion() const {
  return pimpl_->GetVersion();
}

std::string AppInfo::GetBuildNumber() const {
  return pimpl_->GetBuildNumber();
}

}  // namespace nativeapi
