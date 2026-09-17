#include "../../drag_source_impl.h"

namespace nativeapi {

// There is no shared screen to drag data across on this platform.
struct DragSource::Impl::Platform {};

bool DragSource::IsSupported() {
  return false;
}

DragSource::Impl::Impl(DragSource* owner)
    : owner(owner), platform(std::make_unique<Platform>()) {}

DragSource::Impl::~Impl() = default;

bool DragSource::Impl::Start() {
  return false;
}

}  // namespace nativeapi
