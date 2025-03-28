#pragma once

namespace nativeapi {

// /**
//  * Insets are the space added to the edges of a rectangle.
//  */
// struct Insets {
//   double bottom;
//   double left;
//   double right;
//   double top;
// };

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