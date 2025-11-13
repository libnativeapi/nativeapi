#import <Cocoa/Cocoa.h>
#import <objc/runtime.h>
#include <cstring>
#include <iostream>
#include <string>

#include "../../window.h"
#include "../../window_manager.h"
#include "../../window_registry.h"

// Forward declaration for the delegate
@class NativeAPIWindowManagerDelegate;

namespace nativeapi {

// Private implementation to hide Objective-C details
class WindowManager::Impl {
 public:
  Impl(WindowManager* manager);
  ~Impl();
  void StartEventListening();
  void StopEventListening();
  void OnWindowEvent(NSWindow* window, const std::string& event_type);

 private:
  WindowManager* manager_;
  NativeAPIWindowManagerDelegate* delegate_;

  // Optional pre-show/hide hooks
  std::optional<WindowManager::WindowWillShowHook> will_show_hook_;
  std::optional<WindowManager::WindowWillHideHook> will_hide_hook_;

  friend class WindowManager;
};

}  // namespace nativeapi

// MARK: - NSWindow Swizzling

// Swizzled implementations call into WindowManager hooks, then forward to original implementations
@interface NSWindow (NativeAPISwizzle)
- (void)na_swizzled_makeKeyAndOrderFront:(id)sender;
- (void)na_swizzled_orderOut:(id)sender;
@end

@implementation NSWindow (NativeAPISwizzle)

- (void)na_swizzled_makeKeyAndOrderFront:(id)sender {
    // Ensure registry is in sync and then invoke hook with correct WindowId
    auto windows = nativeapi::WindowManager::GetInstance().GetAll();
    for (const auto& window : windows) {
      if (window->GetNativeObject() == (__bridge void*)self) {
        nativeapi::WindowManager::GetInstance().InvokeWillShowHook(window->GetId());
        break;
      }
    }
    // First call original implementation so properties like title are up-to-date
    [self na_swizzled_makeKeyAndOrderFront:sender];
}

- (void)na_swizzled_orderOut:(id)sender {
  // Invoke hook before hiding, resolving id via registry without using windowNumber
  auto windows = nativeapi::WindowManager::GetInstance().GetAll();
  for (const auto& window : windows) {
    if (window->GetNativeObject() == (__bridge void*)self) {
      nativeapi::WindowManager::GetInstance().InvokeWillHideHook(window->GetId());
      break;
    }
  }
  // Call original implementation (swapped)
  [self na_swizzled_orderOut:sender];
}

@end

static void NativeAPIInstallNSWindowWillShowSwizzleOnce() {
  static dispatch_once_t onceTokenShow;
  dispatch_once(&onceTokenShow, ^{
    Class cls = [NSWindow class];
    SEL originalSel = @selector(makeKeyAndOrderFront:);
    SEL swizzledSel = @selector(na_swizzled_makeKeyAndOrderFront:);
    Method original = class_getInstanceMethod(cls, originalSel);
    Method swizzled = class_getInstanceMethod(cls, swizzledSel);
    if (original && swizzled) {
      method_exchangeImplementations(original, swizzled);
    }
  });
}

static void NativeAPIInstallNSWindowWillHideSwizzleOnce() {
  static dispatch_once_t onceTokenHide;
  dispatch_once(&onceTokenHide, ^{
    Class cls = [NSWindow class];
    SEL originalSel = @selector(orderOut:);
    SEL swizzledSel = @selector(na_swizzled_orderOut:);
    Method original = class_getInstanceMethod(cls, originalSel);
    Method swizzled = class_getInstanceMethod(cls, swizzledSel);
    if (original && swizzled) {
      method_exchangeImplementations(original, swizzled);
    }
  });
}

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
  StopEventListening();
}

void WindowManager::Impl::StartEventListening() {
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

void WindowManager::Impl::StopEventListening() {
  if (delegate_) {
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center removeObserver:delegate_];
    delegate_ = nil;
  }
}

void WindowManager::Impl::OnWindowEvent(NSWindow* window, const std::string& event_type) {
  WindowId window_id = [window windowNumber];

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
    // Window closing event - no longer emitted
  }
}

WindowManager::WindowManager() : pimpl_(std::make_unique<Impl>(this)) {
  StartEventListening();
}

WindowManager::~WindowManager() {
  StopEventListening();
}

// Destroy a window by its ID. Returns true if window was destroyed.
bool WindowManager::Destroy(WindowId id) {
  auto window = WindowRegistry::GetInstance().Get(id);
  if (!window) {
    return false;
  }
  NSArray* ns_windows = [[NSApplication sharedApplication] windows];
  for (NSWindow* ns_window in ns_windows) {
    if ([ns_window windowNumber] == id) {
      [ns_window close];
      WindowRegistry::GetInstance().Remove(id);
      return true;
    }
  }
  WindowRegistry::GetInstance().Remove(id);
  return false;
}

std::shared_ptr<Window> WindowManager::Get(WindowId id) {
  // First check if it's already in the registry
  auto window = WindowRegistry::GetInstance().Get(id);
  if (window) {
    return window;
  }

  // If not found, ensure all NSWindows are registered and try again
  GetAll();
  return WindowRegistry::GetInstance().Get(id);
}

std::vector<std::shared_ptr<Window>> WindowManager::GetAll() {
  NSArray* ns_windows = [[NSApplication sharedApplication] windows];

  // First, ensure all NSWindows are registered
  for (NSWindow* ns_window in ns_windows) {
    // Create or get Window wrapper - this will handle ID assignment via associated object
    auto window = std::make_shared<Window>((__bridge void*)ns_window);
    WindowId window_id = window->GetId();
    
    // Add to registry if not already present
    if (!WindowRegistry::GetInstance().Get(window_id)) {
      WindowRegistry::GetInstance().Add(window_id, window);
    }
  }

  // Then return all windows from registry (which now includes all NSWindows)
  return WindowRegistry::GetInstance().GetAll();
}

std::shared_ptr<Window> WindowManager::GetCurrent() {
  NSApplication* app = [NSApplication sharedApplication];
  NSArray* ns_windows = [[NSApplication sharedApplication] windows];
  NSWindow* ns_window = [app mainWindow];
  if (ns_window == nil && [ns_windows count] > 0) {
    ns_window = [ns_windows objectAtIndex:0];
  }
  if (ns_window != nil) {
    // Create or get Window wrapper - this will handle ID retrieval via associated object
    auto window = std::make_shared<Window>((__bridge void*)ns_window);
    WindowId window_id = window->GetId();
    
    // Ensure it's in the registry
    if (!WindowRegistry::GetInstance().Get(window_id)) {
      WindowRegistry::GetInstance().Add(window_id, window);
    }
    
    return window;
  }
  return nullptr;
}

void WindowManager::SetWillShowHook(std::optional<WindowWillShowHook> hook) {
  pimpl_->will_show_hook_ = std::move(hook);
  if (pimpl_->will_show_hook_) {
    NativeAPIInstallNSWindowWillShowSwizzleOnce();
  }
}

void WindowManager::SetWillHideHook(std::optional<WindowWillHideHook> hook) {
  pimpl_->will_hide_hook_ = std::move(hook);
  if (pimpl_->will_hide_hook_) {
    NativeAPIInstallNSWindowWillHideSwizzleOnce();
  }
}

void WindowManager::InvokeWillShowHook(WindowId id) {
  if (pimpl_->will_show_hook_) {
    (*pimpl_->will_show_hook_)(id);
  }
}

void WindowManager::InvokeWillHideHook(WindowId id) {
  if (pimpl_->will_hide_hook_) {
    (*pimpl_->will_hide_hook_)(id);
  }
}

void WindowManager::StartEventListening() {
  pimpl_->StartEventListening();
}

void WindowManager::StopEventListening() {
  pimpl_->StopEventListening();
}

void WindowManager::DispatchWindowEvent(const WindowEvent& event) {
  Emit(event);  // Use Dispatch instead of DispatchSync
}

}  // namespace nativeapi
