#pragma once

#include <memory>
#include <string>
#include <vector>

#include "drop_target.h"
#include "window.h"

namespace nativeapi {

// Shared state of a DropTarget (drop_target.cpp); the platform files define
// Platform, the destructor, IsSupported(), Register() and Unregister().
class DropTarget::Impl {
 public:
  Impl(DropTarget* owner, std::shared_ptr<Window> window);
  ~Impl();

  // Makes the window a drop destination. Returns false if the platform or the
  // window cannot accept drops. Only called while not registered.
  bool Register();
  // Undoes Register(). Only called while registered.
  void Unregister();

  // Called by the platform code on the main thread. Entered() decides whether
  // the drag is accepted; the others do nothing for a drag that was not.
  // `source_operations` is the set the source offers, as a mask of
  // OperationBit() values.
  DragOperation Entered(Point position, unsigned source_operations, bool has_data);
  DragOperation Moved(Point position, unsigned source_operations);
  void Exited(Point position);
  void Dropped(Point position, std::vector<std::string> file_paths, std::string text);

  static unsigned OperationBit(DragOperation operation);

  DropTarget* owner;
  std::shared_ptr<Window> window;
  WindowId window_id;
  DragOperation operation = DragOperation::Copy;
  bool registered = false;

  // The accepted drag in progress.
  bool accepted = false;
  Point last_position{0, 0};

  struct Platform;
  std::unique_ptr<Platform> platform;
};

}  // namespace nativeapi
