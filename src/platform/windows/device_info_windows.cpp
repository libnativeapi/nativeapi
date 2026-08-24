#include "../../device_info.h"

#include <windows.h>

namespace nativeapi {
namespace {

std::string WideToUtf8(const std::wstring& value) {
  if (value.empty()) {
    return "";
  }
  int size = WideCharToMultiByte(CP_UTF8, 0, value.c_str(), static_cast<int>(value.size()),
                                 nullptr, 0, nullptr, nullptr);
  if (size <= 0) {
    return "";
  }
  std::string result(static_cast<size_t>(size), '\0');
  WideCharToMultiByte(CP_UTF8, 0, value.c_str(), static_cast<int>(value.size()), result.data(),
                      size, nullptr, nullptr);
  return result;
}

std::string ReadRegistryString(const wchar_t* subkey, const wchar_t* value) {
  DWORD size = 0;
  if (RegGetValueW(HKEY_LOCAL_MACHINE, subkey, value, RRF_RT_REG_SZ, nullptr, nullptr, &size) !=
          ERROR_SUCCESS ||
      size == 0) {
    return "";
  }
  std::wstring buffer(size / sizeof(wchar_t), L'\0');
  if (RegGetValueW(HKEY_LOCAL_MACHINE, subkey, value, RRF_RT_REG_SZ, nullptr, buffer.data(),
                   &size) != ERROR_SUCCESS) {
    return "";
  }
  while (!buffer.empty() && buffer.back() == L'\0') {
    buffer.pop_back();
  }
  return WideToUtf8(buffer);
}

constexpr const wchar_t* kBiosKey = L"HARDWARE\\DESCRIPTION\\System\\BIOS";

// GetVersionExW lies to manifest-less processes; RtlGetVersion reports the
// real OS version.
std::string RealOsVersion() {
  using RtlGetVersionFn = LONG(WINAPI*)(PRTL_OSVERSIONINFOW);
  HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
  if (!ntdll) {
    return "";
  }
  auto rtl_get_version =
      reinterpret_cast<RtlGetVersionFn>(GetProcAddress(ntdll, "RtlGetVersion"));
  if (!rtl_get_version) {
    return "";
  }
  RTL_OSVERSIONINFOW info{};
  info.dwOSVersionInfoSize = sizeof(info);
  if (rtl_get_version(&info) != 0) {
    return "";
  }
  return std::to_string(info.dwMajorVersion) + "." + std::to_string(info.dwMinorVersion) + "." +
         std::to_string(info.dwBuildNumber);
}

class WindowsDeviceInfoImpl final : public DeviceInfo::Impl {
 public:
  std::string GetName() const override {
    DWORD size = 0;
    GetComputerNameExW(ComputerNamePhysicalDnsHostname, nullptr, &size);
    if (size == 0) {
      return "";
    }
    std::wstring buffer(size, L'\0');
    if (!GetComputerNameExW(ComputerNamePhysicalDnsHostname, buffer.data(), &size)) {
      return "";
    }
    buffer.resize(size);
    return WideToUtf8(buffer);
  }

  std::string GetModel() const override {
    return ReadRegistryString(kBiosKey, L"SystemProductName");
  }

  std::string GetManufacturer() const override {
    return ReadRegistryString(kBiosKey, L"SystemManufacturer");
  }

  std::string GetOsName() const override { return "Windows"; }

  std::string GetOsVersion() const override { return RealOsVersion(); }

  std::string GetKernelVersion() const override { return RealOsVersion(); }

  std::string GetArchitecture() const override {
    SYSTEM_INFO info{};
    GetNativeSystemInfo(&info);
    switch (info.wProcessorArchitecture) {
      case PROCESSOR_ARCHITECTURE_AMD64:
        return "x86_64";
      case PROCESSOR_ARCHITECTURE_ARM64:
        return "arm64";
      case PROCESSOR_ARCHITECTURE_ARM:
        return "arm";
      case PROCESSOR_ARCHITECTURE_INTEL:
        return "x86";
      default:
        return "";
    }
  }
};

}  // namespace

DeviceInfo::DeviceInfo() : pimpl_(std::make_unique<WindowsDeviceInfoImpl>()) {}

DeviceInfo::~DeviceInfo() = default;

}  // namespace nativeapi
