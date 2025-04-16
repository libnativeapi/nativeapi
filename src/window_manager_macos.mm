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

Window WindowManager::Create() {
  return *new Window();
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
  return *new Window(ns_window);
}

std::vector<Window> WindowManager::GetAll() {
  std::vector<Window> windowList;
  NSApplication* app = [NSApplication sharedApplication];
  NSArray* windows = [app windows];
  for (NSWindow* ns_window in windows) {
    if ([ns_window isVisible]) {
      NSRect frame = [ns_window frame];
        std::cout << "Window title: " << [[ns_window title] UTF8String] << std::endl;
        std::cout << "Window size: " << frame.size.width << "x" << frame.size.height
                    << std::endl;
      windowList.push_back(*new Window(ns_window));
    }
  }
  return windowList;
}

}  // namespace nativeapi
