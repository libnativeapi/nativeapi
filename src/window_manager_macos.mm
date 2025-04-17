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

Window WindowManager::Get(WindowID id) {
  NSArray* ns_windows = [[NSApplication sharedApplication] windows];
  for (NSWindow* ns_window in ns_windows) {
    if ([ns_window windowNumber] == id) {
      std::cout << "Found window with ID: " << id << std::endl;
      return Window((__bridge void*)ns_window);
    }
  }
  return nullptr;
}

std::vector<Window> WindowManager::GetAll() {
  std::vector<Window> windows;
  NSArray* ns_windows = [[NSApplication sharedApplication] windows];
  for (NSWindow* ns_window in ns_windows) {
    windows.push_back(Window((__bridge void*)ns_window));
  }
  return windows;
}

Window WindowManager::GetCurrent() {
  NSApplication* app = [NSApplication sharedApplication];
  NSWindow* ns_window = [app mainWindow];
  if (ns_window == nil) {
    std::cerr << "No main window found." << std::endl;
    return Window();
  } else {
    std::cout << "Main window found." << std::endl;
    std::cout << "Window title: " << [[ns_window title] UTF8String] << std::endl;
  }
  return Window((__bridge void*)ns_window);
}

}  // namespace nativeapi
