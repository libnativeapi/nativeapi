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
  NSWindow* window = [app mainWindow];
  if (window == nil) {
    std::cerr << "No main window found." << std::endl;
    return Window();
  } else {
    std::cout << "Main window found." << std::endl;
    std::cout << "Window title: " << [[window title] UTF8String] << std::endl;
  }
  return *new Window();
}

std::vector<Window> WindowManager::GetAll() {
  std::vector<Window> displayList;

  displayList.push_back(GetCurrent());

  return displayList;
}

}  // namespace nativeapi
