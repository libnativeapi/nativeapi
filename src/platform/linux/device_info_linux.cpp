#include "../../device_info.h"

#include <sys/utsname.h>
#include <unistd.h>

#include <fstream>

namespace nativeapi {
namespace {

std::string Trim(const std::string& value) {
  size_t start = value.find_first_not_of(" \t\r\n");
  if (start == std::string::npos) {
    return "";
  }
  size_t end = value.find_last_not_of(" \t\r\n");
  return value.substr(start, end - start + 1);
}

std::string ReadFirstLine(const char* path) {
  std::ifstream file(path);
  if (!file) {
    return "";
  }
  std::string line;
  std::getline(file, line);
  return Trim(line);
}

// Look up `key` in /etc/os-release, e.g. NAME="Ubuntu" or VERSION_ID="22.04".
std::string OsReleaseField(const std::string& key) {
  std::ifstream file("/etc/os-release");
  if (!file) {
    return "";
  }
  std::string line;
  const std::string prefix = key + "=";
  while (std::getline(file, line)) {
    if (line.compare(0, prefix.size(), prefix) != 0) {
      continue;
    }
    std::string value = Trim(line.substr(prefix.size()));
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
      value = value.substr(1, value.size() - 2);
    }
    return value;
  }
  return "";
}

std::string UnameRelease() {
  utsname info{};
  return uname(&info) == 0 ? info.release : "";
}

std::string UnameMachine() {
  utsname info{};
  return uname(&info) == 0 ? info.machine : "";
}

class LinuxDeviceInfoImpl final : public DeviceInfo::Impl {
 public:
  std::string GetName() const override {
    char buffer[256] = {0};
    if (gethostname(buffer, sizeof(buffer) - 1) != 0) {
      return "";
    }
    return buffer;
  }

  std::string GetModel() const override {
    return ReadFirstLine("/sys/devices/virtual/dmi/id/product_name");
  }

  std::string GetManufacturer() const override {
    return ReadFirstLine("/sys/devices/virtual/dmi/id/sys_vendor");
  }

  std::string GetOsName() const override {
    std::string name = OsReleaseField("NAME");
    return name.empty() ? "Linux" : name;
  }

  std::string GetOsVersion() const override { return OsReleaseField("VERSION_ID"); }

  std::string GetKernelVersion() const override { return UnameRelease(); }

  std::string GetArchitecture() const override { return UnameMachine(); }
};

}  // namespace

DeviceInfo::DeviceInfo() : pimpl_(std::make_unique<LinuxDeviceInfoImpl>()) {}

DeviceInfo::~DeviceInfo() = default;

}  // namespace nativeapi
