#include <windows.h>
#include <iostream>
#include "nativeapi.h"

using namespace nativeapi;
namespace {
MessageDialog* active = nullptr;
HWND owner = nullptr;
HWND other = nullptr;
bool timer_passed = false;
bool application_modal = false;
void CALLBACK Dismiss(HWND, UINT, UINT_PTR id, DWORD) {
  KillTimer(nullptr, id);
  timer_passed = !IsWindowEnabled(owner) &&
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
        !Check(IsWindowEnabled(owner) && IsWindowEnabled(other), "Windows not restored")) return 1;
    Pump(200);
  }
  dialog.SetModality(DialogModality::None);
  if (!Check(dialog.Open(), "Modeless open failed") ||
      !Check(IsWindowEnabled(owner) && IsWindowEnabled(other), "Modeless blocked windows") ||
      !Check(!dialog.Open(), "Modeless reentry accepted") ||
      !Check(dialog.Close(), "Modeless Close failed")) return 1;
  Pump(1000);
  if (!Check(!dialog.Close(), "Closed twice")) return 1;
  // Closing via the native title-bar must dismiss the ContentDialog too.
  if (!Check(dialog.Open(), "Reopen failed")) return 1;
  HWND host = FindWindowW(L"nativeapi.WinUI3.MessageDialog", L"Updated title");
  if (!Check(host != nullptr, "Host missing")) return 1;
  SendMessageW(host, WM_CLOSE, 0, 0);
  Pump(1000);
  if (!Check(!dialog.Close(), "Title-bar close failed")) return 1;
  {
    MessageDialog temporary("Destruction", "Destroy while modeless");
    if (!temporary.Open()) return 1;
  }
  Pump(1000);
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
  DestroyWindow(owner);
  std::cout << "WinUI3 dialog tests passed\n";
}
