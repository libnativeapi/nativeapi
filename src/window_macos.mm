#include "window.h"
#include "window_manager.h"
#include <iostream>

// Import Cocoa headers
#import <Cocoa/Cocoa.h>

namespace nativeapi {

// Private implementation class
class Window::Impl {
 public:
  Impl(NSWindow* window) : ns_window_(window) {}
  NSWindow* ns_window_;
};

Window::Window() : pimpl_(nullptr) {
  id = "window1";
  std::cout << "Window created with null pimpl_" << std::endl;
}

Window::Window(void* window) : pimpl_(new Impl((NSWindow*)window)) {
  id = "window1";
  std::cout << "Window created with NSWindow: " << pimpl_->ns_window_ << std::endl;
}

Window::~Window() {
  std::cout << "Window destroyed, pimpl_: " << pimpl_ << std::endl;
  delete pimpl_;
}

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
  return pimpl_->ns_window_;
}

Size Window::GetSize() const {
  Size size = {0, 0};
  if (!pimpl_) {
    std::cout << "GetSize: pimpl_ is null" << std::endl;
    return size;
  }
  if (!pimpl_->ns_window_) {
    std::cout << "GetSize: ns_window_ is null" << std::endl;
    return size;
  }
  NSRect frame = [pimpl_->ns_window_ frame];
  size.width = static_cast<int>(frame.size.width);
  size.height = static_cast<int>(frame.size.height);
  std::cout << "GetSize: window size is " << size.width << "x" << size.height << std::endl;
  return size;
}

}  // namespace nativeapi
