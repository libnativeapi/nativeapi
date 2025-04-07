#include "window.h"
#include "window_manager.h"

// Import Cocoa headers
#import <Cocoa/Cocoa.h>

namespace nativeapi {

Window::Window() {
  id = "window1";
  name = "Window 1";
}

Window::~Window() {}

}  // namespace nativeapi
