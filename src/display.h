#pragma once
#include <string>

namespace nativeapi {

// Representation of a display
struct Display {
  std::string id;
  std::string name;
  double width;
  double height;
  double visiblePositionX;
  double visiblePositionY;
  double visibleSizeWidth;
  double visibleSizeHeight;
  double scaleFactor;
};

}  // namespace nativeapi