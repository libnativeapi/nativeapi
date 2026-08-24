#include "../../app_info.h"

#include <cstdio>

namespace nativeapi {
namespace {

std::string ReadPackageNameFromCmdline() {
  // Android names the app process after its package
  // ("com.example.app" or "com.example.app:service").
  FILE* file = std::fopen("/proc/self/cmdline", "r");
  if (!file) {
    return "";
  }
  char buffer[256] = {0};
  size_t length = std::fread(buffer, 1, sizeof(buffer) - 1, file);
  std::fclose(file);
  if (length == 0) {
    return "";
  }
  std::string name(buffer);  // cmdline is NUL-separated; take argv[0]
  size_t colon = name.find(':');
  if (colon != std::string::npos) {
    name.resize(colon);
  }
  return name;
}

class AndroidAppInfoImpl final : public AppInfo::Impl {
 public:
  std::string GetName() const override {
    // The user-visible label lives in PackageManager, which this native
    // layer cannot reach without a JNI context.
    return "";
  }

  std::string GetIdentifier() const override { return ReadPackageNameFromCmdline(); }

  std::string GetVersion() const override { return ""; }

  std::string GetBuildNumber() const override { return ""; }
};

}  // namespace

AppInfo::AppInfo() : pimpl_(std::make_unique<AndroidAppInfoImpl>()) {}

AppInfo::~AppInfo() = default;

}  // namespace nativeapi
