#include "window_drag_session.h"

#include <cmath>

namespace nativeapi {

bool WindowDragSession::Start(std::shared_ptr<Window> window, Point anchor) {
  Point cursor_position{0, 0};
  bool primary_button_down = false;
  if (!QueryPointer(cursor_position, primary_button_down)) {
    return false;
  }

  // Retargeting an active session releases the previous window silently.
  window_ = std::move(window);
  window_id_ = window_ ? window_->GetId() : 0;
  anchor_ = anchor;
  last_cursor_position_ = cursor_position;
  active_ = true;
  ++generation_;

  if (window_) {
    MoveWindow(*window_, cursor_position);
  }
  StartTicking();
  return true;
}

void WindowDragSession::Cancel() {
  if (!active_) {
    return;
  }
  Point cursor_position = last_cursor_position_;
  bool primary_button_down = false;
  QueryPointer(cursor_position, primary_button_down);
  Finish(true, cursor_position);
}

bool WindowDragSession::IsActive() const {
  return active_;
}

WindowId WindowDragSession::GetWindowId() const {
  return active_ ? window_id_ : 0;
}

Point WindowDragSession::GetAnchor() const {
  return anchor_;
}

void WindowDragSession::HandleTick() {
  if (!active_) {
    StopTicking();
    return;
  }

  Point cursor_position = last_cursor_position_;
  bool primary_button_down = false;
  if (!QueryPointer(cursor_position, primary_button_down)) {
    Finish(true, cursor_position);
    return;
  }

  // The window was closed out from under the session.
  if (window_ && window_->GetNativeObject() == nullptr) {
    Finish(true, cursor_position);
    return;
  }

  const bool moved = std::fabs(cursor_position.x - last_cursor_position_.x) >= 0.5 ||
                     std::fabs(cursor_position.y - last_cursor_position_.y) >= 0.5;
  if (moved) {
    last_cursor_position_ = cursor_position;
    if (window_) {
      MoveWindow(*window_, cursor_position);
    }
    const unsigned long generation = generation_;
    Emit<WindowDragMovedEvent>(window_id_, cursor_position);
    // A listener restarted or stopped the session; the release below, if any,
    // belongs to whatever state it left behind and is picked up next tick.
    if (generation != generation_ || !active_) {
      return;
    }
  }

  if (!primary_button_down) {
    Finish(false, cursor_position);
  }
}

void WindowDragSession::Finish(bool cancelled, Point cursor_position) {
  if (!active_) {
    return;
  }
  active_ = false;
  ++generation_;
  StopTicking();

  const WindowId window_id = window_id_;
  // Keep the window alive until listeners have seen the final event.
  const std::shared_ptr<Window> window = std::move(window_);
  window_id_ = 0;
  last_cursor_position_ = cursor_position;

  if (cancelled) {
    Emit<WindowDragCancelledEvent>(window_id, cursor_position);
  } else {
    Emit<WindowDragEndedEvent>(window_id, cursor_position);
  }
}

}  // namespace nativeapi
