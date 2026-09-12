#include <windows.h>
#include <iostream>
#include "nativeapi.h"

using namespace nativeapi;
namespace {
Menu* active_menu = nullptr;
HWND occluding_panel = nullptr;
bool above_panel = false;
void CALLBACK CheckMenuAbovePanel(HWND, UINT, UINT_PTR timer, DWORD) {
  KillTimer(nullptr, timer);
  // The anchor is (300, 300); this point lies inside the first menu item.
  const HWND hit = GetAncestor(WindowFromPoint({320, 320}), GA_ROOT);
  DWORD process = 0;
  GetWindowThreadProcessId(hit, &process);
  above_panel = hit && hit != occluding_panel && process == GetCurrentProcessId();
  active_menu->Close();
}
void CALLBACK CloseMenu(HWND, UINT, UINT_PTR timer, DWORD) {
  KillTimer(nullptr, timer);
  if (active_menu) {
    auto item = active_menu->GetItemAt(0);
    if (item) {
      item->SetLabel("Updated while open");
      item->SetEnabled(false);
      item->SetTooltip("Updated tooltip");
    }
    active_menu->Close();
  }
}
bool Check(bool condition, const char* message) {
  if (!condition) std::cerr << message << '\n';
  return condition;
}
}

int main(int argc, char**) {
  Application::GetInstance();
  Menu menu;
  if (!Check(menu.GetBackend() == (Menu::IsBackendSupported(MenuBackend::WinUI3) ? MenuBackend::WinUI3 : MenuBackend::Native), "Default changed") ||
      !Check(Menu::IsBackendSupported(MenuBackend::Native), "Native unavailable") ||
      !Check(!menu.SetBackend(static_cast<MenuBackend>(100)), "Invalid enum accepted")) return 1;
  const bool supported = Menu::IsBackendSupported(MenuBackend::WinUI3);
  if (!Check(menu.SetBackend(MenuBackend::WinUI3) == supported, "Capability mismatch")) return 1;
  Menu wrapped(CreatePopupMenu());
  if (!Check(!wrapped.SetBackend(MenuBackend::WinUI3), "Wrapped HMENU accepted modern backend")) return 1;
  if (argc <= 1) return 0;
  if (!supported) return 1;
  HWND preview = nullptr;
  if (argc > 2) {
    preview = CreateWindowExW(0, L"STATIC", L"Modern menu verification", WS_OVERLAPPEDWINDOW,
                             100, 100, 700, 600, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
    SetWindowPos(preview, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
  }
  auto item = std::make_shared<MenuItem>("Modern menu smoke test");
  menu.AddItem(item);
  auto checkbox = std::make_shared<MenuItem>("Checked", MenuItemType::Checkbox);
  checkbox->SetState(MenuItemState::Checked);
  menu.AddItem(checkbox);
  auto radio = std::make_shared<MenuItem>("Radio", MenuItemType::Radio);
  radio->SetState(MenuItemState::Checked);
  menu.AddItem(radio);
  menu.AddSeparator();
  auto child = std::make_shared<Menu>();
  child->AddItem(std::make_shared<MenuItem>("Child item"));
  auto parent = std::make_shared<MenuItem>("Submenu", MenuItemType::Submenu);
  parent->SetSubmenu(child);
  menu.AddItem(parent);
  int opened = 0, closed = 0;
  bool rejected_reentry = false;
  menu.AddListener<MenuOpenedEvent>([&](const auto&) {
    ++opened;
    rejected_reentry = !menu.SetBackend(MenuBackend::Native) &&
                      !menu.Open(PositioningStrategy::CursorPosition());
  });
  menu.AddListener<MenuClosedEvent>([&](const auto&) { ++closed; });
  active_menu = &menu;
  for (int i = 0; i < 2; ++i) {
    auto timer = SetTimer(nullptr, 0, argc > 2 ? 60000 : 1500, CloseMenu);
    if (!timer) return 1;
    const bool result = menu.Open(PositioningStrategy::Absolute({300, 300}));
    KillTimer(nullptr, timer);
    if (!Check(result, "WinUI3 Open failed")) return 1;
  }
  // Model the notification overflow panel: visible, topmost, and still open
  // when a menu is requested by a tray callback.
  HWND menu_owner = CreateWindowW(L"STATIC", L"Non-topmost menu owner", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
      100, 100, 700, 600, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
  SetActiveWindow(menu_owner);
  occluding_panel = CreateWindowExW(WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW,
      L"STATIC", L"Tray overflow test panel", WS_POPUP,
      280, 280, 500, 500, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
  ShowWindow(occluding_panel, SW_SHOWNOACTIVATE);
  auto z_timer = SetTimer(nullptr, 0, 1500, CheckMenuAbovePanel);
  const bool z_opened = z_timer && menu.Open(PositioningStrategy::Absolute({300, 300}));
  if (z_timer) KillTimer(nullptr, z_timer);
  DestroyWindow(occluding_panel);
  const bool owner_unchanged = !(GetWindowLongPtrW(menu_owner, GWL_EXSTYLE) & WS_EX_TOPMOST);
  DestroyWindow(menu_owner);
  if (!Check(z_opened && above_panel, "Menu remained underneath a topmost panel") ||
      !Check(owner_unchanged, "Menu made its owner permanently topmost") ||
      !Check(FindWindowW(L"STATIC", L"nativeapi WinUI menu") == nullptr,
             "Temporary menu host leaked")) return 1;
  active_menu = nullptr;
  if (preview) DestroyWindow(preview);
  if (!Check(opened == 3 && closed == 3 && rejected_reentry, "Lifecycle mismatch")) return 1;
  menu.AddListener<MenuOpenedEvent>([&](const auto&) { menu.Close(); });
  if (!Check(menu.Open(PositioningStrategy::Absolute({300, 300})), "Opening cancellation failed") ||
      !Check(opened == 4 && closed == 4, "Opening cancellation lifecycle mismatch")) return 1;
  std::cout << "WinUI3 smoke test passed\n";
  return 0;
}
