#include "drop_target_impl.h"

#include <cmath>

namespace nativeapi {

DropTarget::DropTarget(std::shared_ptr<Window> window)
    : pimpl_(std::make_unique<Impl>(this, std::move(window))) {}

DropTarget::~DropTarget() {
  if (pimpl_->registered) {
    pimpl_->Unregister();
    pimpl_->registered = false;
  }
}

WindowId DropTarget::GetWindowId() const {
  return pimpl_->window_id;
}

void DropTarget::SetDropOperation(DragOperation operation) {
  pimpl_->operation = operation;
}

DragOperation DropTarget::GetDropOperation() const {
  return pimpl_->operation;
}

bool DropTarget::IsActive() const {
  return pimpl_->registered;
}

void DropTarget::StartEventListening() {
  if (pimpl_->registered || !pimpl_->window || !pimpl_->window->GetNativeObject()) {
    return;
  }
  pimpl_->registered = pimpl_->Register();
}

void DropTarget::StopEventListening() {
  if (!pimpl_->registered) {
    return;
  }
  pimpl_->Unregister();
  pimpl_->registered = false;
  pimpl_->accepted = false;
}

unsigned DropTarget::Impl::OperationBit(DragOperation operation) {
  return operation == DragOperation::None ? 0u : 1u << static_cast<unsigned>(operation);
}

DragOperation DropTarget::Impl::Entered(Point position,
                                        unsigned source_operations,
                                        bool has_data) {
  accepted = has_data && (source_operations & OperationBit(operation)) != 0;
  if (!accepted) {
    return DragOperation::None;
  }
  last_position = position;
  owner->Emit(DropTargetEnteredEvent(window_id, position));
  return operation;
}

DragOperation DropTarget::Impl::Moved(Point position, unsigned source_operations) {
  if (!accepted) {
    return DragOperation::None;
  }
  // The source may change what it offers mid-drag (a modifier key).
  if ((source_operations & OperationBit(operation)) == 0) {
    return DragOperation::None;
  }
  const bool moved = std::fabs(position.x - last_position.x) >= 0.5 ||
                     std::fabs(position.y - last_position.y) >= 0.5;
  if (moved) {
    last_position = position;
    owner->Emit(DropTargetMovedEvent(window_id, position));
  }
  return operation;
}

void DropTarget::Impl::Exited(Point position) {
  if (!accepted) {
    return;
  }
  accepted = false;
  owner->Emit(DropTargetExitedEvent(window_id, position));
}

void DropTarget::Impl::Dropped(Point position,
                               std::vector<std::string> file_paths,
                               std::string text) {
  if (!accepted) {
    return;
  }
  accepted = false;
  owner->Emit(DropTargetDroppedEvent(window_id, position, std::move(file_paths), std::move(text)));
}

}  // namespace nativeapi
