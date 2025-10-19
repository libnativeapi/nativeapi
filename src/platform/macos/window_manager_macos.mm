#import <Cocoa/Cocoa.h>
#include <cstring>
#include <iostream>
#include <string>

#include "../../window.h"
#include "../../window_manager.h"

// Forward declaration for the delegate
@class NativeAPIWindowManagerDelegate;

namespace nativeapi {

// Private implementation to hide Objective-C details
class WindowManager::Impl {
 public:
  Impl(WindowManager* manager);
  ~Impl();
  void SetupEventMonitoring();
  void CleanupEventMonitoring();
  void OnWindowEvent(NSWindow* window, const std::string& event_type);

 private:
  WindowManager* manager_;
  NativeAPIWindowManagerDelegate* delegate_;
};

}  // namespace nativeapi

// Objective-C delegate class to handle NSWindow notifications
@interface NativeAPIWindowManagerDelegate : NSObject
@property(nonatomic, assign) void* impl;  // Use void* instead of private class
- (instancetype)initWithImpl:(void*)impl;
@end

@implementation NativeAPIWindowManagerDelegate

- (instancetype)initWithImpl:(void*)impl {
  if (self = [super init]) {
    _impl = impl;
  }
  return self;
}

- (void)windowDidBecomeKey:(NSNotification*)notification {
  // NSWindow* window = [notification object];
  if (_impl) {
    //    static_cast<nativeapi::WindowManager::Impl*>(_impl)->OnWindowEvent(window, "focused");
  }
}

- (void)windowDidResignKey:(NSNotification*)notification {
  // NSWindow* window = [notification object];
  if (_impl) {
    //    static_cast<nativeapi::WindowManager::Impl*>(_impl)->OnWindowEvent(window, "blurred");
  }
}

- (void)windowDidMiniaturize:(NSNotification*)notification {
  // NSWindow* window = [notification object];
  if (_impl) {
    //    static_cast<nativeapi::WindowManager::Impl*>(_impl)->OnWindowEvent(window, "minimized");
  }
}

- (void)windowDidDeminiaturize:(NSNotification*)notification {
  // NSWindow* window = [notification object];
  if (_impl) {
    //    static_cast<nativeapi::WindowManager::Impl*>(_impl)->OnWindowEvent(window, "restored");
  }
}

- (void)windowDidResize:(NSNotification*)notification {
  // NSWindow* window = [notification object];
  if (_impl) {
    //    static_cast<nativeapi::WindowManager::Impl*>(_impl)->OnWindowEvent(window, "resized");
  }
}

- (void)windowDidMove:(NSNotification*)notification {
  // NSWindow* window = [notification object];
  if (_impl) {
    //    static_cast<nativeapi::WindowManager::Impl*>(_impl)->OnWindowEvent(window, "moved");
  }
}

- (void)windowWillClose:(NSNotification*)notification {
  // NSWindow* window = [notification object];
  if (_impl) {
    //    static_cast<nativeapi::WindowManager::Impl*>(_impl)->OnWindowEvent(window, "closing");
  }
}

@end

namespace nativeapi {

WindowManager::Impl::Impl(WindowManager* manager) : manager_(manager), delegate_(nullptr) {}

WindowManager::Impl::~Impl() {
  CleanupEventMonitoring();
}

void WindowManager::Impl::SetupEventMonitoring() {
  if (!delegate_) {
    delegate_ = [[NativeAPIWindowManagerDelegate alloc] initWithImpl:this];

    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center addObserver:delegate_
               selector:@selector(windowDidBecomeKey:)
                   name:NSWindowDidBecomeKeyNotification
                 object:nil];
    [center addObserver:delegate_
               selector:@selector(windowDidResignKey:)
                   name:NSWindowDidResignKeyNotification
                 object:nil];
    [center addObserver:delegate_
               selector:@selector(windowDidMiniaturize:)
                   name:NSWindowDidMiniaturizeNotification
                 object:nil];
    [center addObserver:delegate_
               selector:@selector(windowDidDeminiaturize:)
                   name:NSWindowDidDeminiaturizeNotification
                 object:nil];
    [center addObserver:delegate_
               selector:@selector(windowDidResize:)
                   name:NSWindowDidResizeNotification
                 object:nil];
    [center addObserver:delegate_
               selector:@selector(windowDidMove:)
                   name:NSWindowDidMoveNotification
                 object:nil];
    [center addObserver:delegate_
               selector:@selector(windowWillClose:)
                   name:NSWindowWillCloseNotification
                 object:nil];
  }
}

void WindowManager::Impl::CleanupEventMonitoring() {
  if (delegate_) {
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center removeObserver:delegate_];
    delegate_ = nil;
  }
}

void WindowManager::Impl::OnWindowEvent(NSWindow* window, const std::string& event_type) {
  WindowID window_id = [window windowNumber];

  if (event_type == "focused") {
    WindowFocusedEvent event(window_id);
    manager_->DispatchWindowEvent(event);
  } else if (event_type == "blurred") {
    WindowBlurredEvent event(window_id);
    manager_->DispatchWindowEvent(event);
  } else if (event_type == "minimized") {
    WindowMinimizedEvent event(window_id);
    manager_->DispatchWindowEvent(event);
  } else if (event_type == "restored") {
    WindowRestoredEvent event(window_id);
    manager_->DispatchWindowEvent(event);
  } else if (event_type == "resized") {
    NSRect frame = [window frame];
    Size new_size = {frame.size.width, frame.size.height};
    WindowResizedEvent event(window_id, new_size);
    manager_->DispatchWindowEvent(event);
  } else if (event_type == "moved") {
    NSRect frame = [window frame];
    Point new_position = {frame.origin.x, frame.origin.y};
    WindowMovedEvent event(window_id, new_position);
    manager_->DispatchWindowEvent(event);
  } else if (event_type == "closing") {
    WindowClosedEvent event(window_id);
    manager_->DispatchWindowEvent(event);
  }
}

WindowManager::WindowManager() : pimpl_(std::make_unique<Impl>(this)) {
  SetupEventMonitoring();
}

WindowManager::~WindowManager() {
  CleanupEventMonitoring();
}

void WindowManager::SetupEventMonitoring() {
  pimpl_->SetupEventMonitoring();
}

void WindowManager::CleanupEventMonitoring() {
  pimpl_->CleanupEventMonitoring();
}

void WindowManager::DispatchWindowEvent(const WindowEvent& event) {
  Emit(event);  // Use Dispatch instead of DispatchSync
}

// Create a new window with the given options.
std::shared_ptr<Window> WindowManager::Create(const WindowOptions& options) {
  NSRect frame = NSMakeRect(100, 100, options.size.width, options.size.height);
  NSUInteger style =
      NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable;
  NSWindow* ns_window = [[NSWindow alloc] initWithContentRect:frame
                                                    styleMask:style
                                                      backing:NSBackingStoreBuffered
                                                        defer:NO];
  [ns_window setTitle:[NSString stringWithUTF8String:options.title.c_str()]];
  [ns_window center];
  [ns_window makeKeyAndOrderFront:nil];
  [ns_window makeMainWindow];
  WindowID window_id = [ns_window windowNumber];
  auto window = std::make_shared<Window>((__bridge void*)ns_window);
  windows_[window_id] = window;

  // Dispatch window created event
  WindowCreatedEvent created_event(window_id);
  DispatchWindowEvent(created_event);

  return window;
}

// Destroy a window by its ID. Returns true if window was destroyed.
bool WindowManager::Destroy(WindowID id) {
  auto it = windows_.find(id);
  if (it != windows_.end()) {
    // Get the NSWindow to close it
    NSArray* ns_windows = [[NSApplication sharedApplication] windows];
    for (NSWindow* ns_window in ns_windows) {
      if ([ns_window windowNumber] == id) {
        [ns_window close];
        windows_.erase(it);
        return true;
      }
    }
    // Remove from our map even if we couldn't find the NSWindow
    windows_.erase(it);
  }
  return false;
}

std::shared_ptr<Window> WindowManager::Get(WindowID id) {
  auto it = windows_.find(id);
  if (it != windows_.end()) {
    return it->second;
  }
  NSArray* ns_windows = [[NSApplication sharedApplication] windows];
  for (NSWindow* ns_window in ns_windows) {
    if ([ns_window windowNumber] == id) {
      auto window = std::make_shared<Window>((__bridge void*)ns_window);
      windows_[id] = window;
      return window;
    }
  }
  return nullptr;
}

std::vector<std::shared_ptr<Window>> WindowManager::GetAll() {
  std::vector<std::shared_ptr<Window>> windows;
  NSArray* ns_windows = [[NSApplication sharedApplication] windows];
  for (NSWindow* ns_window in ns_windows) {
    WindowID window_id = [ns_window windowNumber];
    auto it = windows_.find(window_id);
    if (it == windows_.end()) {
      auto window = std::make_shared<Window>((__bridge void*)ns_window);
      windows_[window_id] = window;
    }
  }
  for (auto& window : windows_) {
    windows.push_back(window.second);
  }
  return windows;
}

std::shared_ptr<Window> WindowManager::GetCurrent() {
  NSApplication* app = [NSApplication sharedApplication];
  NSArray* ns_windows = [[NSApplication sharedApplication] windows];
  NSWindow* ns_window = [app mainWindow];
  if (ns_window == nil) {
    ns_window = [ns_windows objectAtIndex:0];
  }
  if (ns_window != nil) {
    return Get([ns_window windowNumber]);
  }
  return nullptr;
}

}  // namespace nativeapi
