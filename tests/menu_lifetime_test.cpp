// Destroying a Menu or a MenuItem must not take the process down, and must not
// leave later menus without their events (issue 54).
//
// Both failures lived in WindowMessageDispatcher:
//   - DispatchWindowProc copied the handler list and walked the copy, so a menu
//     freed from a listener that runs inside the window procedure (a
//     MenuOpenedEvent listener, say) was still called afterwards, through a
//     dangling pimpl_. Windows kills a process whose window procedure faults
//     (STATUS_FATAL_USER_CALLBACK_EXCEPTION).
//   - UninstallHook() dropped the host window from original_procs_ when its last
//     handler went away, and InstallHook() never added it back, because the
//     procedure installed there was already DispatchWindowProc. After the first
//     menu had been destroyed, no menu ever saw another message.
//
// Needs a desktop session: it opens real popup menus (closed from a timer, no
// input required). Without arguments it is a headless no-op, so CTest can run it.
#include <windows.h>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include "nativeapi.h"

using namespace nativeapi;

namespace {

int failures = 0;

bool Check(bool condition, const std::string& what) {
  std::cout << (condition ? "PASS " : "FAIL ") << what << std::endl;
  if (!condition) ++failures;
  return condition;
}

Menu* closing = nullptr;
void CALLBACK CloseMenu(HWND, UINT, UINT_PTR timer, DWORD) {
  KillTimer(nullptr, timer);
  if (closing) closing->Close();
}

std::shared_ptr<Menu> MakeMenu(const std::string& prefix) {
  auto menu = std::make_shared<Menu>();
  menu->SetBackend(MenuBackend::Native);
  for (int i = 1; i <= 3; ++i) {
    menu->AddItem(std::make_shared<MenuItem>(prefix + " " + std::to_string(i)));
  }
  auto submenu = std::make_shared<Menu>();
  submenu->AddItem(std::make_shared<MenuItem>(prefix + " child"));
  auto parent = std::make_shared<MenuItem>(prefix + " submenu", MenuItemType::Submenu);
  parent->SetSubmenu(submenu);
  menu->AddItem(parent);
  return menu;
}

// Opens the menu and closes it again from a timer. Returns false when Open()
// failed or never came back with the lifecycle events.
bool OpenAndClose(const std::shared_ptr<Menu>& menu,
                  const std::string& what,
                  const std::function<void()>& on_opened = nullptr) {
  int opened = 0, closed = 0;
  const size_t opened_listener = menu->AddListener<MenuOpenedEvent>(
      [&](const MenuOpenedEvent&) {
        ++opened;
        if (on_opened) on_opened();
      });
  const size_t closed_listener =
      menu->AddListener<MenuClosedEvent>([&](const MenuClosedEvent&) { ++closed; });

  closing = menu.get();
  const UINT_PTR timer = SetTimer(nullptr, 0, 800, CloseMenu);
  const bool returned = timer && menu->Open(PositioningStrategy::Absolute({300, 300}));
  if (timer) KillTimer(nullptr, timer);
  closing = nullptr;

  menu->RemoveListener(opened_listener);
  menu->RemoveListener(closed_listener);
  return Check(returned, what + ": Open() returned") &
         Check(opened == 1 && closed == 1,
               what + ": one opened and one closed event (got " + std::to_string(opened) +
                   " and " + std::to_string(closed) + ")");
}

}  // namespace

int main(int argc, char**) {
  if (argc <= 1) return 0;  // headless: no desktop to open a menu on
  Application::GetInstance();

  // 1. A menu destroyed from inside a listener that runs in the window
  //    procedure. The dispatcher is walking its handlers at that moment, and the
  //    victim's handlers are older, so they come later in the walk.
  std::cout << "step 1: a menu destroyed from a listener inside the window procedure"
            << std::endl;
  auto victim = MakeMenu("Victim");
  OpenAndClose(victim, "warm-up open of the menu about to be destroyed");

  auto survivor = MakeMenu("Survivor");
  OpenAndClose(survivor, "menu that destroys another while opening", [&victim]() {
    victim.reset();
  });
  Check(!victim, "the other menu was really destroyed");

  // 2. With every menu gone the dispatcher has no handler left for the host
  //    window, and it used to stop listening for good. A fresh menu must still
  //    get its events.
  survivor.reset();
  std::cout << "step 2: a menu created after every other menu was destroyed" << std::endl;
  auto later = MakeMenu("Later");
  OpenAndClose(later, "menu created after every other menu was destroyed");

  std::cout << (failures ? "FAILED" : "OK") << std::endl;
  return failures ? 1 : 0;
}
