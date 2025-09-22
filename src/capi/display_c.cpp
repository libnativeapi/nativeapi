#include "display_c.h"
#include <cstring>
#include <memory>
#include "../display.h"

using namespace nativeapi;

// Internal structure to hold the actual Display pointer
struct native_display_handle {
  std::shared_ptr<Display> display;
  explicit native_display_handle(std::shared_ptr<Display> d)
      : display(std::move(d)) {}
};

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
  if (!display || !display->display)
    return nullptr;
  return copy_string(display->display->GetId());
}

FFI_PLUGIN_EXPORT
char* native_display_get_name(native_display_t display) {
  if (!display || !display->display)
    return nullptr;
  return copy_string(display->display->GetName());
}

// Physical properties getters
FFI_PLUGIN_EXPORT
native_point_t native_display_get_position(native_display_t display) {
  native_point_t result = {0.0, 0.0};
  if (!display || !display->display)
    return result;

  Point pos = display->display->GetPosition();
  result.x = pos.x;
  result.y = pos.y;
  return result;
}

FFI_PLUGIN_EXPORT
native_size_t native_display_get_size(native_display_t display) {
  native_size_t result = {0.0, 0.0};
  if (!display || !display->display)
    return result;

  Size size = display->display->GetSize();
  result.width = size.width;
  result.height = size.height;
  return result;
}

FFI_PLUGIN_EXPORT
native_rectangle_t native_display_get_work_area(native_display_t display) {
  native_rectangle_t result = {0.0, 0.0, 0.0, 0.0};
  if (!display || !display->display)
    return result;

  Rectangle work_area = display->display->GetWorkArea();
  result.x = work_area.x;
  result.y = work_area.y;
  result.width = work_area.width;
  result.height = work_area.height;
  return result;
}

FFI_PLUGIN_EXPORT
double native_display_get_scale_factor(native_display_t display) {
  if (!display || !display->display)
    return 1.0;
  return display->display->GetScaleFactor();
}

// Additional properties getters
FFI_PLUGIN_EXPORT
bool native_display_is_primary(native_display_t display) {
  if (!display || !display->display)
    return false;
  return display->display->IsPrimary();
}

FFI_PLUGIN_EXPORT
native_display_orientation_t native_display_get_orientation(
    native_display_t display) {
  if (!display || !display->display)
    return NATIVE_DISPLAY_ORIENTATION_PORTRAIT;

  DisplayOrientation orientation = display->display->GetOrientation();
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
  if (!display || !display->display)
    return 0;
  return display->display->GetRefreshRate();
}

FFI_PLUGIN_EXPORT
int native_display_get_bit_depth(native_display_t display) {
  if (!display || !display->display)
    return 0;
  return display->display->GetBitDepth();
}

// Hardware information getters
FFI_PLUGIN_EXPORT
char* native_display_get_manufacturer(native_display_t display) {
  if (!display || !display->display)
    return nullptr;
  return copy_string(display->display->GetManufacturer());
}

FFI_PLUGIN_EXPORT
char* native_display_get_model(native_display_t display) {
  if (!display || !display->display)
    return nullptr;
  return copy_string(display->display->GetModel());
}

FFI_PLUGIN_EXPORT
char* native_display_get_serial_number(native_display_t display) {
  if (!display || !display->display)
    return nullptr;
  return copy_string(display->display->GetSerialNumber());
}

// Platform-specific functions
FFI_PLUGIN_EXPORT
void* native_display_get_native_object(native_display_t display) {
  if (!display || !display->display)
    return nullptr;
  return display->display->GetNativeObject();
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
    delete display;
  }
}

FFI_PLUGIN_EXPORT
void native_display_list_free(native_display_list_t* list) {
  if (!list || !list->displays)
    return;

  // Free individual display handles
  for (long i = 0; i < list->count; i++) {
    if (list->displays[i]) {
      delete list->displays[i];
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
    // Create a shared_ptr from the Display object (copy constructor)
    auto display_ptr = std::make_shared<Display>(cpp_display);

    // Create the native handle
    auto* handle = new (std::nothrow) native_display_handle(display_ptr);
    return handle;
  } catch (const std::exception&) {
    return nullptr;
  }
}
