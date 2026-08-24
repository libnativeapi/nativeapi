#pragma once

#include <memory>
#include <string>

namespace nativeapi {

/**
 * @brief Read-only metadata about the running application.
 *
 * AppInfo exposes what the platform packaging system records about the
 * current process. Values are constant for the lifetime of the process;
 * a getter whose value the platform cannot provide returns an empty string.
 *
 * Platform sources:
 * - macOS / iOS: the main bundle's Info.plist
 * - Windows: the executable's version resource; the MSIX package identity
 *   when the app is packaged
 * - Linux: the executable path (there is no standard metadata source)
 * - Android / OHOS: not available from this native layer
 *
 * Typical usage:
 * @code
 * auto& info = nativeapi::AppInfo::GetInstance();
 * std::string version = info.GetVersion();  // e.g. "1.2.3"
 * @endcode
 */
class AppInfo {
 public:
  /**
   * @brief Get the singleton instance of AppInfo.
   *
   * @return Reference to the singleton AppInfo instance
   */
  static AppInfo& GetInstance();

  /**
   * @brief Get the user-visible application name.
   *
   * - macOS / iOS: CFBundleDisplayName, falling back to CFBundleName, then
   *   the process name
   * - Windows: version resource ProductName, falling back to
   *   FileDescription, then the executable file name
   * - Linux: the executable file name
   *
   * @return The application name, or "" if unavailable.
   */
  std::string GetName() const;

  /**
   * @brief Get the stable application identifier.
   *
   * The reverse-DNS style identifier the platform knows the app by
   * (what package_info_plus calls the package name):
   * - macOS / iOS: the bundle identifier, e.g. "com.example.myapp"
   * - Windows: the MSIX package family name; "" for unpackaged apps
   * - Linux: "" (no standard identifier source)
   *
   * @return The identifier, or "" if unavailable.
   */
  std::string GetIdentifier() const;

  /**
   * @brief Get the user-visible version string, e.g. "1.2.3".
   *
   * - macOS / iOS: CFBundleShortVersionString
   * - Windows: version resource ProductVersion
   *
   * @return The version, or "" if unavailable.
   */
  std::string GetVersion() const;

  /**
   * @brief Get the build number string, e.g. "42".
   *
   * - macOS / iOS: CFBundleVersion
   * - Windows: the fourth component of the fixed file version
   *
   * @return The build number, or "" if unavailable.
   */
  std::string GetBuildNumber() const;

  // Prevent copying and moving
  AppInfo(const AppInfo&) = delete;
  AppInfo& operator=(const AppInfo&) = delete;
  AppInfo(AppInfo&&) = delete;
  AppInfo& operator=(AppInfo&&) = delete;

  class Impl {
   public:
    virtual ~Impl() = default;

    virtual std::string GetName() const = 0;
    virtual std::string GetIdentifier() const = 0;
    virtual std::string GetVersion() const = 0;
    virtual std::string GetBuildNumber() const = 0;
  };

 private:
  AppInfo();
  ~AppInfo();

  std::unique_ptr<Impl> pimpl_;
};

}  // namespace nativeapi
