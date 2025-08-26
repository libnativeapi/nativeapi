#pragma once

/**
 * Representation of a point
 */
typedef struct {
  double x;
  double y;
} native_point_t;

/**
 * Representation of a point
 */
typedef struct {
  double width;
  double height;
} native_size_t;

/**
 * Representation of a rectangle
 */
typedef struct {
  double x;
  double y;
  double width;
  double height;
} native_rectangle_t;
