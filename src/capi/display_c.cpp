#include "display_c.h"
#include <cstring>
#include <memory>
#include "../display.h"
#include "string_utils_c.h"

using namespace nativeapi;

// Basic identification getters
FFI_PLUGIN_EXPORT
char* native_display_get_id(native_display_t display) {
  if (!display)
    return nullptr;
  return to_c_str(static_cast<Display*>(display)->GetId());
}

FFI_PLUGIN_EXPORT
char* native_display_get_name(native_display_t display) {
  if (!display)
    return nullptr;
  return to_c_str(static_cast<Display*>(display)->GetName());
}

// Physical properties getters
FFI_PLUGIN_EXPORT
native_point_t native_display_get_position(native_display_t display) {
  native_point_t result = {0.0, 0.0};
  if (!display)
    return result;

  auto pos = static_cast<Display*>(display)->GetPosition();
  result.x = pos.x;
  result.y = pos.y;
  return result;
}

FFI_PLUGIN_EXPORT
native_size_t native_display_get_size(native_display_t display) {
  native_size_t result = {0.0, 0.0};
  if (!display)
    return result;

  auto size = static_cast<Display*>(display)->GetSize();
  result.width = size.width;
  result.height = size.height;
  return result;
}

FFI_PLUGIN_EXPORT
native_rectangle_t native_display_get_work_area(native_display_t display) {
  native_rectangle_t result = {0.0, 0.0, 0.0, 0.0};
  if (!display)
    return result;

  Rectangle work_area = static_cast<Display*>(display)->GetWorkArea();
  result.x = work_area.x;
  result.y = work_area.y;
  result.width = work_area.width;
  result.height = work_area.height;
  return result;
}

FFI_PLUGIN_EXPORT
double native_display_get_scale_factor(native_display_t display) {
  if (!display)
    return 1.0;
  return static_cast<Display*>(display)->GetScaleFactor();
}

// Additional properties getters
FFI_PLUGIN_EXPORT
bool native_display_is_primary(native_display_t display) {
  if (!display)
    return false;
  return static_cast<Display*>(display)->IsPrimary();
}

FFI_PLUGIN_EXPORT
native_display_orientation_t native_display_get_orientation(
    native_display_t display) {
  if (!display)
    return NATIVE_DISPLAY_ORIENTATION_PORTRAIT;

  DisplayOrientation orientation =
      static_cast<Display*>(display)->GetOrientation();
  switch (orientation) {
    case DisplayOrientation::kPortrait:
      return NATIVE_DISPLAY_ORIENTATION_PORTRAIT;
    case DisplayOrientation::kLandscape:
      return NATIVE_DISPLAY_ORIENTATION_LANDSCAPE;
    case DisplayOrientation::kPortraitFlipped:
      return NATIVE_DISPLAY_ORIENTATION_PORTRAIT_FLIPPED;
    case DisplayOrientation::kLandscapeFlipped:
      return NATIVE_DISPLAY_ORIENTATION_LANDSCAPE_FLIPPED;
    default:
      return NATIVE_DISPLAY_ORIENTATION_PORTRAIT;
  }
}

FFI_PLUGIN_EXPORT
int native_display_get_refresh_rate(native_display_t display) {
  if (!display)
    return 0;
  return static_cast<Display*>(display)->GetRefreshRate();
}

FFI_PLUGIN_EXPORT
int native_display_get_bit_depth(native_display_t display) {
  if (!display)
    return 0;
  return static_cast<Display*>(display)->GetBitDepth();
}

// Platform-specific functions
FFI_PLUGIN_EXPORT
void* native_display_get_native_object(native_display_t display) {
  if (!display)
    return nullptr;
  return static_cast<Display*>(display)->GetNativeObject();
}

// Memory management
FFI_PLUGIN_EXPORT
void native_display_free(native_display_t display) {
  if (display) {
    delete static_cast<Display*>(display);
  }
}

FFI_PLUGIN_EXPORT
void native_display_list_free(native_display_list_t* list) {
  if (!list || !list->displays)
    return;

  // Free individual display handles
  for (long i = 0; i < list->count; i++) {
    if (list->displays[i]) {
      delete static_cast<Display*>(list->displays[i]);
    }
  }

  // Free the displays array
  delete[] list->displays;
  list->displays = nullptr;
  list->count = 0;
}
