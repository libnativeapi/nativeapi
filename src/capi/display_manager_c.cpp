#include "display_manager_c.h"
#include <iostream>
#include <memory>
#include <vector>
#include "../display.h"
#include "../display_manager.h"
#include "display_c.h"

using namespace nativeapi;

DisplayManager& g_display_manager = DisplayManager::GetInstance();

FFI_PLUGIN_EXPORT
native_display_list_t native_display_manager_get_all() {
  native_display_list_t list = {};

  try {
    auto displays = g_display_manager.GetAll();

    list.count = static_cast<long>(displays.size());
    if (list.count > 0) {
      // Allocate array for display handles
      list.displays = new (std::nothrow) native_display_t[list.count];
      if (list.displays) {
        for (size_t i = 0; i < displays.size(); i++) {
          try {
            list.displays[i] = new (std::nothrow) Display(displays[i]);
            if (!list.displays[i]) {
              // If creation fails, clean up and return empty list
              for (size_t j = 0; j < i; j++) {
                native_display_free(list.displays[j]);
              }
              delete[] list.displays;
              list.displays = nullptr;
              list.count = 0;
              break;
            }
          } catch (const std::exception&) {
            // If creation fails, clean up and return empty list
            for (size_t j = 0; j < i; j++) {
              native_display_free(list.displays[j]);
            }
            delete[] list.displays;
            list.displays = nullptr;
            list.count = 0;
            break;
          }
        }
      } else {
        list.count = 0;
      }
    } else {
      list.displays = nullptr;
    }

    return list;
  } catch (const std::exception& e) {
    std::cerr << "Error in native_display_manager_get_all: " << e.what() << std::endl;
    list.count = 0;
    list.displays = nullptr;
    return list;
  }
}

FFI_PLUGIN_EXPORT
native_display_t native_display_manager_get_primary() {
  try {
    auto primary_display = g_display_manager.GetPrimary();
    auto* handle = new (std::nothrow) Display(primary_display);
    return handle;
  } catch (const std::exception& e) {
    std::cerr << "Error in native_display_manager_get_primary: " << e.what() << std::endl;
    return nullptr;
  }
}

FFI_PLUGIN_EXPORT
native_point_t native_display_manager_get_cursor_position() {
  native_point_t point = {0.0, 0.0};

  try {
    auto cursor_position = g_display_manager.GetCursorPosition();
    point.x = cursor_position.x;
    point.y = cursor_position.y;
    return point;
  } catch (const std::exception& e) {
    std::cerr << "Error in native_display_manager_get_cursor_position: " << e.what() << std::endl;
    return point;
  }
}
