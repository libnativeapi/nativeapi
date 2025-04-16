#pragma once
#include <string>
#include "geometry.h"

namespace nativeapi {

class Window {
 public:
  Window();
  Window(void* window);  // Constructor that takes NSWindow*
  virtual ~Window();

  std::string id;
  std::string name;

  void* GetNSWindow() const;  // Returns NSWindow* on macOS
  Size GetSize() const;  // Get window size

 private:
  class Impl;
  Impl* pimpl_;  // Pointer to implementation
};

}  // namespace nativeapi