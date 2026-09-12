#include <windows.h>
#include <iostream>
#include "nativeapi.h"

using namespace nativeapi;
namespace {
Menu* active_menu = nullptr;
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
  active_menu = nullptr;
  if (preview) DestroyWindow(preview);
  if (!Check(opened == 2 && closed == 2 && rejected_reentry, "Lifecycle mismatch")) return 1;
  menu.AddListener<MenuOpenedEvent>([&](const auto&) { menu.Close(); });
  if (!Check(menu.Open(PositioningStrategy::Absolute({300, 300})), "Opening cancellation failed") ||
      !Check(opened == 3 && closed == 3, "Opening cancellation lifecycle mismatch")) return 1;
  std::cout << "WinUI3 smoke test passed\n";
  return 0;
}
