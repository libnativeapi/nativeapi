#include <windows.h>
#include <iostream>
#include "nativeapi.h"

using namespace nativeapi;
namespace {
MessageDialog* active = nullptr;
HWND owner = nullptr;
HWND other = nullptr;
HWND content = nullptr;
HWND disabled_content = nullptr;
bool timer_passed = false;
bool application_modal = false;
void CALLBACK Dismiss(HWND, UINT, UINT_PTR id, DWORD) {
  KillTimer(nullptr, id);
  timer_passed = IsWindowEnabled(owner) && !IsWindowEnabled(content) &&
      !IsWindowEnabled(disabled_content) &&
      (application_modal ? !IsWindowEnabled(other) : IsWindowEnabled(other));
  active->SetTitle("Updated title");
  active->SetMessage("Updated message while open");
  const bool reentry = active->Open();
  const bool closed = active->Close();
  if (!timer_passed || reentry || !closed)
    std::cerr << "modality=" << application_modal << " disabled=" << timer_passed
              << " reentry=" << reentry << " close=" << closed << std::endl;
  timer_passed = !reentry && closed && timer_passed;
}
void Pump(unsigned milliseconds) {
  const auto end = GetTickCount64() + milliseconds;
  do {
    MSG msg{};
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }
    Sleep(5);
  } while (GetTickCount64() < end);
}
bool Check(bool result, const char* message) {
  if (!result) std::cerr << message << '\n';
  return result;
}
}
int main() {
  Application::GetInstance();
  owner = CreateWindowW(L"STATIC", L"Dialog test owner", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                        100, 100, 400, 300, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
  other = CreateWindowW(L"STATIC", L"Dialog test other", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                        520, 100, 400, 300, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
  ShowWindow(owner, SW_SHOW);
  ShowWindow(other, SW_SHOW);
  content = CreateWindowW(L"EDIT", L"Existing host content", WS_CHILD | WS_VISIBLE,
                          0, 0, 200, 100, owner, nullptr, GetModuleHandleW(nullptr), nullptr);
  disabled_content = CreateWindowW(L"BUTTON", L"Already disabled", WS_CHILD | WS_VISIBLE | WS_DISABLED,
                                   0, 100, 200, 40, owner, nullptr, GetModuleHandleW(nullptr), nullptr);
  MessageDialog dialog("WinUI3 dialog test", "Real ContentDialog\nUnicode: \xE4\xBD\xA0\xE5\xA5\xBD");
  active = &dialog;
  for (auto modality : {DialogModality::Window, DialogModality::Application}) {
    SetActiveWindow(owner);
    application_modal = modality == DialogModality::Application;
    dialog.SetModality(modality);
    auto timer = SetTimer(nullptr, 0, 1800, Dismiss);
    if (!timer) return 1;
    const bool opened = dialog.Open();
    KillTimer(nullptr, timer);
    if (!Check(opened && timer_passed, "Modal open/close/reentry/disable failed") ||
        !Check(IsWindowEnabled(owner) && IsWindowEnabled(other) && IsWindowEnabled(content) &&
               !IsWindowEnabled(disabled_content), "Original enabled states not restored")) return 1;
    Pump(200);
  }
  dialog.SetModality(DialogModality::None);
  SetActiveWindow(owner);
  SetFocus(content);
  if (!Check(dialog.Open(), "Modeless open failed") ||
      !Check(IsWindowEnabled(owner) && IsWindowEnabled(other) && !IsWindowEnabled(content),
             "Embedded dialog must keep its parent enabled and block underlying content") ||
      !Check(FindWindowW(L"nativeapi.WinUI3.MessageDialog", nullptr) == nullptr,
             "Parented dialog created an extra top-level host") ||
      !Check(!dialog.Open(), "Modeless reentry accepted")) return 1;
  // A second island must not steal the same parent's input or restore it early.
  MessageDialog duplicate("Duplicate", "Same parent");
  duplicate.SetParentWindow(std::make_shared<Window>(owner));
  if (!Check(!duplicate.Open(), "Second dialog on the same parent accepted")) return 1;
  SetWindowPos(owner, nullptr, 140, 160, 760, 580, SWP_NOZORDER | SWP_NOACTIVATE);
  Pump(200);
  RECT client{};
  GetClientRect(owner, &client);
  bool filling_island = false;
  for (HWND child = GetWindow(owner, GW_CHILD); child; child = GetWindow(child, GW_HWNDNEXT)) {
    if (child == content || child == disabled_content || !IsWindowVisible(child)) continue;
    RECT rect{};
    GetWindowRect(child, &rect);
    MapWindowPoints(nullptr, owner, reinterpret_cast<POINT*>(&rect), 2);
    if (EqualRect(&rect, &client) && IsWindowEnabled(child)) filling_island = true;
  }
  if (!Check(filling_island, "XAML island did not follow the parent client size") ||
      !Check(dialog.Close(), "Modeless Close failed")) return 1;
  Pump(1000);
  if (!Check(!dialog.Close(), "Closed twice") ||
      !Check(GetFocus() == content, "Host focus was not restored")) return 1;
  // Parentless/tray-only apps keep the standalone fallback and its close button.
  ShowWindow(owner, SW_HIDE);
  ShowWindow(other, SW_HIDE);
  SetActiveWindow(nullptr);
  if (!Check(dialog.Open(), "Reopen failed")) return 1;
  HWND host = FindWindowW(L"nativeapi.WinUI3.MessageDialog", L"Updated title");
  if (!Check(host != nullptr, "Host missing")) return 1;
  SendMessageW(host, WM_CLOSE, 0, 0);
  Pump(1000);
  if (!Check(!dialog.Close(), "Title-bar close failed")) return 1;
  ShowWindow(owner, SW_SHOW);
  ShowWindow(other, SW_SHOW);
  SetActiveWindow(owner);
  {
    MessageDialog temporary("Destruction", "Destroy while modeless");
    if (!temporary.Open()) return 1;
  }
  Pump(1000);
  // Native parent destruction must end a modeless session before destroying
  // its island; disposing/reopening the wrapper afterwards must stay safe.
  dialog.SetParentWindow(std::make_shared<Window>(owner));
  if (!Check(dialog.Open(), "Parent destruction setup failed")) return 1;
  DestroyWindow(owner);
  Pump(200);
  if (!Check(!dialog.IsOpen() && !dialog.Close() && IsWindowEnabled(other),
             "Parent destruction left an active dialog")) return 1;
  Menu menu;
  menu.AddItem(std::make_shared<MenuItem>("Shared WinUI3 runtime"));
  static Menu* current_menu;
  current_menu = &menu;
  auto timer = SetTimer(nullptr, 0, 1000, [](HWND, UINT, UINT_PTR id, DWORD) {
    KillTimer(nullptr, id);
    current_menu->Close();
  });
  if (!timer || !Check(menu.GetBackend() == MenuBackend::WinUI3 &&
      menu.Open(PositioningStrategy::Absolute({300, 300})), "Shared menu runtime failed")) return 1;
  KillTimer(nullptr, timer);
  DestroyWindow(other);
  std::cout << "WinUI3 dialog tests passed\n";
}
