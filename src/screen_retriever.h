#pragma once

#include <memory>
#include <vector>

#include "ui/display.h"
#include "ui/geometry.h"

namespace nativeapi {
// Abstract base class for ScreenRetriever
class ScreenRetriever {
 public:
  virtual ~ScreenRetriever() = default;

  // Static factory method to create platform-specific instance
  static std::unique_ptr<ScreenRetriever> Create();

  // Get the current cursor screen point
  virtual Point GetCursorScreenPoint() = 0;

  // Get the primary display information
  virtual Display GetPrimaryDisplay() = 0;

  // Get all displays information
  virtual DisplayList GetAllDisplays() = 0;
};

}  // namespace nativeapi