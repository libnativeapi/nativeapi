#include <string.h>
#include <iostream>
#include <vector>

#include "../display.h"
#include "../display_manager.h"

#include "display_c.h"
#include "display_manager_c.h"
#include "geometry_c.h"

using namespace nativeapi;

native_display_orientation_t to_native_orientation(
    DisplayOrientation orientation) {
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

native_display_t to_native_display(const Display& raw_display) {
  native_display_t display = {};

  // Allocate and copy strings
  display.id = strdup(raw_display.id.c_str());
  display.name = strdup(raw_display.name.c_str());
  display.manufacturer = raw_display.manufacturer.empty()
                             ? nullptr
                             : strdup(raw_display.manufacturer.c_str());
  display.model =
      raw_display.model.empty() ? nullptr : strdup(raw_display.model.c_str());
  display.serial_number = nullptr;  // Not available in the C++ API

  // Copy position
  display.position.x = raw_display.position.x;
  display.position.y = raw_display.position.y;

  // Copy size
  display.size.width = raw_display.size.width;
  display.size.height = raw_display.size.height;

  // Copy work area
  display.work_area.x = raw_display.workArea.x;
  display.work_area.y = raw_display.workArea.y;
  display.work_area.width = raw_display.workArea.width;
  display.work_area.height = raw_display.workArea.height;

  // Copy other properties
  display.scale_factor = raw_display.scaleFactor;
  display.is_primary = raw_display.isPrimary;
  display.orientation = to_native_orientation(raw_display.orientation);
  display.refresh_rate = raw_display.refreshRate;
  display.bit_depth = raw_display.bitDepth;

  return display;
}

DisplayManager g_display_manager = DisplayManager();

FFI_PLUGIN_EXPORT
native_display_list_t native_display_manager_get_all() {
  try {
    auto displays = g_display_manager.GetAll();
    native_display_list_t list = {};

    list.count = displays.size();
    if (list.count > 0) {
      // Allocate array for displays
      list.displays =
          (native_display_t*)malloc(sizeof(native_display_t) * list.count);
      if (list.displays != nullptr) {
        for (size_t i = 0; i < displays.size(); i++) {
          list.displays[i] = to_native_display(displays[i]);
        }
      } else {
        list.count = 0;
      }
    } else {
      list.displays = nullptr;
    }

    return list;
  } catch (const std::exception& e) {
    std::cerr << "Error in native_display_manager_get_all: " << e.what()
              << std::endl;
    native_display_list_t empty_list = {};
    empty_list.count = 0;
    empty_list.displays = nullptr;
    return empty_list;
  }
}

FFI_PLUGIN_EXPORT
native_display_t native_display_manager_get_primary() {
  try {
    auto display = g_display_manager.GetPrimary();
    return to_native_display(display);
  } catch (const std::exception& e) {
    std::cerr << "Error in native_display_manager_get_primary: " << e.what()
              << std::endl;
    native_display_t empty_display = {};
    return empty_display;
  }
}

FFI_PLUGIN_EXPORT
native_point_t native_display_manager_get_cursor_position() {
  try {
    auto cursor_position = g_display_manager.GetCursorPosition();
    native_point_t point;
    point.x = cursor_position.x;
    point.y = cursor_position.y;
    return point;
  } catch (const std::exception& e) {
    std::cerr << "Error in native_display_manager_get_cursor_position: "
              << e.what() << std::endl;
    native_point_t point = {0.0, 0.0};
    return point;
  }
}

FFI_PLUGIN_EXPORT
void native_display_list_free(native_display_list_t* list) {
  if (list != nullptr && list->displays != nullptr) {
    // Free individual display strings
    for (size_t i = 0; i < list->count; i++) {
      native_display_t* display = &list->displays[i];
      if (display->id != nullptr) {
        free(display->id);
        display->id = nullptr;
      }
      if (display->name != nullptr) {
        free(display->name);
        display->name = nullptr;
      }
      if (display->manufacturer != nullptr) {
        free(display->manufacturer);
        display->manufacturer = nullptr;
      }
      if (display->model != nullptr) {
        free(display->model);
        display->model = nullptr;
      }
      if (display->serial_number != nullptr) {
        free(display->serial_number);
        display->serial_number = nullptr;
      }
    }

    // Free the displays array
    free(list->displays);
    list->displays = nullptr;
    list->count = 0;
  }
}
