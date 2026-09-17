#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "nativeapi.h"

using nativeapi::DragOperation;
using nativeapi::DragSource;
using nativeapi::DragSourceEndedEvent;
using nativeapi::DropTarget;
using nativeapi::DropTargetDroppedEvent;
using nativeapi::DropTargetEnteredEvent;
using nativeapi::DropTargetExitedEvent;
using nativeapi::DropTargetMovedEvent;
using nativeapi::RunApp;
using nativeapi::Window;
using nativeapi::WindowFocusedEvent;
using nativeapi::WindowManager;

namespace {

const char* OperationName(DragOperation operation) {
  switch (operation) {
    case DragOperation::Copy:
      return "copy";
    case DragOperation::Move:
      return "move";
    case DragOperation::Link:
      return "link";
    case DragOperation::None:
      break;
  }
  return "none";
}

}  // namespace

// Drag and drop without any UI toolkit.
//
// - Drop files or text from a file manager or editor onto the "Drop here"
//   window; every event is printed.
// - Press inside the "Drag me" window (not on its title bar) and keep dragging
//   to drag a text file out of it, into a file manager or the "Drop here"
//   window. There is no widget to press on, so the window becoming focused
//   stands in for the mouse-down.
int main() {
  WindowManager& window_manager = WindowManager::GetInstance();

  auto drop_window = std::make_shared<Window>();
  drop_window->SetTitle("Drop here");
  drop_window->SetSize({480, 320}, false);
  drop_window->Center();

  auto source_window = std::make_shared<Window>();
  source_window->SetTitle("Drag me");
  source_window->SetSize({240, 160}, false);
  auto position = drop_window->GetPosition();
  source_window->SetPosition({position.x + 520, position.y});
  source_window->Show();

  std::cout << "Drop target supported: " << DropTarget::IsSupported()
            << ", drag source supported: " << DragSource::IsSupported() << std::endl;

  auto target = std::make_shared<DropTarget>(drop_window);
  target->AddListener<DropTargetEnteredEvent>([](const DropTargetEnteredEvent& event) {
    auto p = event.GetPosition();
    std::cout << "Entered at (" << p.x << ", " << p.y << ")" << std::endl;
  });
  target->AddListener<DropTargetMovedEvent>([](const DropTargetMovedEvent& event) {
    auto p = event.GetPosition();
    std::cout << "Moved to (" << p.x << ", " << p.y << ")" << std::endl;
  });
  target->AddListener<DropTargetExitedEvent>([](const DropTargetExitedEvent&) {
    std::cout << "Exited" << std::endl;
  });
  target->AddListener<DropTargetDroppedEvent>([](const DropTargetDroppedEvent& event) {
    auto p = event.GetPosition();
    std::cout << "Dropped at (" << p.x << ", " << p.y << ")" << std::endl;
    for (const auto& path : event.GetFilePaths()) {
      std::cout << "  file: " << path << std::endl;
    }
    if (!event.GetText().empty()) {
      std::cout << "  text: " << event.GetText() << std::endl;
    }
  });
  std::cout << "Drop target active: " << target->IsActive() << std::endl;
  auto content = drop_window->GetContentBounds();
  std::cout << "Drop content at (" << content.x << ", " << content.y << ")" << std::endl;

  // A file to drag out.
  auto file = std::filesystem::temp_directory_path() / "nativeapi-drag-drop-example.txt";
  std::ofstream(file) << "Dragged out of drag_drop_example.\n";

  auto source = std::make_shared<DragSource>();
  source->SetFilePaths({file.string()});
  source->SetText("Hello from drag_drop_example");
  source->AddListener<DragSourceEndedEvent>([](const DragSourceEndedEvent& event) {
    std::cout << "Drag ended: " << OperationName(event.GetOperation()) << std::endl;
  });

  window_manager.AddListener<WindowFocusedEvent>([&](const WindowFocusedEvent& event) {
    if (event.GetWindowId() != source_window->GetId() || source->IsDragging()) {
      return;
    }
    if (source->StartDragging(source_window)) {
      std::cout << "Dragging " << file.string() << std::endl;
    }
  });

  std::cout << "Drop something on \"Drop here\", or press inside \"Drag me\" and drag."
            << std::endl;
  return RunApp(drop_window);
}
