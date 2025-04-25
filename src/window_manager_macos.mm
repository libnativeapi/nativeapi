#include <cstring>
#include <iostream>
#include <string>

#include "window.h"
#include "window_manager.h"

// Import Cocoa headers
#import <Cocoa/Cocoa.h>

namespace nativeapi {

WindowManager::WindowManager() {}

WindowManager::~WindowManager() {}

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
