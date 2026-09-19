// Menu item clicks must reach their listener before Menu::Open() returns.
//
// The native Windows backend used to let TrackPopupMenu post WM_COMMAND, which the
// host's message loop dispatches after Open() has returned; bindings whose callbacks
// are only valid inside the call (Dart's NativeCallable.isolateLocal) cannot receive
// an event that late.
//
// Without arguments this is a headless no-op, so CTest can run it. With a round name
// it opens a real menu and waits for a driver to work it with the mouse:
// tools/gui/core_menu_backend_test.ps1 in the libnativeapi workspace.
//
//   menu_click_test pick_top_level   driver clicks "Alpha"
//   menu_click_test dismiss          driver clicks the window, clear of the menu
//   menu_click_test pick_submenu     driver opens "More" and clicks "Beta"
//
// The driver creates go.flag in the working directory once it has activated the
// window, so that no activation click can race the menu open.
#include <windows.h>
#include <iostream>
#include <memory>
#include <string>
#include "nativeapi.h"

using namespace nativeapi;

namespace {

int alpha_clicks = 0, beta_clicks = 0, alpha_inside = 0, beta_inside = 0;
int opened = 0, closed = 0;
bool inside_open = false;
int failures = 0;

void Pump(int ms) {
  const ULONGLONG end = GetTickCount64() + ms;
  MSG msg;
  while (GetTickCount64() < end) {
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    Sleep(10);
  }
}

bool Check(bool condition, const std::string& what) {
  if (!condition) {
    ++failures;
    std::cout << "FAIL " << what << std::endl;
  } else {
    std::cout << "PASS " << what << std::endl;
  }
  return condition;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc <= 1) return 0;  // headless: nothing to drive the menu with
  const std::string round = argv[1];
  if (round != "pick_top_level" && round != "dismiss" && round != "pick_submenu") {
    std::cerr << "unknown round: " << round << '\n';
    return 2;
  }

  Application::GetInstance();

  // A window of our own: the menu behaves like it does in an app only when its owner
  // is the foreground window, and the driver needs something safe to click on.
  WNDCLASSW wc = {};
  wc.lpfnWndProc = DefWindowProcW;
  wc.hInstance = GetModuleHandleW(nullptr);
  wc.hCursor = LoadCursorW(nullptr, reinterpret_cast<LPCWSTR>(IDC_ARROW));
  wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
  wc.lpszClassName = L"NativeApiMenuClickTest";
  RegisterClassW(&wc);
  HWND host = CreateWindowExW(0, wc.lpszClassName, L"nativeapi menu click test",
                              WS_OVERLAPPEDWINDOW | WS_VISIBLE, 200, 200, 700, 520, nullptr,
                              nullptr, wc.hInstance, nullptr);
  if (!host) {
    std::cerr << "could not create the test window\n";
    return 2;
  }
  std::cout << "READY" << std::endl;

  Menu menu;
  if (!Check(menu.SetBackend(MenuBackend::Native), "native backend selected")) return 1;
  auto alpha = std::make_shared<MenuItem>("Alpha");
  menu.AddItem(alpha);
  auto submenu = std::make_shared<Menu>();
  auto beta = std::make_shared<MenuItem>("Beta");
  submenu->AddItem(beta);
  auto more = std::make_shared<MenuItem>("More", MenuItemType::Submenu);
  more->SetSubmenu(submenu);
  menu.AddItem(more);

  alpha->AddListener<MenuItemClickedEvent>([&](const MenuItemClickedEvent&) {
    ++alpha_clicks;
    if (inside_open) ++alpha_inside;
  });
  beta->AddListener<MenuItemClickedEvent>([&](const MenuItemClickedEvent&) {
    ++beta_clicks;
    if (inside_open) ++beta_inside;
  });
  menu.AddListener<MenuOpenedEvent>([&](const MenuOpenedEvent&) { ++opened; });
  menu.AddListener<MenuClosedEvent>([&](const MenuClosedEvent&) { ++closed; });

  // Wait for the driver's go flag; it activates the window first.
  const ULONGLONG deadline = GetTickCount64() + 60000;
  while (GetFileAttributesA("go.flag") == INVALID_FILE_ATTRIBUTES &&
         GetTickCount64() < deadline) {
    Pump(200);
  }
  Pump(300);
  const bool foreground = GetForegroundWindow() == host;
  std::cout << "GO foreground=" << foreground << std::endl;
  // A popup menu whose owner is not the foreground window ignores mouse input and
  // never closes, which would hang Open(): say so rather than time out silently.
  if (!Check(foreground, "test window is in the foreground")) return 1;

  inside_open = true;
  const bool returned = menu.Open(PositioningStrategy::Absolute({300, 300}));
  inside_open = false;

  Check(returned, "Open() reported success");
  Check(opened == 1 && closed == 1, "one opened and one closed event");
  if (round == "pick_top_level") {
    Check(alpha_inside == 1, "the click reached the listener before Open() returned");
    Check(beta_clicks == 0, "no other item fired");
  } else if (round == "pick_submenu") {
    Check(beta_inside == 1, "the submenu click reached the listener before Open() returned");
    Check(alpha_clicks == 0, "no other item fired");
  } else {
    Check(alpha_clicks == 0 && beta_clicks == 0, "dismissing fires no click");
  }

  const int before = alpha_clicks + beta_clicks;
  Pump(1500);  // a posted WM_COMMAND would be dispatched in here
  Check(alpha_clicks + beta_clicks == before, "no second click arrives after Open() returned");

  DestroyWindow(host);
  std::cout << (failures ? "FAILED" : "OK") << std::endl;
  return failures ? 1 : 0;
}
