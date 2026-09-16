#include "../../window_drag_session.h"

#include "coordinate_utils_macos.h"

#import <Cocoa/Cocoa.h>

namespace nativeapi {

// Ticks come from two sources: a local event monitor, so the window follows
// mouse-dragged events without lag, and a timer, which keeps the session
// honest when the events are delivered somewhere the monitor cannot see.
class WindowDragSession::Impl {
 public:
  id event_monitor_ = nil;
  NSTimer* timer_ = nil;
  // Set while handling the mouse-up event itself: +[NSEvent pressedMouseButtons]
  // may still report the button as down at that point.
  bool release_pending_ = false;
};

WindowDragSession::WindowDragSession() : pimpl_(std::make_unique<Impl>()) {}

WindowDragSession::~WindowDragSession() {
  active_ = false;
  StopTicking();
}

void WindowDragSession::StartTicking() {
  pimpl_->release_pending_ = false;
  if (pimpl_->event_monitor_ == nil) {
    WindowDragSession* session = this;
    NSEventMask mask = NSEventMaskLeftMouseDragged | NSEventMaskLeftMouseUp | NSEventMaskMouseMoved;
    pimpl_->event_monitor_ =
        [NSEvent addLocalMonitorForEventsMatchingMask:mask
                                              handler:^NSEvent*(NSEvent* event) {
                                                if (event.type == NSEventTypeLeftMouseUp) {
                                                  session->pimpl_->release_pending_ = true;
                                                }
                                                session->HandleTick();
                                                return event;
                                              }];
  }
  if (pimpl_->timer_ == nil) {
    WindowDragSession* session = this;
    pimpl_->timer_ = [NSTimer timerWithTimeInterval:1.0 / 120.0
                                            repeats:YES
                                              block:^(NSTimer* timer) {
                                                session->HandleTick();
                                              }];
    // Common modes keep the timer firing during event tracking loops.
    [[NSRunLoop mainRunLoop] addTimer:pimpl_->timer_ forMode:NSRunLoopCommonModes];
  }
}

void WindowDragSession::StopTicking() {
  if (pimpl_->event_monitor_ != nil) {
    [NSEvent removeMonitor:pimpl_->event_monitor_];
    pimpl_->event_monitor_ = nil;
  }
  if (pimpl_->timer_ != nil) {
    [pimpl_->timer_ invalidate];
    pimpl_->timer_ = nil;
  }
  pimpl_->release_pending_ = false;
}

bool WindowDragSession::QueryPointer(Point& position, bool& primary_button_down) const {
  if ([NSScreen screens].count == 0) {
    return false;
  }
  CGPoint top_left = NSPointExt::topLeft([NSEvent mouseLocation]);
  position = {top_left.x, top_left.y};
  primary_button_down =
      !pimpl_->release_pending_ && ([NSEvent pressedMouseButtons] & (1 << 0)) != 0;
  return true;
}

void WindowDragSession::MoveWindow(Window& window, Point cursor_position) const {
  window.SetPosition({cursor_position.x - anchor_.x, cursor_position.y - anchor_.y});
}

}  // namespace nativeapi
