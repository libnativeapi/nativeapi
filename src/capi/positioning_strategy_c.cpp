#include "positioning_strategy_c.h"
#include "../foundation/positioning_strategy.h"

using namespace nativeapi;

extern "C" {

native_positioning_strategy_t native_positioning_strategy_absolute(const native_point_t* point) {
  if (!point) {
    return nullptr;
  }

  nativeapi::Point cpp_point{point->x, point->y};
  PositioningStrategy* strategy = new PositioningStrategy(PositioningStrategy::Absolute(cpp_point));
  return static_cast<native_positioning_strategy_t>(strategy);
}

native_positioning_strategy_t native_positioning_strategy_cursor_position() {
  PositioningStrategy* strategy = new PositioningStrategy(PositioningStrategy::CursorPosition());
  return static_cast<native_positioning_strategy_t>(strategy);
}

native_positioning_strategy_t native_positioning_strategy_relative(const native_rectangle_t* rect,
                                                                   const native_point_t* offset) {
  if (!rect) {
    return nullptr;
  }

  nativeapi::Rectangle cpp_rect{rect->x, rect->y, rect->width, rect->height};
  nativeapi::Point cpp_offset{0, 0};
  if (offset) {
    cpp_offset = nativeapi::Point{offset->x, offset->y};
  }

  PositioningStrategy* strategy =
      new PositioningStrategy(PositioningStrategy::Relative(cpp_rect, cpp_offset));
  return static_cast<native_positioning_strategy_t>(strategy);
}

void native_positioning_strategy_free(native_positioning_strategy_t strategy) {
  if (strategy) {
    delete static_cast<PositioningStrategy*>(strategy);
  }
}

}  // extern "C"
