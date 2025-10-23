#pragma once
#include "geometry.h"

namespace nativeapi {

/**
 * @brief Strategy for determining where to position UI elements.
 *
 * PositioningStrategy defines how to calculate the position for UI elements
 * such as menus, tooltips, or popovers. It supports various positioning modes:
 * - Absolute: Fixed screen coordinates
 * - CursorPosition: Current mouse cursor position
 * - Relative: Position relative to a rectangle
 *
 * @example
 * ```cpp
 * // Position menu at absolute screen coordinates
 * menu->Open(PositioningStrategy::Absolute(100, 200), Placement::Bottom);
 *
 * // Position menu at current mouse location
 * menu->Open(PositioningStrategy::CursorPosition(), Placement::BottomStart);
 *
 * // Position menu relative to a rectangle
 * Rectangle buttonRect = button->GetBounds();
 * menu->Open(PositioningStrategy::Relative(buttonRect, {0, 10}), Placement::Bottom);
 * ```
 */
class PositioningStrategy {
 public:
  /**
   * @brief Type of positioning strategy.
   */
  enum class Type {
    /**
     * Position at fixed screen coordinates.
     */
    Absolute,

    /**
     * Position at current mouse cursor location.
     */
    CursorPosition,

    /**
     * Position relative to a rectangle.
     */
    Relative
  };

  /**
   * @brief Create a strategy for absolute positioning at fixed coordinates.
   *
   * @param x X-coordinate in screen space
   * @param y Y-coordinate in screen space
   * @return PositioningStrategy configured for absolute positioning
   *
   * @example
   * ```cpp
   * auto strategy = PositioningStrategy::Absolute(100, 200);
   * menu->Open(strategy, Placement::Bottom);
   * ```
   */
  static PositioningStrategy Absolute(double x, double y);

  /**
   * @brief Create a strategy for positioning at current mouse location.
   *
   * @return PositioningStrategy configured to use mouse cursor position
   *
   * @example
   * ```cpp
   * auto strategy = PositioningStrategy::CursorPosition();
   * contextMenu->Open(strategy, Placement::BottomStart);
   * ```
   */
  static PositioningStrategy CursorPosition();

  /**
   * @brief Create a strategy for positioning relative to a rectangle.
   *
   * @param rect Rectangle in screen coordinates to position relative to
   * @param offset Optional offset point to apply to the position (default: {0, 0})
   * @return PositioningStrategy configured for rectangle-relative positioning
   *
   * @example
   * ```cpp
   * Rectangle buttonRect = button->GetBounds();
   * // Position at bottom of button
   * auto strategy = PositioningStrategy::Relative(buttonRect);
   * menu->Open(strategy, Placement::Bottom);
   *
   * // Position at bottom of button with 10px offset
   * auto strategy2 = PositioningStrategy::Relative(buttonRect, {0, 10});
   * menu->Open(strategy2, Placement::Bottom);
   * ```
   */
  static PositioningStrategy Relative(const Rectangle& rect, const Point& offset = {0, 0});

  /**
   * @brief Get the type of this positioning strategy.
   *
   * @return The Type enum value indicating the strategy type
   */
  Type GetType() const { return type_; }

  /**
   * @brief Get the absolute position (for Absolute type).
   *
   * @return Point containing x,y coordinates
   * @note Only valid when GetType() == Type::Absolute
   */
  Point GetAbsolutePosition() const { return absolute_position_; }

  /**
   * @brief Get the relative rectangle (for Relative type).
   *
   * @return Rectangle in screen coordinates
   * @note Only valid when GetType() == Type::Relative
   */
  Rectangle GetRelativeRectangle() const { return relative_rect_; }

  /**
   * @brief Get the relative offset point (for Relative type).
   *
   * @return Point containing x,y relative offset
   * @note Only valid when GetType() == Type::Relative
   */
  Point GetRelativeOffset() const { return relative_offset_; }

 private:
  /**
   * @brief Private constructor - use static factory methods instead.
   */
  PositioningStrategy(Type type);

  Type type_;
  Point absolute_position_;
  Rectangle relative_rect_;
  Point relative_offset_;
};

}  // namespace nativeapi
