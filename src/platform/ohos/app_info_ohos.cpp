#include "../../app_info.h"

namespace nativeapi {
namespace {

class OhosAppInfoImpl final : public AppInfo::Impl {
 public:
  // Bundle metadata lives in the ArkTS layer; it is not reachable from
  // this native layer.
  std::string GetName() const override { return ""; }
  std::string GetIdentifier() const override { return ""; }
  std::string GetVersion() const override { return ""; }
  std::string GetBuildNumber() const override { return ""; }
};

}  // namespace

AppInfo::AppInfo() : pimpl_(std::make_unique<OhosAppInfoImpl>()) {}

AppInfo::~AppInfo() = default;

}  // namespace nativeapi
