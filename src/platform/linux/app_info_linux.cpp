#include "../../app_info.h"

#include <glib.h>
#include <limits.h>
#include <unistd.h>

namespace nativeapi {
namespace {

std::string ExecutableBaseName() {
  char buffer[PATH_MAX];
  ssize_t length = ::readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
  if (length <= 0) {
    return "";
  }
  buffer[length] = '\0';
  std::string path(buffer);
  size_t separator = path.find_last_of('/');
  return separator == std::string::npos ? path : path.substr(separator + 1);
}

class LinuxAppInfoImpl final : public AppInfo::Impl {
 public:
  std::string GetName() const override {
    // g_get_application_name() returns the name set via
    // g_set_application_name(), falling back to the program name that
    // gtk_init()/g_set_prgname() recorded, or NULL if neither happened.
    const gchar* name = g_get_application_name();
    if (name && *name) {
      return name;
    }
    return ExecutableBaseName();
  }

  std::string GetIdentifier() const override {
    // There is no standard identifier source for a plain Linux process.
    return "";
  }

  std::string GetVersion() const override { return ""; }

  std::string GetBuildNumber() const override { return ""; }
};

}  // namespace

AppInfo::AppInfo() : pimpl_(std::make_unique<LinuxAppInfoImpl>()) {}

AppInfo::~AppInfo() = default;

}  // namespace nativeapi
