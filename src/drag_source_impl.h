#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "drag_source.h"
#include "window.h"

namespace nativeapi {

// Shared state of a DragSource (drag_source.cpp); the platform files define
// Platform, the destructor, IsSupported() and Start().
class DragSource::Impl {
 public:
  explicit Impl(DragSource* owner);
  ~Impl();

  // Begins the platform drag. Called with valid data and window, while no drag
  // is in progress; `dragging` is already set and is cleared again on failure.
  bool Start();

  // Ends the drag in progress: releases the window and emits
  // DragSourceEndedEvent. Does nothing when no drag is in progress.
  void Finish(Point position, DragOperation operation);

  DragSource* owner;
  std::vector<std::string> file_paths;
  std::optional<std::string> text;
  std::shared_ptr<Image> image;
  DragOperation operation = DragOperation::Copy;

  // The drag in progress.
  bool dragging = false;
  std::shared_ptr<Window> window;

  struct Platform;
  std::unique_ptr<Platform> platform;
};

}  // namespace nativeapi
