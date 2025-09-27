#include "display_c.h"
#include <cstring>
#include <memory>
#include "../display.h"

using namespace nativeapi;

// Helper to cast opaque handle to C++ Display pointer
static inline Display* to_display(native_display_t handle) {
  return static_cast<Display*>(handle);
}

// Helper function to safely copy C++ string to C string
static char* copy_string(const std::string& str) {
  if (str.empty())
    return nullptr;

  size_t len = str.length() + 1;
  char* result = new (std::nothrow) char[len];
  if (result) {
    std::strcpy(result, str.c_str());
  }
  return result;
}

// Basic identification getters
FFI_PLUGIN_EXPORT
char* native_display_get_id(native_display_t display) {
  if (!display)
    return nullptr;
  return copy_string(to_display(display)->GetId());
}

FFI_PLUGIN_EXPORT
char* native_display_get_name(native_display_t display) {
  if (!display)
    return nullptr;
  return copy_string(to_display(display)->GetName());
}

// Physical properties getters
FFI_PLUGIN_EXPORT
native_point_t native_display_get_position(native_display_t display) {
  native_point_t result = {0.0, 0.0};
  if (!display)
    return result;

  auto pos = to_display(display)->GetPosition();
  result.x = pos.x;
  result.y = pos.y;
  return result;
}

FFI_PLUGIN_EXPORT
native_size_t native_display_get_size(native_display_t display) {
  native_size_t result = {0.0, 0.0};
  if (!display)
    return result;

  auto size = to_display(display)->GetSize();
  result.width = size.width;
  result.height = size.height;
  return result;
}

FFI_PLUGIN_EXPORT
native_rectangle_t native_display_get_work_area(native_display_t display) {
  native_rectangle_t result = {0.0, 0.0, 0.0, 0.0};
  if (!display)
    return result;

  Rectangle work_area = to_display(display)->GetWorkArea();
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
  return to_display(display)->GetScaleFactor();
}

// Additional properties getters
FFI_PLUGIN_EXPORT
bool native_display_is_primary(native_display_t display) {
  if (!display)
    return false;
  return to_display(display)->IsPrimary();
}

FFI_PLUGIN_EXPORT
native_display_orientation_t native_display_get_orientation(
    native_display_t display) {
  if (!display)
    return NATIVE_DISPLAY_ORIENTATION_PORTRAIT;

  DisplayOrientation orientation = to_display(display)->GetOrientation();
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
  return to_display(display)->GetRefreshRate();
}

FFI_PLUGIN_EXPORT
int native_display_get_bit_depth(native_display_t display) {
  if (!display)
    return 0;
  return to_display(display)->GetBitDepth();
}

// Hardware information getters
FFI_PLUGIN_EXPORT
char* native_display_get_manufacturer(native_display_t display) {
  if (!display)
    return nullptr;
  return copy_string(to_display(display)->GetManufacturer());
}

FFI_PLUGIN_EXPORT
char* native_display_get_model(native_display_t display) {
  if (!display)
    return nullptr;
  return copy_string(to_display(display)->GetModel());
}

FFI_PLUGIN_EXPORT
char* native_display_get_serial_number(native_display_t display) {
  if (!display)
    return nullptr;
  return copy_string(to_display(display)->GetSerialNumber());
}

// Platform-specific functions
FFI_PLUGIN_EXPORT
void* native_display_get_native_object(native_display_t display) {
  if (!display)
    return nullptr;
  return to_display(display)->GetNativeObject();
}

// Memory management
FFI_PLUGIN_EXPORT
void native_display_free_string(char* str) {
  if (str) {
    delete[] str;
  }
}

FFI_PLUGIN_EXPORT
void native_display_free(native_display_t display) {
  if (display) {
    delete to_display(display);
  }
}

FFI_PLUGIN_EXPORT
void native_display_list_free(native_display_list_t* list) {
  if (!list || !list->displays)
    return;

  // Free individual display handles
  for (long i = 0; i < list->count; i++) {
    if (list->displays[i]) {
      delete to_display(list->displays[i]);
    }
  }

  // Free the displays array
  delete[] list->displays;
  list->displays = nullptr;
  list->count = 0;
}

// Internal function for creating display handles
native_display_t native_display_create_handle(
    const nativeapi::Display& cpp_display) {
  try {
    auto* handle = new (std::nothrow) Display(cpp_display);
    return handle;
  } catch (const std::exception&) {
    return nullptr;
  }
}
