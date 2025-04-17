#pragma once
#include <string>
#include "geometry.h"

namespace nativeapi {

typedef int32_t WindowID;

class Window {
 public:
  Window();
  Window(void* window);
  virtual ~Window();

  WindowID id;

  void* GetNSWindow() const;
  Size GetSize() const;

 private:
  class Impl;
  Impl* pimpl_;
};

}  // namespace nativeapi