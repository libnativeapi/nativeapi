#include <chrono>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "nativeapi.h"

using nativeapi::Point;
using nativeapi::Window;
using nativeapi::WindowManager;

// Shows what Window::SetParentWindow() does, and checks it against what the
// header promises for the platform: the exit code is the number of expectations
// that did not hold. "INFO" lines report behaviour the header leaves open.
//
// It drives the windows itself and needs no input.
int main() {
  nativeapi::SetMainThread();
  WindowManager& window_manager = WindowManager::GetInstance();

#if defined(__APPLE__)
  const bool child_moves_with_parent = true;
#else
  const bool child_moves_with_parent = false;
#endif

  int failures = 0;
  auto expect = [&](const std::string& what, bool ok, const std::string& detail = "") {
    std::cout << (ok ? "PASS " : "FAIL ") << what;
    if (!detail.empty()) {
      std::cout << " - " << detail;
    }
    std::cout << std::endl;
    if (!ok) {
      failures++;
    }
  };
  auto describe = [](const Point& p) {
    return std::to_string(std::lround(p.x)) + "," + std::to_string(std::lround(p.y));
  };
  auto parent_id_of = [](const std::shared_ptr<Window>& window) {
    auto parent = window->GetParentWindow();
    return parent ? parent->GetId() : 0;
  };

  std::shared_ptr<Window> parent;
  std::shared_ptr<Window> child;
  Point child_before = {0, 0};

  // Moves the parent and reports how far the child went along.
  auto move_parent = [&] {
    child_before = child->GetPosition();
    Point position = parent->GetPosition();
    parent->SetPosition({position.x + 120, position.y + 60});
  };
  auto child_followed = [&] {
    Point now = child->GetPosition();
    return std::lround(now.x - child_before.x) == 120 && std::lround(now.y - child_before.y) == 60;
  };

  std::vector<std::function<void()>> steps = {
      [&] {
        std::cout << "show a parent, create a hidden child on top of it" << std::endl;
        parent = std::make_shared<Window>();
        parent->SetTitle("Parent");
        parent->SetSize({520, 360}, false);
        parent->SetPosition({200, 200});
        parent->Show();
        child = std::make_shared<Window>();
        child->SetTitle("Child");
        child->SetSize({260, 140}, false);
        child->SetPosition({330, 310});
      },
      [&] {
        expect("no parent to begin with", child->GetParentWindow() == nullptr);
        expect("a window cannot be its own parent", !child->SetParentWindow(child));
        expect("set the parent of a hidden child", child->SetParentWindow(parent));
        expect("the parent is reported", parent_id_of(child) == parent->GetId());
      },
      [&] {
        expect("the hidden child stays hidden", !child->IsVisible());
        std::cout << "show the child" << std::endl;
        child->Show();
      },
      [&] {
        expect("the child is visible", child->IsVisible());
        expect("the parent is still reported", parent_id_of(child) == parent->GetId());
        expect("a parent cannot become the child of its child", !parent->SetParentWindow(child));
        std::cout << "focus the parent" << std::endl;
        parent->Focus();
      },
      [&] {
        Point inside_child = child->GetPosition();
        Point parent_position = parent->GetPosition();
        auto top = window_manager.GetWindowAtPoint({inside_child.x + 130, inside_child.y + 100}, 0);
        if (inside_child.x == parent_position.x && inside_child.y == parent_position.y) {
          // Wayland: a client is never told where its windows are, so there is
          // no point to ask about.
          std::cout << "SKIP the child stays above its focused parent - positions are not known"
                    << std::endl;
        } else if (!top) {
          std::cout << "SKIP the child stays above its focused parent - no window at the point"
                    << std::endl;
        } else {
          expect("the child stays above its focused parent", top->GetId() == child->GetId());
        }
        std::cout << "move the parent" << std::endl;
        move_parent();
      },
      [&] {
        if (child_moves_with_parent) {
          expect("the child moved with its parent", child_followed(),
                 describe(child_before) + " -> " + describe(child->GetPosition()));
        } else {
          expect("the child did not move with its parent", !child_followed(),
                 describe(child_before) + " -> " + describe(child->GetPosition()));
        }
        std::cout << "hide the child, focus the parent" << std::endl;
        child->Hide();
        parent->Focus();
      },
      [&] {
        expect("the hidden child is not brought back by its parent", !child->IsVisible());
        expect("a hidden child keeps its parent", parent_id_of(child) == parent->GetId());
        std::cout << "show the child again, move the parent" << std::endl;
        child->Show();
      },
      [&] { move_parent(); },
      [&] {
        if (child_moves_with_parent) {
          expect("the child shown again moved with its parent", child_followed(),
                 describe(child_before) + " -> " + describe(child->GetPosition()));
        }
        std::cout << "clear the parent, move the former parent" << std::endl;
        expect("clear the parent", child->SetParentWindow(nullptr));
        expect("no parent is reported", child->GetParentWindow() == nullptr);
      },
      [&] { move_parent(); },
      [&] {
        expect("an independent window does not move along", !child_followed(),
               describe(child_before) + " -> " + describe(child->GetPosition()));
        std::cout << (failures == 0 ? "ALL PASS" : "FAILED") << std::endl;
        // Not Quit(): how a platform's loop ends must not decide the exit code.
        std::cout.flush();
        std::_Exit(failures);
      },
  };

  std::thread driver([&] {
    for (auto& step : steps) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1200));
      nativeapi::RunOnMainThread(step);
    }
  });
  driver.detach();

  // On Windows destroying any window of the library ends the loop (its window
  // procedure posts WM_QUIT), so enter it again: the last step ends the process.
  while (true) {
    nativeapi::Application::GetInstance().Run();
  }
}
