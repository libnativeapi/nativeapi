#include "../../app_info.h"

#include <windows.h>

#include <appmodel.h>

#include <cstdint>
#include <vector>

namespace nativeapi {
namespace {

std::string WideToUtf8(const wchar_t* value, int length) {
  if (!value || length <= 0) {
    return "";
  }
  int size = WideCharToMultiByte(CP_UTF8, 0, value, length, nullptr, 0, nullptr, nullptr);
  if (size <= 0) {
    return "";
  }
  std::string result(static_cast<size_t>(size), '\0');
  WideCharToMultiByte(CP_UTF8, 0, value, length, result.data(), size, nullptr, nullptr);
  return result;
}

std::string WideToUtf8(const std::wstring& value) {
  return WideToUtf8(value.c_str(), static_cast<int>(value.size()));
}

std::wstring GetExecutablePath() {
  std::wstring buffer(MAX_PATH, L'\0');
  for (;;) {
    DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (length == 0) {
      return L"";
    }
    if (length < buffer.size()) {
      buffer.resize(length);
      return buffer;
    }
    buffer.resize(buffer.size() * 2);
  }
}

std::wstring ExecutableBaseName(const std::wstring& path) {
  size_t separator = path.find_last_of(L"\\/");
  std::wstring name = separator == std::wstring::npos ? path : path.substr(separator + 1);
  size_t dot = name.find_last_of(L'.');
  if (dot != std::wstring::npos && dot > 0) {
    name.resize(dot);
  }
  return name;
}

std::string PackageFamilyName() {
  UINT32 length = 0;
  LONG rc = GetCurrentPackageFamilyName(&length, nullptr);
  if (rc != ERROR_INSUFFICIENT_BUFFER || length == 0) {
    return "";  // APPMODEL_ERROR_NO_PACKAGE: not running from an MSIX package
  }
  std::wstring buffer(length, L'\0');
  rc = GetCurrentPackageFamilyName(&length, buffer.data());
  if (rc != ERROR_SUCCESS) {
    return "";
  }
  while (!buffer.empty() && buffer.back() == L'\0') {
    buffer.pop_back();
  }
  return WideToUtf8(buffer);
}

class WindowsAppInfoImpl final : public AppInfo::Impl {
 public:
  WindowsAppInfoImpl() { Load(); }

  std::string GetName() const override { return name_; }
  std::string GetIdentifier() const override { return identifier_; }
  std::string GetVersion() const override { return version_; }
  std::string GetBuildNumber() const override { return build_number_; }

 private:
  void Load() {
    identifier_ = PackageFamilyName();

    std::wstring exe_path = GetExecutablePath();
    if (exe_path.empty()) {
      return;
    }
    name_ = WideToUtf8(ExecutableBaseName(exe_path));

    DWORD ignored = 0;
    DWORD size = GetFileVersionInfoSizeW(exe_path.c_str(), &ignored);
    if (size == 0) {
      return;
    }
    std::vector<uint8_t> data(size);
    if (!GetFileVersionInfoW(exe_path.c_str(), 0, size, data.data())) {
      return;
    }

    std::string product_name = QueryString(data, L"ProductName");
    if (product_name.empty()) {
      product_name = QueryString(data, L"FileDescription");
    }
    if (!product_name.empty()) {
      name_ = product_name;
    }

    version_ = QueryString(data, L"ProductVersion");

    VS_FIXEDFILEINFO* fixed_info = nullptr;
    UINT fixed_size = 0;
    if (VerQueryValueW(data.data(), L"\\", reinterpret_cast<void**>(&fixed_info), &fixed_size) &&
        fixed_info && fixed_size >= sizeof(VS_FIXEDFILEINFO)) {
      if (version_.empty()) {
        version_ = std::to_string(HIWORD(fixed_info->dwProductVersionMS)) + "." +
                   std::to_string(LOWORD(fixed_info->dwProductVersionMS)) + "." +
                   std::to_string(HIWORD(fixed_info->dwProductVersionLS));
      }
      build_number_ = std::to_string(LOWORD(fixed_info->dwFileVersionLS));
    }
  }

  static std::string QueryString(const std::vector<uint8_t>& data, const wchar_t* key) {
    struct LangCodePage {
      WORD language;
      WORD code_page;
    };

    LangCodePage* translations = nullptr;
    UINT translations_size = 0;
    if (!VerQueryValueW(const_cast<uint8_t*>(data.data()), L"\\VarFileInfo\\Translation",
                        reinterpret_cast<void**>(&translations), &translations_size) ||
        translations_size < sizeof(LangCodePage)) {
      return "";
    }

    const size_t count = translations_size / sizeof(LangCodePage);
    for (size_t i = 0; i < count; ++i) {
      wchar_t sub_block[64];
      swprintf(sub_block, sizeof(sub_block) / sizeof(sub_block[0]),
               L"\\StringFileInfo\\%04x%04x\\%s", translations[i].language,
               translations[i].code_page, key);

      wchar_t* value = nullptr;
      UINT value_length = 0;
      if (VerQueryValueW(const_cast<uint8_t*>(data.data()), sub_block,
                         reinterpret_cast<void**>(&value), &value_length) &&
          value && value_length > 0) {
        // value_length includes the terminating null.
        std::string result = WideToUtf8(value, static_cast<int>(wcsnlen(value, value_length)));
        if (!result.empty()) {
          return result;
        }
      }
    }
    return "";
  }

  std::string name_;
  std::string identifier_;
  std::string version_;
  std::string build_number_;
};

}  // namespace

AppInfo::AppInfo() : pimpl_(std::make_unique<WindowsAppInfoImpl>()) {}

AppInfo::~AppInfo() = default;

}  // namespace nativeapi
