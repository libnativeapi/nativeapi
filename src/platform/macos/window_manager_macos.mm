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
class WindowManager::WindowManagerImpl {
public:
  WindowManagerImpl(WindowManager* manager);
  ~WindowManagerImpl();
  
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
@property (nonatomic, assign) nativeapi::WindowManager::WindowManagerImpl* impl;
- (instancetype)initWithImpl:(nativeapi::WindowManager::WindowManagerImpl*)impl;
@end

@implementation NativeAPIWindowManagerDelegate

- (instancetype)initWithImpl:(nativeapi::WindowManager::WindowManagerImpl*)impl {
  if (self = [super init]) {
    _impl = impl;
  }
  return self;
}

- (void)windowDidBecomeKey:(NSNotification*)notification {
  NSWindow* window = [notification object];
  if (_impl) {
    _impl->OnWindowEvent(window, "focused");
  }
}

- (void)windowDidResignKey:(NSNotification*)notification {
  NSWindow* window = [notification object];
  if (_impl) {
    _impl->OnWindowEvent(window, "blurred");
  }
}

- (void)windowDidMiniaturize:(NSNotification*)notification {
  NSWindow* window = [notification object];
  if (_impl) {
    _impl->OnWindowEvent(window, "minimized");
  }
}

- (void)windowDidDeminiaturize:(NSNotification*)notification {
  NSWindow* window = [notification object];
  if (_impl) {
    _impl->OnWindowEvent(window, "restored");
  }
}

- (void)windowDidResize:(NSNotification*)notification {
  NSWindow* window = [notification object];
  if (_impl) {
    _impl->OnWindowEvent(window, "resized");
  }
}

- (void)windowDidMove:(NSNotification*)notification {
  NSWindow* window = [notification object];
  if (_impl) {
    _impl->OnWindowEvent(window, "moved");
  }
}

- (void)windowWillClose:(NSNotification*)notification {
  NSWindow* window = [notification object];
  if (_impl) {
    _impl->OnWindowEvent(window, "closing");
  }
}

@end

namespace nativeapi {

WindowManager::WindowManagerImpl::WindowManagerImpl(WindowManager* manager)
    : manager_(manager), delegate_(nullptr) {
}

WindowManager::WindowManagerImpl::~WindowManagerImpl() {
  CleanupEventMonitoring();
}

void WindowManager::WindowManagerImpl::SetupEventMonitoring() {
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

void WindowManager::WindowManagerImpl::CleanupEventMonitoring() {
  if (delegate_) {
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center removeObserver:delegate_];
    delegate_ = nil;
  }
}

void WindowManager::WindowManagerImpl::OnWindowEvent(NSWindow* window, const std::string& event_type) {
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
    Size new_size = {static_cast<int>(frame.size.width), static_cast<int>(frame.size.height)};
    WindowResizedEvent event(window_id, new_size);
    manager_->DispatchWindowEvent(event);
  } else if (event_type == "moved") {
    NSRect frame = [window frame];
    Point new_position = {static_cast<int>(frame.origin.x), static_cast<int>(frame.origin.y)};
    WindowMovedEvent event(window_id, new_position);
    manager_->DispatchWindowEvent(event);
  } else if (event_type == "closing") {
    WindowClosedEvent event(window_id);
    manager_->DispatchWindowEvent(event);
  }
}

WindowManager::WindowManager() : impl_(std::make_unique<WindowManagerImpl>(this)) {
  SetupEventMonitoring();
}

WindowManager::~WindowManager() {
  CleanupEventMonitoring();
}

void WindowManager::SetupEventMonitoring() {
  impl_->SetupEventMonitoring();
}

void WindowManager::CleanupEventMonitoring() {
  impl_->CleanupEventMonitoring();
}

void WindowManager::DispatchWindowEvent(const Event& event) {
  event_dispatcher_.DispatchSync(event);
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
