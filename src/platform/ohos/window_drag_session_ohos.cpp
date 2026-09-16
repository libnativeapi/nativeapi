#include "../../window_drag_session.h"

namespace nativeapi {

// Windows are not dragged around a shared screen on this platform, so there is
// nothing to track: QueryPointer() fails and Start() returns false.
class WindowDragSession::Impl {};

WindowDragSession::WindowDragSession() : pimpl_(std::make_unique<Impl>()) {}

WindowDragSession::~WindowDragSession() = default;

void WindowDragSession::StartTicking() {}

void WindowDragSession::StopTicking() {}

bool WindowDragSession::QueryPointer(Point& /*position*/, bool& /*primary_button_down*/) const {
  return false;
}

void WindowDragSession::MoveWindow(Window& window, Point cursor_position) const {
  window.SetPosition({cursor_position.x - anchor_.x, cursor_position.y - anchor_.y});
}

}  // namespace nativeapi
