#include <iostream>
#include "nativeapi.h"

using nativeapi::Point;
using nativeapi::RunApp;
using nativeapi::Window;
using nativeapi::WindowDragCancelledEvent;
using nativeapi::WindowDragEndedEvent;
using nativeapi::WindowDragMovedEvent;
using nativeapi::WindowDragSession;
using nativeapi::WindowFocusedEvent;
using nativeapi::WindowManager;

// A tear-off panel without any UI toolkit.
//
// There is no widget to press on, so a window becoming focused stands in for
// "mouse down on the panel": press inside the Panel window (not on its title
// bar, which starts the system's own window drag) and keep dragging to carry
// it, drop it onto the Dock window to dock it, and press inside the Dock window
// to tear the panel off again.
int main() {
  WindowManager& window_manager = WindowManager::GetInstance();

  auto dock = std::make_shared<Window>();
  dock->SetTitle("Dock - drop the panel here");
  dock->SetSize({640, 420}, false);
  dock->Center();

  auto panel = std::make_shared<Window>();
  panel->SetTitle("Panel - drag me");
  panel->SetSize({260, 180}, false);
  Point dock_position = dock->GetPosition();
  panel->SetPosition({dock_position.x + 680, dock_position.y});
  panel->Show();

  auto session = std::make_shared<WindowDragSession>();
  bool docked = false;
  bool over_dock = false;
  // Hiding the panel on drop hands focus to the dock; that is not a press.
  bool dropping = false;
  // Focus changes are not always presses (and arrive late on some platforms),
  // so a tear-off only happens once the cursor moves with the button down.
  bool tear_off_pending = false;

  auto contains = [](const nativeapi::Rectangle& r, const Point& p) {
    return p.x >= r.x && p.y >= r.y && p.x < r.x + r.width && p.y < r.y + r.height;
  };

  window_manager.AddListener<WindowFocusedEvent>([&](const WindowFocusedEvent& event) {
    if (session->IsActive() || dropping) {
      return;
    }
    Point cursor = nativeapi::DisplayManager::GetInstance().GetCursorPosition();

    if (event.GetWindowId() == panel->GetId() && !docked) {
      // Keep the point that was pressed under the cursor.
      Point position = panel->GetPosition();
      session->Start(panel, {cursor.x - position.x, cursor.y - position.y});
    } else if (event.GetWindowId() == dock->GetId() && docked &&
               contains(dock->GetContentBounds(), cursor)) {
      // Follow the pointer; the panel only comes out once it moves.
      tear_off_pending = session->Start(nullptr, {0, 0});
    }
  });

  session->AddListener<WindowDragMovedEvent>([&](const WindowDragMovedEvent& event) {
    if (tear_off_pending) {
      // Tear off: bring the panel back under the cursor, grabbed by its title bar.
      tear_off_pending = false;
      Point cursor = event.GetCursorPosition();
      Point anchor{130, 12};
      panel->SetPosition({cursor.x - anchor.x, cursor.y - anchor.y});
      panel->Show();
      docked = false;
      session->Start(panel, anchor);
      dock->SetTitle("Dock - drop the panel here");
      std::cout << "Panel torn off" << std::endl;
      return;
    }
    auto target = window_manager.GetWindowAtPoint(event.GetCursorPosition(), panel->GetId());
    bool now_over_dock = target && target->GetId() == dock->GetId();
    if (now_over_dock != over_dock) {
      over_dock = now_over_dock;
      panel->SetOpacity(over_dock ? 0.5f : 1.0f);
      std::cout << (over_dock ? "Over the dock: release to dock" : "Left the dock") << std::endl;
    }
  });

  session->AddListener<WindowDragEndedEvent>([&](const WindowDragEndedEvent& event) {
    if (tear_off_pending) {
      tear_off_pending = false;  // Released without moving: just a click.
      return;
    }
    Point cursor = event.GetCursorPosition();
    std::cout << "Drag ended at (" << cursor.x << ", " << cursor.y << ")" << std::endl;
    panel->SetOpacity(1.0f);
    if (over_dock) {
      over_dock = false;
      docked = true;
      dropping = true;
      panel->Hide();
      dropping = false;
      dock->SetTitle("Dock - panel docked, press here to tear it off");
      std::cout << "Panel docked" << std::endl;
    }
  });

  session->AddListener<WindowDragCancelledEvent>([&](const WindowDragCancelledEvent&) {
    tear_off_pending = false;
    std::cout << "Drag cancelled" << std::endl;
  });

  std::cout << "Press inside the Panel window and drag it onto the Dock window to dock it."
            << std::endl;
  return RunApp(dock);
}
