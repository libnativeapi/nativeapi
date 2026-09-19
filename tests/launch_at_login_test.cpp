// Windows launch-at-login, checked against the registry it writes: the Run value, the
// Explorer StartupApproved flag that Task Manager toggles, and a non-ASCII identifier
// (which the ANSI registry API used to mangle).
//
// Everything it writes lives under HKCU and a test-only identifier, and is removed again
// -- including on a failing path.

#include <windows.h>

#include <iostream>
#include <string>

#include "../src/launch_at_login.h"

namespace {

constexpr wchar_t kRunKey[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
constexpr wchar_t kStartupApprovedKey[] =
    L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run";

constexpr char kTestId[] = "com.nativeapi.test.launch_at_login";
constexpr wchar_t kTestIdW[] = L"com.nativeapi.test.launch_at_login";
// A name with characters the system code page cannot hold, to catch a narrow API.
// Written as escapes so the source file itself stays ASCII: MSVC reads it in the system
// code page here, and a literal would not survive that.
constexpr char kUnicodeId[] = "\xE5\x90\xAF\xE5\x8A\xA8\xE6\xB5\x8B\xE8\xAF\x95 app";
constexpr wchar_t kUnicodeIdW[] = L"\u542F\u52A8\u6D4B\u8BD5 app";

int failures = 0;

void Check(const char* what, bool ok) {
  std::cout << (ok ? "PASS " : "FAIL ") << what << std::endl;
  if (!ok) {
    ++failures;
  }
}

std::wstring ModuleFileName() {
  wchar_t buffer[MAX_PATH] = {0};
  DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
  return std::wstring(buffer, length);
}

bool ReadRunValue(const wchar_t* name, std::wstring& out) {
  HKEY key = nullptr;
  if (RegOpenKeyExW(HKEY_CURRENT_USER, kRunKey, 0, KEY_READ, &key) != ERROR_SUCCESS) {
    return false;
  }
  wchar_t buffer[2048] = {0};
  DWORD size = sizeof(buffer);
  DWORD type = 0;
  LONG result = RegQueryValueExW(key, name, nullptr, &type, reinterpret_cast<BYTE*>(buffer), &size);
  RegCloseKey(key);
  if (result != ERROR_SUCCESS || type != REG_SZ) {
    return false;
  }
  out.assign(buffer);
  return true;
}

bool ReadApprovedFirstByte(const wchar_t* name, BYTE& out) {
  HKEY key = nullptr;
  if (RegOpenKeyExW(HKEY_CURRENT_USER, kStartupApprovedKey, 0, KEY_READ, &key) != ERROR_SUCCESS) {
    return false;
  }
  BYTE buffer[32] = {0};
  DWORD size = sizeof(buffer);
  LONG result = RegQueryValueExW(key, name, nullptr, nullptr, buffer, &size);
  RegCloseKey(key);
  if (result != ERROR_SUCCESS || size == 0) {
    return false;
  }
  out = buffer[0];
  return true;
}

// What Task Manager does when the user switches a startup entry off.
bool WriteApprovedFirstByte(const wchar_t* name, BYTE first) {
  HKEY key = nullptr;
  DWORD disposition = 0;
  if (RegCreateKeyExW(HKEY_CURRENT_USER, kStartupApprovedKey, 0, nullptr, REG_OPTION_NON_VOLATILE,
                      KEY_SET_VALUE | KEY_WRITE, nullptr, &key, &disposition) != ERROR_SUCCESS) {
    return false;
  }
  BYTE value[12] = {0};
  value[0] = first;
  LONG result = RegSetValueExW(key, name, 0, REG_BINARY, value, sizeof(value));
  RegCloseKey(key);
  return result == ERROR_SUCCESS;
}

bool ValueExists(const wchar_t* subkey, const wchar_t* name) {
  HKEY key = nullptr;
  if (RegOpenKeyExW(HKEY_CURRENT_USER, subkey, 0, KEY_READ, &key) != ERROR_SUCCESS) {
    return false;
  }
  LONG result = RegQueryValueExW(key, name, nullptr, nullptr, nullptr, nullptr);
  RegCloseKey(key);
  return result == ERROR_SUCCESS;
}

void RunTests() {
  using namespace nativeapi;

  Check("IsSupported on Windows", LaunchAtLogin::IsSupported());

  {
    LaunchAtLogin launch_at_login(kTestId, "NativeAPI launch-at-login test");
    launch_at_login.Disable();  // Start from a known state.
    Check("not enabled to begin with", !launch_at_login.IsEnabled());

    Check("Enable succeeds", launch_at_login.Enable());
    Check("IsEnabled after Enable", launch_at_login.IsEnabled());

    std::wstring command;
    Check("Run value written", ReadRunValue(kTestIdW, command));
    Check("Run value names this executable",
          command.find(ModuleFileName()) != std::wstring::npos);

    BYTE approved = 0;
    Check("StartupApproved written", ReadApprovedFirstByte(kTestIdW, approved));
    Check("StartupApproved says approved", (approved % 2) == 0);

    // The user switches it off in Task Manager: the Run value stays, the flag turns odd.
    WriteApprovedFirstByte(kTestIdW, 3);
    Check("disabled in Task Manager reads as disabled", !launch_at_login.IsEnabled());
    Check("Run value still there", ValueExists(kRunKey, kTestIdW));

    Check("Enable again succeeds", launch_at_login.Enable());
    Check("Enable clears the Task Manager flag", launch_at_login.IsEnabled());

    // A path and an argument with spaces have to come back out as one parseable command
    // line, not as four words.
    launch_at_login.SetProgram("C:\\Program Files\\My App\\app.exe", {"--open", "a b"});
    Check("Enable with a spaced path succeeds", launch_at_login.Enable());
    Check("Run value written", ReadRunValue(kTestIdW, command));
    Check("spaces are quoted",
          command == L"\"C:\\Program Files\\My App\\app.exe\" --open \"a b\"");

    Check("Disable succeeds", launch_at_login.Disable());
    Check("IsEnabled after Disable", !launch_at_login.IsEnabled());
    Check("Run value removed", !ValueExists(kRunKey, kTestIdW));
    Check("StartupApproved value removed", !ValueExists(kStartupApprovedKey, kTestIdW));
  }

  {
    // A narrow registry API would write a different, mangled value name here, and the
    // entry could never be found again.
    LaunchAtLogin launch_at_login(kUnicodeId, "Unicode name test");
    launch_at_login.Disable();
    Check("non-ASCII identifier enables", launch_at_login.Enable());
    Check("non-ASCII identifier reads back", launch_at_login.IsEnabled());
    Check("non-ASCII value name is intact", ValueExists(kRunKey, kUnicodeIdW));
    Check("non-ASCII identifier disables", launch_at_login.Disable());
    Check("non-ASCII value removed", !ValueExists(kRunKey, kUnicodeIdW));
  }
}

}  // namespace

int main() {
  RunTests();
  std::cout << failures << " failure(s)" << std::endl;
  return failures == 0 ? 0 : 1;
}
