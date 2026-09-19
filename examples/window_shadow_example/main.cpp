#include <chrono>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "nativeapi.h"

using nativeapi::Size;
using nativeapi::Window;

// Shows what Window::SetHasShadow() does to a window's state and - the part that went
// wrong on Linux - to its content size: the exit code is the number of expectations that
// did not hold. Whether a shadow is really drawn has to be looked at; "INFO" lines say
// what the platform reports.
//
// It drives the windows itself and needs no input.
int main() {
  nativeapi::SetMainThread();

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
  auto describe = [](const Size& s) {
    return std::to_string(std::lround(s.width)) + " x " + std::to_string(std::lround(s.height));
  };
  auto same = [](const Size& a, const Size& b) {
    return std::lround(a.width) == std::lround(b.width) &&
           std::lround(a.height) == std::lround(b.height);
  };

  std::shared_ptr<Window> shown;
  std::shared_ptr<Window> early;
  Size before = {0, 0};
  // Platforms where the shadow is not the application's to remove report it unchanged.
  bool can_remove = true;

  std::vector<std::function<void()>> steps = {
      [&] {
        std::cout << "show a window" << std::endl;
        shown = std::make_shared<Window>();
        shown->SetTitle("Shadow - toggled while shown");
        shown->SetContentSize({480, 320});
        shown->SetPosition({200, 200});
        shown->Show();
      },
      [&] {
        before = shown->GetContentSize();
        expect("a window has a shadow to begin with", shown->HasShadow());
        std::cout << "remove the shadow" << std::endl;
        shown->SetHasShadow(false);
      },
      [&] {
        can_remove = !shown->HasShadow();
        std::cout << "INFO the platform " << (can_remove ? "removed" : "kept") << " the shadow"
                  << std::endl;
        expect("the content keeps its size without the shadow",
               same(shown->GetContentSize(), before),
               describe(before) + " -> " + describe(shown->GetContentSize()));
        std::cout << "bring the shadow back" << std::endl;
        shown->SetHasShadow(true);
      },
      [&] {
        expect("the shadow is reported again", shown->HasShadow());
        expect("the content keeps its size with the shadow back",
               same(shown->GetContentSize(), before),
               describe(before) + " -> " + describe(shown->GetContentSize()));
        std::cout << "a second window: no shadow and a content size, both before it is shown"
                  << std::endl;
        early = std::make_shared<Window>();
        early->SetTitle("Shadow - removed before shown");
        early->SetHasShadow(false);
        early->SetContentSize({300, 200});
        early->SetPosition({760, 200});
        if (can_remove) {
          expect("the state is reported before the window is shown", !early->HasShadow());
        }
        early->Show();
      },
      [&] {
        if (can_remove) {
          expect("the state holds once the window is shown", !early->HasShadow());
        }
        expect("the content has the size it was given", same(early->GetContentSize(), {300, 200}),
               describe(early->GetContentSize()));
        std::cout << (failures == 0 ? "ALL PASS" : "FAILED") << std::endl;
        // Not Quit(): how a platform's loop ends must not decide the exit code.
        std::cout.flush();
        std::_Exit(failures);
      },
  };

  std::thread driver([&] {
    for (auto& step : steps) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1500));
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
