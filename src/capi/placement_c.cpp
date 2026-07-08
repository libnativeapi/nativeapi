// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

#include "placement_c.h"

#include "../placement.h"

namespace {

native_placement_t ToCPlacement(nativeapi::Placement value) {
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

}  // namespace

