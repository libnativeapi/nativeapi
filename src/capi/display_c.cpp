// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

#include "display_c.h"

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
#include "../display.h"

namespace {

native_display_orientation_t ToCDisplayOrientation(nativeapi::DisplayOrientation value) {
  switch (value) {
    case nativeapi::DisplayOrientation::kPortrait:
      return NATIVE_DISPLAY_ORIENTATION_PORTRAIT;
    case nativeapi::DisplayOrientation::kLandscape:
      return NATIVE_DISPLAY_ORIENTATION_LANDSCAPE;
    case nativeapi::DisplayOrientation::kPortraitFlipped:
      return NATIVE_DISPLAY_ORIENTATION_PORTRAIT_FLIPPED;
    case nativeapi::DisplayOrientation::kLandscapeFlipped:
      return NATIVE_DISPLAY_ORIENTATION_LANDSCAPE_FLIPPED;
    default:
      return NATIVE_DISPLAY_ORIENTATION_PORTRAIT;
  }
}

nativeapi::DisplayOrientation ToCppDisplayOrientation(native_display_orientation_t value) {
  switch (value) {
    case NATIVE_DISPLAY_ORIENTATION_PORTRAIT:
      return nativeapi::DisplayOrientation::kPortrait;
    case NATIVE_DISPLAY_ORIENTATION_LANDSCAPE:
      return nativeapi::DisplayOrientation::kLandscape;
    case NATIVE_DISPLAY_ORIENTATION_PORTRAIT_FLIPPED:
      return nativeapi::DisplayOrientation::kPortraitFlipped;
    case NATIVE_DISPLAY_ORIENTATION_LANDSCAPE_FLIPPED:
      return nativeapi::DisplayOrientation::kLandscapeFlipped;
    default:
      return nativeapi::DisplayOrientation::kPortrait;
  }
}

native_point_t ToCPoint(const nativeapi::Point& value) {
  native_point_t result = {};
  result.x = value.x;
  result.y = value.y;
  return result;
}

nativeapi::Point ToCppPoint(const native_point_t& value) {
  nativeapi::Point result = {};
  result.x = value.x;
  result.y = value.y;
  return result;
}

native_size_t ToCSize(const nativeapi::Size& value) {
  native_size_t result = {};
  result.width = value.width;
  result.height = value.height;
  return result;
}

nativeapi::Size ToCppSize(const native_size_t& value) {
  nativeapi::Size result = {};
  result.width = value.width;
  result.height = value.height;
  return result;
}

native_rectangle_t ToCRectangle(const nativeapi::Rectangle& value) {
  native_rectangle_t result = {};
  result.x = value.x;
  result.y = value.y;
  result.width = value.width;
  result.height = value.height;
  return result;
}

nativeapi::Rectangle ToCppRectangle(const native_rectangle_t& value) {
  nativeapi::Rectangle result = {};
  result.x = value.x;
  result.y = value.y;
  result.width = value.width;
  result.height = value.height;
  return result;
}

}  // namespace

native_display_t native_display_create(void) {
  try {
    return nativeapi::HandleTable::GetInstance().Insert(
        std::make_shared<nativeapi::Display>());
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_display_create");
    return 0;
  }
}

native_display_t native_display_create_with_display(void* display) {
  try {
    return nativeapi::HandleTable::GetInstance().Insert(
        std::make_shared<nativeapi::Display>(display));
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_display_create_with_display");
    return 0;
  }
}

char* native_display_get_id(native_display_t display) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Display>(display);
  if (!self) {
    return nullptr;
  }
  try {
    return to_c_str(self->GetId());
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_display_get_id");
    return nullptr;
  }
}

char* native_display_get_name(native_display_t display) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Display>(display);
  if (!self) {
    return nullptr;
  }
  try {
    return to_c_str(self->GetName());
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_display_get_name");
    return nullptr;
  }
}

native_point_t native_display_get_position(native_display_t display) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Display>(display);
  if (!self) {
    native_point_t result = {};
    return result;
  }
  try {
    const auto cpp_result = self->GetPosition();
    return ToCPoint(cpp_result);
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_display_get_position");
    native_point_t result = {};
    return result;
  }
}

native_size_t native_display_get_size(native_display_t display) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Display>(display);
  if (!self) {
    native_size_t result = {};
    return result;
  }
  try {
    const auto cpp_result = self->GetSize();
    return ToCSize(cpp_result);
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_display_get_size");
    native_size_t result = {};
    return result;
  }
}

native_rectangle_t native_display_get_work_area(native_display_t display) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Display>(display);
  if (!self) {
    native_rectangle_t result = {};
    return result;
  }
  try {
    const auto cpp_result = self->GetWorkArea();
    return ToCRectangle(cpp_result);
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_display_get_work_area");
    native_rectangle_t result = {};
    return result;
  }
}

double native_display_get_scale_factor(native_display_t display) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Display>(display);
  if (!self) {
    return 0;
  }
  try {
    return self->GetScaleFactor();
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_display_get_scale_factor");
    return 0;
  }
}

bool native_display_is_primary(native_display_t display) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Display>(display);
  if (!self) {
    return false;
  }
  try {
    return self->IsPrimary();
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_display_is_primary");
    return false;
  }
}

native_display_orientation_t native_display_get_orientation(native_display_t display) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Display>(display);
  if (!self) {
    return (native_display_orientation_t)NATIVE_DISPLAY_ORIENTATION_PORTRAIT;
  }
  try {
    return ToCDisplayOrientation(self->GetOrientation());
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_display_get_orientation");
    return (native_display_orientation_t)NATIVE_DISPLAY_ORIENTATION_PORTRAIT;
  }
}

int native_display_get_refresh_rate(native_display_t display) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Display>(display);
  if (!self) {
    return 0;
  }
  try {
    return self->GetRefreshRate();
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_display_get_refresh_rate");
    return 0;
  }
}

int native_display_get_bit_depth(native_display_t display) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Display>(display);
  if (!self) {
    return 0;
  }
  try {
    return self->GetBitDepth();
  } catch (...) {
    fprintf(stderr, "[nativeapi] %s: unexpected exception\n", "native_display_get_bit_depth");
    return 0;
  }
}

void* native_display_get_native_object(native_display_t display) {
  auto self = nativeapi::HandleTable::GetInstance().Resolve<nativeapi::Display>(display);
  if (!self) {
    return nullptr;
  }
  return self->GetNativeObject();
}

void native_display_free(native_display_t display) {
  // The table invalidates the handle itself, so releasing an unknown or
  // already-released one is a no-op rather than a double free.
  nativeapi::HandleTable::GetInstance().Release(display);
}

void native_display_list_free(native_display_list_t* list) {
  if (!list || !list->displays) {
    return;
  }
  for (long i = 0; i < list->count; ++i) {
    nativeapi::HandleTable::GetInstance().Release(list->displays[i]);
  }
  delete[] list->displays;
  list->displays = nullptr;
  list->count = 0;
}

void native_display_list_release(native_display_list_t* list) {
  if (!list) {
    return;
  }
  delete[] list->displays;
  list->displays = nullptr;
  list->count = 0;
}

bool ToCDisplayEvent(const nativeapi::DisplayEvent& event, native_display_event_t* out) {
  if (!out) {
    return false;
  }
  *out = native_display_event_t{};
  out->display = nativeapi::HandleTable::GetInstance().Insert(
      std::make_shared<nativeapi::Display>(event.GetDisplay()));
  if (const auto* typed = dynamic_cast<const nativeapi::DisplayAddedEvent*>(&event)) {
    out->type = NATIVE_DISPLAY_EVENT_TYPE_ADDED;
    (void)typed;
    return true;
  }
  if (const auto* typed = dynamic_cast<const nativeapi::DisplayRemovedEvent*>(&event)) {
    out->type = NATIVE_DISPLAY_EVENT_TYPE_REMOVED;
    (void)typed;
    return true;
  }
  if (const auto* typed = dynamic_cast<const nativeapi::DisplayChangedEvent*>(&event)) {
    out->type = NATIVE_DISPLAY_EVENT_TYPE_CHANGED;
    out->data.changed.old_display = nativeapi::HandleTable::GetInstance().Insert(
        std::make_shared<nativeapi::Display>(typed->GetOldDisplay()));
    out->data.changed.new_display = nativeapi::HandleTable::GetInstance().Insert(
        std::make_shared<nativeapi::Display>(typed->GetNewDisplay()));
    return true;
  }
  return false;
}

void FreeCDisplayEvent(native_display_event_t* value) {
  if (!value) {
    return;
  }
  nativeapi::HandleTable::GetInstance().Release(value->display);
  value->display = 0;
  if (value->type == NATIVE_DISPLAY_EVENT_TYPE_CHANGED) {
    nativeapi::HandleTable::GetInstance().Release(value->data.changed.old_display);
    value->data.changed.old_display = 0;
    nativeapi::HandleTable::GetInstance().Release(value->data.changed.new_display);
    value->data.changed.new_display = 0;
  }
}

