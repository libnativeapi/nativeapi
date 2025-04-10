#pragma once

namespace nativeapi {

/**
 * Point is a 2D point in the coordinate system.
 */
struct Point {
  double x;
  double y;
};

/**
 * Size is a 2D size in the coordinate system.
 */
struct Size {
  double height;
  double width;
};

}  // namespace nativeapi