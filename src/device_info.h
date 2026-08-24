#pragma once

#include <memory>
#include <string>

namespace nativeapi {

/**
 * @brief Read-only description of the machine the process runs on.
 *
 * DeviceInfo answers "what is this machine": its user-visible name, hardware
 * model, and operating system. It deliberately excludes dynamic resource
 * metrics (CPU load, memory) and any unique machine identifier. Values are
 * read from the platform on every call; a getter whose value the platform
 * cannot provide returns an empty string.
 *
 * Typical usage:
 * @code
 * auto& info = nativeapi::DeviceInfo::GetInstance();
 * std::string model = info.GetModel();  // e.g. "MacBookPro18,1"
 * @endcode
 */
class DeviceInfo {
 public:
  /**
   * @brief Get the singleton instance of DeviceInfo.
   *
   * @return Reference to the singleton DeviceInfo instance
   */
  static DeviceInfo& GetInstance();

  /**
   * @brief Get the user-visible device name.
   *
   * - macOS: the computer name, e.g. "Jerry's MacBook Pro"
   * - Windows: the computer's DNS host name
   * - Linux: the host name
   * - iOS: the device name from UIDevice
   *
   * @return The device name, or "" if unavailable.
   */
  std::string GetName() const;

  /**
   * @brief Get the hardware model identifier.
   *
   * - macOS / iOS: e.g. "MacBookPro18,1" / "iPhone15,2"
   * - Windows / Linux: the SMBIOS product name, e.g. "XPS 13 9310"
   * - Android: e.g. "Pixel 7 Pro"
   *
   * @return The model, or "" if unavailable.
   */
  std::string GetModel() const;

  /**
   * @brief Get the hardware manufacturer, e.g. "Apple", "Dell Inc.".
   *
   * @return The manufacturer, or "" if unavailable.
   */
  std::string GetManufacturer() const;

  /**
   * @brief Get the operating system name.
   *
   * E.g. "macOS", "Windows", "Ubuntu", "iOS", "Android", "OpenHarmony".
   * On Linux this is the distribution name from /etc/os-release.
   *
   * @return The OS name, or "" if unavailable.
   */
  std::string GetOsName() const;

  /**
   * @brief Get the operating system version, e.g. "14.5" or "10.0.22631".
   *
   * @return The OS version, or "" if unavailable.
   */
  std::string GetOsVersion() const;

  /**
   * @brief Get the kernel version.
   *
   * - macOS / iOS / Linux / Android: the uname release, e.g. "23.5.0"
   * - Windows: same as GetOsVersion()
   *
   * @return The kernel version, or "" if unavailable.
   */
  std::string GetKernelVersion() const;

  /**
   * @brief Get the processor architecture of the running process.
   *
   * E.g. "arm64", "x86_64".
   *
   * @return The architecture, or "" if unavailable.
   */
  std::string GetArchitecture() const;

  // Prevent copying and moving
  DeviceInfo(const DeviceInfo&) = delete;
  DeviceInfo& operator=(const DeviceInfo&) = delete;
  DeviceInfo(DeviceInfo&&) = delete;
  DeviceInfo& operator=(DeviceInfo&&) = delete;

  class Impl {
   public:
    virtual ~Impl() = default;

    virtual std::string GetName() const = 0;
    virtual std::string GetModel() const = 0;
    virtual std::string GetManufacturer() const = 0;
    virtual std::string GetOsName() const = 0;
    virtual std::string GetOsVersion() const = 0;
    virtual std::string GetKernelVersion() const = 0;
    virtual std::string GetArchitecture() const = 0;
  };

 private:
  DeviceInfo();
  ~DeviceInfo();

  std::unique_ptr<Impl> pimpl_;
};

}  // namespace nativeapi
