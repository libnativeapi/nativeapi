// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

#include "positioning_strategy_c.h"

#include <cstdio>
#include <memory>
#include <new>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "string_utils_c.h"
#include "../foundation/handle_table.h"
#include "../foundation/geometry.h"
#include "geometry_c.h"
#include "../window.h"
#include "window_c.h"
#include "../positioning_strategy.h"

// Conversion helpers between the C ABI types and their C++ originals.

inline native_positioning_strategy_type_t ToCPositioningStrategyType(nativeapi::PositioningStrategy::Type value) {
  switch (value) {
    case nativeapi::PositioningStrategy::Type::Absolute:
      return NATIVE_POSITIONING_STRATEGY_TYPE_ABSOLUTE;
    case nativeapi::PositioningStrategy::Type::CursorPosition:
      return NATIVE_POSITIONING_STRATEGY_TYPE_CURSOR_POSITION;
    case nativeapi::PositioningStrategy::Type::Relative:
      return NATIVE_POSITIONING_STRATEGY_TYPE_RELATIVE;
    default:
      return NATIVE_POSITIONING_STRATEGY_TYPE_ABSOLUTE;
  }
}

inline nativeapi::PositioningStrategy::Type ToCppPositioningStrategyType(native_positioning_strategy_type_t value) {
  switch (value) {
    case NATIVE_POSITIONING_STRATEGY_TYPE_ABSOLUTE:
      return nativeapi::PositioningStrategy::Type::Absolute;
    case NATIVE_POSITIONING_STRATEGY_TYPE_CURSOR_POSITION:
      return nativeapi::PositioningStrategy::Type::CursorPosition;
    case NATIVE_POSITIONING_STRATEGY_TYPE_RELATIVE:
      return nativeapi::PositioningStrategy::Type::Relative;
    default:
      return nativeapi::PositioningStrategy::Type::Absolute;
  }
}

inline native_point_t ToCPoint(const nativeapi::Point& value) {
  native_point_t result = {};
  result.x = value.x;
  result.y = value.y;
  return result;
}

inline nativeapi::Point ToCppPoint(const native_point_t& value) {
  nativeapi::Point result = {};
  result.x = value.x;
  result.y = value.y;
  return result;
}

inline native_rectangle_t ToCRectangle(const nativeapi::Rectangle& value) {
  native_rectangle_t result = {};
  result.x = value.x;
  result.y = value.y;
  result.width = value.width;
  result.height = value.height;
  return result;
}

inline nativeapi::Rectangle ToCppRectangle(const native_rectangle_t& value) {
  nativeapi::Rectangle result = {};
  result.x = value.x;
  result.y = value.y;
  result.width = value.width;
  result.height = value.height;
  return result;
}

native_positioning_strategy_t native_positioning_strategy_absolute(native_point_t point) {
  try {
    auto point_cpp = ToCppPoint(point);
    return nativeapi::HandleTable::GetInstance().Insert(
        std::make_shared<nativeapi::PositioningStrategy>(nativeapi::PositioningStrategy::Absolute(point_cpp)));
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_positioning_strategy_absolute");
    return 0;
  }
}

native_positioning_strategy_t native_positioning_strategy_cursor_position(void) {
  try {
    return nativeapi::HandleTable::GetInstance().Insert(
        std::make_shared<nativeapi::PositioningStrategy>(nativeapi::PositioningStrategy::CursorPosition()));
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_positioning_strategy_cursor_position");
    return 0;
  }
}

native_positioning_strategy_t native_positioning_strategy_relative_with_rect_and_offset(native_rectangle_t rect, native_point_t offset) {
  try {
    auto rect_cpp = ToCppRectangle(rect);
    auto offset_cpp = ToCppPoint(offset);
    return nativeapi::HandleTable::GetInstance().Insert(
        std::make_shared<nativeapi::PositioningStrategy>(nativeapi::PositioningStrategy::Relative(rect_cpp, offset_cpp)));
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_positioning_strategy_relative_with_rect_and_offset");
    return 0;
  }
}

native_positioning_strategy_t native_positioning_strategy_relative_with_window_and_offset(native_window_t window, native_point_t offset) {
  try {
    auto window_cpp = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Window>(window);
    if (!window_cpp) {
      return 0;
    }
    auto offset_cpp = ToCppPoint(offset);
    return nativeapi::HandleTable::GetInstance().Insert(
        std::make_shared<nativeapi::PositioningStrategy>(nativeapi::PositioningStrategy::Relative(*window_cpp, offset_cpp)));
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_positioning_strategy_relative_with_window_and_offset");
    return 0;
  }
}

native_positioning_strategy_type_t native_positioning_strategy_get_type(native_positioning_strategy_t positioning_strategy) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::PositioningStrategy>(positioning_strategy);
  if (!self) {
    return (native_positioning_strategy_type_t)NATIVE_POSITIONING_STRATEGY_TYPE_ABSOLUTE;
  }
  try {
    return ToCPositioningStrategyType(self->GetType());
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_positioning_strategy_get_type");
    return (native_positioning_strategy_type_t)NATIVE_POSITIONING_STRATEGY_TYPE_ABSOLUTE;
  }
}

native_point_t native_positioning_strategy_get_absolute_position(native_positioning_strategy_t positioning_strategy) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::PositioningStrategy>(positioning_strategy);
  if (!self) {
    native_point_t result = {};
    return result;
  }
  try {
    const auto cpp_result = self->GetAbsolutePosition();
    return ToCPoint(cpp_result);
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_positioning_strategy_get_absolute_position");
    native_point_t result = {};
    return result;
  }
}

native_rectangle_t native_positioning_strategy_get_relative_rectangle(native_positioning_strategy_t positioning_strategy) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::PositioningStrategy>(positioning_strategy);
  if (!self) {
    native_rectangle_t result = {};
    return result;
  }
  try {
    const auto cpp_result = self->GetRelativeRectangle();
    return ToCRectangle(cpp_result);
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_positioning_strategy_get_relative_rectangle");
    native_rectangle_t result = {};
    return result;
  }
}

native_point_t native_positioning_strategy_get_relative_offset(native_positioning_strategy_t positioning_strategy) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::PositioningStrategy>(positioning_strategy);
  if (!self) {
    native_point_t result = {};
    return result;
  }
  try {
    const auto cpp_result = self->GetRelativeOffset();
    return ToCPoint(cpp_result);
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_positioning_strategy_get_relative_offset");
    native_point_t result = {};
    return result;
  }
}

void native_positioning_strategy_free(native_positioning_strategy_t positioning_strategy) {
  // The table invalidates the handle itself, so releasing an unknown or
  // already-released one is a no-op rather than a double free.
  nativeapi::HandleTable::GetInstance().Release(positioning_strategy);
}

