#include <iostream>
#include "../../window.h"
#include "../../window_manager.h"

// Import Cocoa headers
#import <Cocoa/Cocoa.h>

namespace nativeapi {

// Private implementation class
class Window::Impl {
 public:
  Impl(NSWindow* window) : ns_window_(window) {}
  NSWindow* ns_window_;
};

Window::Window() : pimpl_(new Impl(nil)) {
  id = -1;
}

Window::Window(void* window) : pimpl_(new Impl((__bridge NSWindow*)window)) {
  id = pimpl_->ns_window_ ? [pimpl_->ns_window_ windowNumber] : 0;
}

Window::~Window() {
  delete pimpl_;
}

void Window::Focus() {
  [pimpl_->ns_window_ makeKeyAndOrderFront:nil];
}

void Window::Blur() {
  [pimpl_->ns_window_ orderBack:nil];
}

bool Window::IsFocused() const {
  return [pimpl_->ns_window_ isKeyWindow];
}

void Window::Show() {
  [pimpl_->ns_window_ setIsVisible:YES];
  // Panels receive key focus when shown but should not activate the app.
  if (![pimpl_->ns_window_ isKindOfClass:[NSPanel class]]) {
    [[NSApplication sharedApplication] activateIgnoringOtherApps:YES];
  }
  [pimpl_->ns_window_ makeKeyAndOrderFront:nil];
}

void Window::ShowInactive() {
  [pimpl_->ns_window_ setIsVisible:YES];
  [pimpl_->ns_window_ orderFrontRegardless];
}

void Window::Hide() {
  [pimpl_->ns_window_ setIsVisible:NO];
  [pimpl_->ns_window_ orderOut:nil];
}

bool Window::IsVisible() const {
  return [pimpl_->ns_window_ isVisible];
}

void Window::Maximize() {
  if (!IsMaximized()) {
    [pimpl_->ns_window_ zoom:nil];
  }
}

void Window::Unmaximize() {
  if (IsMaximized()) {
    [pimpl_->ns_window_ zoom:nil];
  }
}

bool Window::IsMaximized() const {
  return [pimpl_->ns_window_ isZoomed];
}

void Window::Minimize() {
  if (!IsMinimized()) {
    [pimpl_->ns_window_ miniaturize:nil];
  }
}

void Window::Restore() {
  if (IsMinimized()) {
    [pimpl_->ns_window_ deminiaturize:nil];
  }
}

bool Window::IsMinimized() const {
  return [pimpl_->ns_window_ isMiniaturized];
}

void Window::SetFullScreen(bool is_full_screen) {
  if (is_full_screen) {
    if (!IsFullScreen()) {
      [pimpl_->ns_window_ toggleFullScreen:nil];
    }
  } else {
    if (IsFullScreen()) {
      [pimpl_->ns_window_ toggleFullScreen:nil];
    }
  }
}

bool Window::IsFullScreen() const {
  return [pimpl_->ns_window_ styleMask] & NSWindowStyleMaskFullScreen;
}

//// void Window::SetBackgroundColor(Color color);
//// Color Window::GetBackgroundColor() const;

void Window::SetBounds(Rectangle bounds) {
  [pimpl_->ns_window_ setFrame:NSMakeRect(bounds.x, bounds.y, bounds.width, bounds.height)
                       display:YES];
}

Rectangle Window::GetBounds() const {
  NSRect frame = [pimpl_->ns_window_ frame];
  Rectangle bounds = {static_cast<double>(frame.origin.x), static_cast<double>(frame.origin.y),
                      static_cast<double>(frame.size.height),
                      static_cast<double>(frame.size.width)};
  return bounds;
}

void Window::SetSize(Size size, bool animate) {
  NSRect frame = [pimpl_->ns_window_ frame];
  frame.origin.y += (frame.size.height - size.height);
  frame.size.width = size.width;
  frame.size.height = size.height;
  if (animate) {
    [[pimpl_->ns_window_ animator] setFrame:frame display:YES animate:YES];
  } else {
    [pimpl_->ns_window_ setFrame:frame display:YES];
  }
}

Size Window::GetSize() const {
  NSRect frame = [pimpl_->ns_window_ frame];
  Size size = {static_cast<double>(frame.size.width), static_cast<double>(frame.size.height)};
  return size;
}

void Window::SetContentSize(Size size) {
  [pimpl_->ns_window_ setContentSize:NSMakeSize(size.width, size.height)];
}

Size Window::GetContentSize() const {
  NSRect frame = [pimpl_->ns_window_ contentRectForFrameRect:[pimpl_->ns_window_ frame]];
  Size size = {static_cast<double>(frame.size.width), static_cast<double>(frame.size.height)};
  return size;
}

void Window::SetMinimumSize(Size size) {
  [pimpl_->ns_window_ setMinSize:NSMakeSize(size.width, size.height)];
}

Size Window::GetMinimumSize() const {
  NSSize size = [pimpl_->ns_window_ minSize];
  return Size{static_cast<double>(size.width), static_cast<double>(size.height)};
}

void Window::SetMaximumSize(Size size) {
  [pimpl_->ns_window_ setMaxSize:NSMakeSize(size.width, size.height)];
}

Size Window::GetMaximumSize() const {
  NSSize size = [pimpl_->ns_window_ maxSize];
  return Size{static_cast<double>(size.width), static_cast<double>(size.height)};
}

void Window::SetResizable(bool is_resizable) {
  NSUInteger style_mask = [pimpl_->ns_window_ styleMask];
  if (is_resizable) {
    style_mask |= NSWindowStyleMaskResizable;
  } else {
    style_mask &= ~NSWindowStyleMaskResizable;
  }
  [pimpl_->ns_window_ setStyleMask:style_mask];
}

bool Window::IsResizable() const {
  return [pimpl_->ns_window_ styleMask] & NSWindowStyleMaskResizable;
}

void Window::SetMovable(bool is_movable) {
  [pimpl_->ns_window_ setMovable:is_movable];
}

bool Window::IsMovable() const {
  return [pimpl_->ns_window_ isMovable];
}

void Window::SetMinimizable(bool is_minimizable) {
  NSUInteger style_mask = [pimpl_->ns_window_ styleMask];
  if (is_minimizable) {
    style_mask |= NSWindowStyleMaskMiniaturizable;
  } else {
    style_mask &= ~NSWindowStyleMaskMiniaturizable;
  }
  [pimpl_->ns_window_ setStyleMask:style_mask];
}

bool Window::IsMinimizable() const {
  return [pimpl_->ns_window_ styleMask] & NSWindowStyleMaskMiniaturizable;
}

void Window::SetMaximizable(bool is_maximizable) {
  NSUInteger style_mask = [pimpl_->ns_window_ styleMask];
  if (is_maximizable) {
    style_mask |= NSWindowStyleMaskResizable;
  } else {
    style_mask &= ~NSWindowStyleMaskResizable;
  }
  [pimpl_->ns_window_ setStyleMask:style_mask];
}

bool Window::IsMaximizable() const {
  return [pimpl_->ns_window_ styleMask] & NSWindowStyleMaskResizable;
}

void Window::SetFullScreenable(bool is_full_screenable) {
  // TODO: Implement this
}

bool Window::IsFullScreenable() const {
  return [pimpl_->ns_window_ styleMask] & NSWindowStyleMaskFullScreen;
}

void Window::SetClosable(bool is_closable) {
  NSUInteger style_mask = [pimpl_->ns_window_ styleMask];
  if (is_closable) {
    style_mask |= NSWindowStyleMaskClosable;
  } else {
    style_mask &= ~NSWindowStyleMaskClosable;
  }
  [pimpl_->ns_window_ setStyleMask:style_mask];
}

bool Window::IsClosable() const {
  return [pimpl_->ns_window_ styleMask] & NSWindowStyleMaskClosable;
}

void Window::SetAlwaysOnTop(bool is_always_on_top) {
  [pimpl_->ns_window_ setLevel:is_always_on_top ? NSFloatingWindowLevel : NSNormalWindowLevel];
}

bool Window::IsAlwaysOnTop() const {
  return [pimpl_->ns_window_ level] == NSFloatingWindowLevel;
}

void Window::SetPosition(Point point) {
  NSRect screenFrameRect = [[NSScreen screens][0] frame];
  // Convert point to bottom left origin
  NSPoint bottomLeft =
      NSMakePoint(point.x, screenFrameRect.size.height - point.y - GetSize().height);
  [pimpl_->ns_window_ setFrameOrigin:bottomLeft];
}

Point Window::GetPosition() const {
  NSRect frame = [pimpl_->ns_window_ frame];
  Point point = {static_cast<double>(frame.origin.x), static_cast<double>(frame.origin.y)};
  return point;
}

void Window::SetTitle(std::string title) {
  [pimpl_->ns_window_ setTitle:[NSString stringWithUTF8String:title.c_str()]];
}

std::string Window::GetTitle() const {
  NSString* title = [pimpl_->ns_window_ title];
  return title ? std::string([title UTF8String]) : std::string();
}

void Window::SetHasShadow(bool has_shadow) {
  [pimpl_->ns_window_ setHasShadow:has_shadow];
  [pimpl_->ns_window_ invalidateShadow];
}

bool Window::HasShadow() const {
  return [pimpl_->ns_window_ hasShadow];
}

void Window::SetOpacity(float opacity) {
  [pimpl_->ns_window_ setAlphaValue:opacity];
}

float Window::GetOpacity() const {
  return [pimpl_->ns_window_ alphaValue];
}

void Window::SetVisibleOnAllWorkspaces(bool is_visible_on_all_workspaces) {
  [pimpl_->ns_window_ setCollectionBehavior:is_visible_on_all_workspaces
                                                ? NSWindowCollectionBehaviorCanJoinAllSpaces
                                                : NSWindowCollectionBehaviorDefault];
}

bool Window::IsVisibleOnAllWorkspaces() const {
  return [pimpl_->ns_window_ collectionBehavior] & NSWindowCollectionBehaviorCanJoinAllSpaces;
}

void Window::SetIgnoreMouseEvents(bool is_ignore_mouse_events) {
  [pimpl_->ns_window_ setIgnoresMouseEvents:is_ignore_mouse_events];
}

bool Window::IsIgnoreMouseEvents() const {
  return [pimpl_->ns_window_ ignoresMouseEvents];
}

void Window::SetFocusable(bool is_focusable) {
  // TODO: Implement this
}

bool Window::IsFocusable() const {
  return [pimpl_->ns_window_ canBecomeKeyWindow];
}

void Window::StartDragging() {
  NSWindow* window = pimpl_->ns_window_;
  if (window.currentEvent) {
    [window performWindowDragWithEvent:window.currentEvent];
  }
}

void Window::StartResizing() {}

void* Window::GetNSWindow() const {
  if (!pimpl_) {
    std::cout << "GetNSWindow: pimpl_ is null" << std::endl;
    return nullptr;
  }
  if (!pimpl_->ns_window_) {
    std::cout << "GetNSWindow: ns_window_ is null" << std::endl;
    return nullptr;
  }
  std::cout << "GetNSWindow: returning valid NSWindow pointer" << std::endl;
  return (__bridge void*)pimpl_->ns_window_;
}

}  // namespace nativeapi
