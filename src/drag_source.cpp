#include "drag_source_impl.h"

namespace nativeapi {

DragSource::DragSource() : pimpl_(std::make_unique<Impl>(this)) {}

DragSource::~DragSource() = default;

void DragSource::SetFilePaths(const std::vector<std::string>& file_paths) {
  pimpl_->file_paths = file_paths;
}

std::vector<std::string> DragSource::GetFilePaths() const {
  return pimpl_->file_paths;
}

void DragSource::SetText(const std::optional<std::string>& text) {
  pimpl_->text = text;
}

std::optional<std::string> DragSource::GetText() const {
  return pimpl_->text;
}

void DragSource::SetImage(std::shared_ptr<Image> image) {
  pimpl_->image = std::move(image);
}

std::shared_ptr<Image> DragSource::GetImage() const {
  return pimpl_->image;
}

void DragSource::SetDragOperation(DragOperation operation) {
  pimpl_->operation = operation;
}

DragOperation DragSource::GetDragOperation() const {
  return pimpl_->operation;
}

bool DragSource::StartDragging(std::shared_ptr<Window> window) {
  if (!IsSupported() || pimpl_->dragging || !window || !window->GetNativeObject()) {
    return false;
  }
  if (pimpl_->operation == DragOperation::None ||
      (pimpl_->file_paths.empty() && !pimpl_->text.has_value())) {
    return false;
  }
  pimpl_->dragging = true;
  pimpl_->window = std::move(window);
  if (!pimpl_->Start()) {
    pimpl_->dragging = false;
    pimpl_->window.reset();
    return false;
  }
  return true;
}

bool DragSource::IsDragging() const {
  return pimpl_->dragging;
}

void DragSource::Impl::Finish(Point position, DragOperation result) {
  if (!dragging) {
    return;
  }
  dragging = false;
  // Keep the window alive until listeners have seen the event.
  const std::shared_ptr<Window> source_window = std::move(window);
  owner->Emit(DragSourceEndedEvent(source_window->GetId(), position, result));
}

}  // namespace nativeapi
