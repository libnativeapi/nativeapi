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

/**
 * Placement options for positioning UI elements relative to an anchor.
 */
typedef enum {
  /**
   * Position above the anchor, horizontally centered.
   */
  NATIVE_PLACEMENT_TOP,

  /**
   * Position above the anchor, aligned to the start (left).
   */
  NATIVE_PLACEMENT_TOP_START,

  /**
   * Position above the anchor, aligned to the end (right).
   */
  NATIVE_PLACEMENT_TOP_END,

  /**
   * Position to the right of the anchor, vertically centered.
   */
  NATIVE_PLACEMENT_RIGHT,

  /**
   * Position to the right of the anchor, aligned to the start (top).
   */
  NATIVE_PLACEMENT_RIGHT_START,

  /**
   * Position to the right of the anchor, aligned to the end (bottom).
   */
  NATIVE_PLACEMENT_RIGHT_END,

  /**
   * Position below the anchor, horizontally centered.
   */
  NATIVE_PLACEMENT_BOTTOM,

  /**
   * Position below the anchor, aligned to the start (left).
   */
  NATIVE_PLACEMENT_BOTTOM_START,

  /**
   * Position below the anchor, aligned to the end (right).
   */
  NATIVE_PLACEMENT_BOTTOM_END,

  /**
   * Position to the left of the anchor, vertically centered.
   */
  NATIVE_PLACEMENT_LEFT,

  /**
   * Position to the left of the anchor, aligned to the start (top).
   */
  NATIVE_PLACEMENT_LEFT_START,

  /**
   * Position to the left of the anchor, aligned to the end (bottom).
   */
  NATIVE_PLACEMENT_LEFT_END
} native_placement_t;
