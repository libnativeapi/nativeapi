#include "positioning_strategy.h"

namespace nativeapi {

PositioningStrategy::PositioningStrategy(Type type)
    : type_(type), absolute_position_{0, 0}, relative_rect_{0, 0, 0, 0}, relative_offset_{0, 0} {}

PositioningStrategy PositioningStrategy::Absolute(double x, double y) {
  PositioningStrategy strategy(Type::Absolute);
  strategy.absolute_position_ = {x, y};
  return strategy;
}

PositioningStrategy PositioningStrategy::CursorPosition() {
  return PositioningStrategy(Type::CursorPosition);
}

PositioningStrategy PositioningStrategy::Relative(const Rectangle& rect, const Point& offset) {
  PositioningStrategy strategy(Type::Relative);
  strategy.relative_rect_ = rect;
  strategy.relative_offset_ = offset;
  return strategy;
}

}  // namespace nativeapi

