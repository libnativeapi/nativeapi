#include <shlobj.h>
#include <shobjidl.h>
#include <windows.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#include "../../launch_at_login.h"
#include "string_utils_windows.h"

namespace nativeapi {

namespace {

constexpr wchar_t kRunKey[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
constexpr wchar_t kStartupApprovedKey[] =
    L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run";

// The 12 bytes Explorer keeps under StartupApproved\Run: the first one says whether the
// entry is approved (even) or the user switched it off in Task Manager (odd); the rest is
// a timestamp Explorer writes itself.
constexpr DWORD kStartupApprovedValueSize = 12;
constexpr BYTE kStartupApproved = 2;

// Get the absolute path to the current executable.
std::wstring DetectDefaultProgramPathW() {
  std::vector<wchar_t> buffer(MAX_PATH);
  for (;;) {
    DWORD len = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (len == 0) {
      return std::wstring();
    }
    if (len < buffer.size() - 1) {
      return std::wstring(buffer.data(), len);
    }
    buffer.resize(buffer.size() * 2);
  }
}

std::string DetectDefaultProgramPath() {
  return WStringToString(DetectDefaultProgramPathW());
}

std::string Basename(const std::string& path) {
  if (path.empty())
    return std::string();
  size_t pos = path.find_last_of("\\/");
  if (pos == std::string::npos)
    return path;
  if (pos + 1 >= path.size())
    return path;  // trailing slash
  return path.substr(pos + 1);
}

std::string StripExtension(const std::string& name) {
  size_t pos = name.find_last_of('.');
  if (pos == std::string::npos)
    return name;
  return name.substr(0, pos);
}

// Default identifier for Windows; consistent with other platforms' pattern.
std::string DetectDefaultId() {
  std::string name = StripExtension(Basename(DetectDefaultProgramPath()));
  if (name.empty())
    name = "app";
  return "com.nativeapi.launch_at_login." + name;
}

std::string DetectDefaultDisplayName() {
  std::string name = StripExtension(Basename(DetectDefaultProgramPath()));
  if (name.empty())
    name = "Application";
  return name;
}

// Determine if a Windows command argument needs quoting.
bool NeedsQuoting(const std::wstring& s) {
  if (s.empty())
    return true;
  for (wchar_t c : s) {
    if (c == L' ' || c == L'\t' || c == L'\n' || c == L'\v' || c == L'"') {
      return true;
    }
  }
  return false;
}

// Quote a single argument for Windows command-line per CRT parsing rules.
// Reference: https://learn.microsoft.com/en-us/cpp/cpp/parsing-c-command-line-arguments
std::wstring QuoteArgWindows(const std::wstring& arg) {
  if (!NeedsQuoting(arg)) {
    return arg;
  }

  std::wstring result;
  result.push_back(L'"');

  size_t i = 0;
  while (i < arg.size()) {
    // Count number of backslashes before next character
    size_t backslash_count = 0;
    while (i < arg.size() && arg[i] == L'\\') {
      ++backslash_count;
      ++i;
    }

    if (i == arg.size()) {
      // Escape all backslashes at the end
      result.append(backslash_count * 2, L'\\');
      break;
    }

    if (arg[i] == L'"') {
      // Escape all backslashes (double them), then escape the quote
      result.append(backslash_count * 2 + 1, L'\\');
      result.push_back(L'"');
    } else {
      // Just copy the backslashes
      result.append(backslash_count, L'\\');
      result.push_back(arg[i]);
    }
    ++i;
  }

  result.push_back(L'"');
  return result;
}

// Build full command line: "C:\Path To\app.exe" "arg1" "arg 2"
std::wstring BuildCommandLine(const std::wstring& program, const std::vector<std::string>& args) {
  std::wostringstream oss;
  oss << QuoteArgWindows(program);
  for (const auto& a : args) {
    oss << L' ' << QuoteArgWindows(StringToWString(a));
  }
  return oss.str();
}

// Join arguments the way IShellLink wants them: one string, without the program.
std::wstring BuildArgumentString(const std::vector<std::string>& args) {
  std::wostringstream oss;
  bool first = true;
  for (const auto& a : args) {
    if (!first) {
      oss << L' ';
    }
    first = false;
    oss << QuoteArgWindows(StringToWString(a));
  }
  return oss.str();
}

// Whether this process runs from an MSIX/AppX package. A packaged app cannot use the Run
// key: the registry it sees is virtualized, so Explorer never reads what is written there.
bool IsPackagedApp() {
  using GetCurrentPackageFullNameFn = LONG(WINAPI*)(UINT32*, PWSTR);
  HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
  if (!kernel32) {
    return false;
  }
  auto get_current_package_full_name = reinterpret_cast<GetCurrentPackageFullNameFn>(
      GetProcAddress(kernel32, "GetCurrentPackageFullName"));
  if (!get_current_package_full_name) {
    return false;  // Before Windows 8 there are no packaged apps.
  }
  UINT32 length = 0;
  LONG result = get_current_package_full_name(&length, nullptr);
  return result == ERROR_INSUFFICIENT_BUFFER;
}

// Replace the characters a file name cannot hold.
std::wstring SanitizeForFileName(const std::wstring& name) {
  std::wstring out = name;
  for (wchar_t& c : out) {
    if (c == L'\\' || c == L'/' || c == L':' || c == L'*' || c == L'?' || c == L'"' || c == L'<' ||
        c == L'>' || c == L'|' || c == L'\t' || c == L'\n' || c == L'\r') {
      c = L'_';
    }
  }
  return out;
}

// <user profile>\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup\<id>.lnk
std::wstring ShortcutPath(const std::string& id) {
  PWSTR folder = nullptr;
  if (FAILED(SHGetKnownFolderPath(FOLDERID_Startup, KF_FLAG_CREATE, nullptr, &folder))) {
    return std::wstring();
  }
  std::wstring path(folder);
  CoTaskMemFree(folder);
  if (path.empty()) {
    return std::wstring();
  }
  return path + L"\\" + SanitizeForFileName(StringToWString(id)) + L".lnk";
}

bool FileExists(const std::wstring& path) {
  if (path.empty()) {
    return false;
  }
  DWORD attributes = GetFileAttributesW(path.c_str());
  return attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY);
}

// Initializes COM for this call only when the thread has none, and leaves the thread as it
// was found.
class ScopedCom {
 public:
  ScopedCom() {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    initialized_ = SUCCEEDED(hr);
    ok_ = initialized_ || hr == RPC_E_CHANGED_MODE;
  }

  ~ScopedCom() {
    if (initialized_) {
      CoUninitialize();
    }
  }

  bool ok() const { return ok_; }

  ScopedCom(const ScopedCom&) = delete;
  ScopedCom& operator=(const ScopedCom&) = delete;

 private:
  bool initialized_ = false;
  bool ok_ = false;
};

bool WriteShortcut(const std::wstring& path,
                   const std::wstring& target,
                   const std::wstring& arguments,
                   const std::wstring& description) {
  ScopedCom com;
  if (!com.ok()) {
    return false;
  }

  IShellLinkW* link = nullptr;
  if (FAILED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkW,
                              reinterpret_cast<void**>(&link)))) {
    return false;
  }

  bool ok = SUCCEEDED(link->SetPath(target.c_str()));
  if (ok && !arguments.empty()) {
    ok = SUCCEEDED(link->SetArguments(arguments.c_str()));
  }
  if (ok && !description.empty()) {
    // The Description is what Task Manager and the Startup folder show.
    ok = SUCCEEDED(link->SetDescription(description.c_str()));
  }
  if (ok) {
    size_t separator = target.find_last_of(L'\\');
    if (separator != std::wstring::npos) {
      link->SetWorkingDirectory(target.substr(0, separator).c_str());
    }
  }

  if (ok) {
    IPersistFile* file = nullptr;
    ok = SUCCEEDED(link->QueryInterface(IID_IPersistFile, reinterpret_cast<void**>(&file)));
    if (ok) {
      ok = SUCCEEDED(file->Save(path.c_str(), TRUE));
      file->Release();
    }
  }

  link->Release();
  return ok;
}

bool DeleteShortcut(const std::wstring& path) {
  if (path.empty()) {
    return false;
  }
  if (!FileExists(path)) {
    return true;
  }
  return DeleteFileW(path.c_str()) != 0;
}

bool OpenKey(const wchar_t* subkey, REGSAM access, bool create, HKEY& out) {
  if (create) {
    DWORD disposition = 0;
    return RegCreateKeyExW(HKEY_CURRENT_USER, subkey, 0, nullptr, REG_OPTION_NON_VOLATILE, access,
                           nullptr, &out, &disposition) == ERROR_SUCCESS;
  }
  return RegOpenKeyExW(HKEY_CURRENT_USER, subkey, 0, access, &out) == ERROR_SUCCESS;
}

bool DeleteValue(const wchar_t* subkey, const std::wstring& name) {
  HKEY key = nullptr;
  LONG open_result = RegOpenKeyExW(HKEY_CURRENT_USER, subkey, 0, KEY_SET_VALUE, &key);
  if (open_result != ERROR_SUCCESS) {
    // A key that is not there holds no value to delete.
    return open_result == ERROR_FILE_NOT_FOUND;
  }
  LONG result = RegDeleteValueW(key, name.c_str());
  RegCloseKey(key);
  return result == ERROR_SUCCESS || result == ERROR_FILE_NOT_FOUND;
}

// Whether Explorer still approves this entry: absent or empty means it was never switched
// off, an even first byte means approved, an odd one means the user disabled it in
// Task Manager.
bool IsStartupApproved(const std::wstring& name) {
  HKEY key = nullptr;
  if (!OpenKey(kStartupApprovedKey, KEY_READ, false, key)) {
    return true;
  }
  BYTE value[kStartupApprovedValueSize] = {0};
  DWORD size = sizeof(value);
  DWORD type = 0;
  LONG result = RegQueryValueExW(key, name.c_str(), nullptr, &type, value, &size);
  RegCloseKey(key);
  if (result != ERROR_SUCCESS || size == 0) {
    return true;
  }
  return (value[0] % 2) == 0;
}

// Clears the "disabled in Task Manager" flag, so that enabling really enables.
bool SetStartupApproved(const std::wstring& name) {
  HKEY key = nullptr;
  if (!OpenKey(kStartupApprovedKey, KEY_SET_VALUE | KEY_WRITE, true, key)) {
    return false;
  }
  BYTE value[kStartupApprovedValueSize] = {0};
  value[0] = kStartupApproved;
  LONG result = RegSetValueExW(key, name.c_str(), 0, REG_BINARY, value, sizeof(value));
  RegCloseKey(key);
  return result == ERROR_SUCCESS;
}

}  // namespace

class LaunchAtLogin::Impl {
 public:
  static bool IsSupported() { return true; }

  Impl()
      : id_(DetectDefaultId()),
        display_name_(DetectDefaultDisplayName()),
        program_path_(DetectDefaultProgramPath()) {}

  explicit Impl(const std::string& id)
      : id_(id),
        display_name_(DetectDefaultDisplayName()),
        program_path_(DetectDefaultProgramPath()) {}

  Impl(const std::string& id, const std::string& display_name)
      : id_(id), display_name_(display_name), program_path_(DetectDefaultProgramPath()) {}

  ~Impl() = default;

  std::string GetId() const { return id_; }

  std::string GetDisplayName() const { return display_name_; }

  bool SetDisplayName(const std::string& display_name) {
    display_name_ = display_name;
    return true;
  }

  bool SetProgram(const std::string& executable_path, const std::vector<std::string>& arguments) {
    program_path_ = executable_path;
    arguments_ = arguments;
    return true;
  }

  std::string GetExecutablePath() const { return program_path_; }

  std::vector<std::string> GetArguments() const { return arguments_; }

  bool Enable() {
    if (program_path_.empty()) {
      program_path_ = DetectDefaultProgramPath();
      if (program_path_.empty()) {
        return false;
      }
    }
    const std::wstring program = StringToWString(program_path_);

    if (IsPackagedApp()) {
      return WriteShortcut(ShortcutPath(id_), program, BuildArgumentString(arguments_),
                           StringToWString(display_name_));
    }

    HKEY key = nullptr;
    if (!OpenKey(kRunKey, KEY_SET_VALUE | KEY_WRITE, true, key)) {
      return false;
    }
    const std::wstring command = BuildCommandLine(program, arguments_);
    const std::wstring name = StringToWString(id_);
    LONG result = RegSetValueExW(key, name.c_str(), 0, REG_SZ,
                                 reinterpret_cast<const BYTE*>(command.c_str()),
                                 static_cast<DWORD>((command.size() + 1) * sizeof(wchar_t)));
    RegCloseKey(key);
    if (result != ERROR_SUCCESS) {
      return false;
    }
    // An entry the user switched off in Task Manager stays off until this is cleared.
    SetStartupApproved(name);
    return true;
  }

  bool Disable() {
    if (IsPackagedApp()) {
      return DeleteShortcut(ShortcutPath(id_));
    }
    const std::wstring name = StringToWString(id_);
    bool removed = DeleteValue(kRunKey, name);
    DeleteValue(kStartupApprovedKey, name);
    return removed;
  }

  bool IsEnabled() const {
    if (IsPackagedApp()) {
      return FileExists(ShortcutPath(id_));
    }

    HKEY key = nullptr;
    if (!OpenKey(kRunKey, KEY_READ, false, key)) {
      return false;
    }
    const std::wstring name = StringToWString(id_);
    LONG result = RegQueryValueExW(key, name.c_str(), nullptr, nullptr, nullptr, nullptr);
    RegCloseKey(key);
    if (result != ERROR_SUCCESS) {
      return false;
    }
    return IsStartupApproved(name);
  }

 private:
  std::string id_;
  std::string display_name_;
  std::string program_path_;
  std::vector<std::string> arguments_;
};

// LaunchAtLogin public API implementations

LaunchAtLogin::LaunchAtLogin() : pimpl_(std::make_unique<Impl>()) {}

LaunchAtLogin::LaunchAtLogin(const std::string& id) : pimpl_(std::make_unique<Impl>(id)) {}

LaunchAtLogin::LaunchAtLogin(const std::string& id, const std::string& display_name)
    : pimpl_(std::make_unique<Impl>(id, display_name)) {}

LaunchAtLogin::~LaunchAtLogin() = default;

bool LaunchAtLogin::IsSupported() {
  return Impl::IsSupported();
}

std::string LaunchAtLogin::GetId() const {
  return pimpl_->GetId();
}

std::string LaunchAtLogin::GetDisplayName() const {
  return pimpl_->GetDisplayName();
}

bool LaunchAtLogin::SetDisplayName(const std::string& display_name) {
  return pimpl_->SetDisplayName(display_name);
}

bool LaunchAtLogin::SetProgram(const std::string& executable_path,
                               const std::vector<std::string>& arguments) {
  return pimpl_->SetProgram(executable_path, arguments);
}

std::string LaunchAtLogin::GetExecutablePath() const {
  return pimpl_->GetExecutablePath();
}

std::vector<std::string> LaunchAtLogin::GetArguments() const {
  return pimpl_->GetArguments();
}

bool LaunchAtLogin::Enable() {
  return pimpl_->Enable();
}

bool LaunchAtLogin::Disable() {
  return pimpl_->Disable();
}

bool LaunchAtLogin::IsEnabled() const {
  return pimpl_->IsEnabled();
}

}  // namespace nativeapi
