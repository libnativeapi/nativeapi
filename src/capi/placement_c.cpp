// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

#include "placement_c.h"

#include <cstdio>
#include <memory>
#include <new>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "string_utils_c.h"
#include "../foundation/handle_table.h"
#include "../placement.h"

// Conversion helpers between the C ABI types and their C++ originals.

inline native_placement_t ToCPlacement(nativeapi::Placement value) {
  switch (value) {
    case nativeapi::Placement::Top:
      return NATIVE_PLACEMENT_TOP;
    case nativeapi::Placement::TopStart:
      return NATIVE_PLACEMENT_TOP_START;
    case nativeapi::Placement::TopEnd:
      return NATIVE_PLACEMENT_TOP_END;
    case nativeapi::Placement::Right:
      return NATIVE_PLACEMENT_RIGHT;
    case nativeapi::Placement::RightStart:
      return NATIVE_PLACEMENT_RIGHT_START;
    case nativeapi::Placement::RightEnd:
      return NATIVE_PLACEMENT_RIGHT_END;
    case nativeapi::Placement::Bottom:
      return NATIVE_PLACEMENT_BOTTOM;
    case nativeapi::Placement::BottomStart:
      return NATIVE_PLACEMENT_BOTTOM_START;
    case nativeapi::Placement::BottomEnd:
      return NATIVE_PLACEMENT_BOTTOM_END;
    case nativeapi::Placement::Left:
      return NATIVE_PLACEMENT_LEFT;
    case nativeapi::Placement::LeftStart:
      return NATIVE_PLACEMENT_LEFT_START;
    case nativeapi::Placement::LeftEnd:
      return NATIVE_PLACEMENT_LEFT_END;
    default:
      return NATIVE_PLACEMENT_TOP;
  }
}

inline nativeapi::Placement ToCppPlacement(native_placement_t value) {
  switch (value) {
    case NATIVE_PLACEMENT_TOP:
      return nativeapi::Placement::Top;
    case NATIVE_PLACEMENT_TOP_START:
      return nativeapi::Placement::TopStart;
    case NATIVE_PLACEMENT_TOP_END:
      return nativeapi::Placement::TopEnd;
    case NATIVE_PLACEMENT_RIGHT:
      return nativeapi::Placement::Right;
    case NATIVE_PLACEMENT_RIGHT_START:
      return nativeapi::Placement::RightStart;
    case NATIVE_PLACEMENT_RIGHT_END:
      return nativeapi::Placement::RightEnd;
    case NATIVE_PLACEMENT_BOTTOM:
      return nativeapi::Placement::Bottom;
    case NATIVE_PLACEMENT_BOTTOM_START:
      return nativeapi::Placement::BottomStart;
    case NATIVE_PLACEMENT_BOTTOM_END:
      return nativeapi::Placement::BottomEnd;
    case NATIVE_PLACEMENT_LEFT:
      return nativeapi::Placement::Left;
    case NATIVE_PLACEMENT_LEFT_START:
      return nativeapi::Placement::LeftStart;
    case NATIVE_PLACEMENT_LEFT_END:
      return nativeapi::Placement::LeftEnd;
    default:
      return nativeapi::Placement::Top;
  }
}

