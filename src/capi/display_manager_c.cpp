#include <string.h>
#include <iostream>

#include "../display.h"
#include "../display_manager.h"

#include "display_c.h"
#include "display_manager_c.h"
#include "geometry_c.h"

using namespace nativeapi;

native_display_t to_native_display(const Display& raw_display) {
  native_display_t display;
  display.id = strdup(raw_display.id.c_str());
  display.name = strdup(raw_display.name.c_str());
  display.size.width = raw_display.size.width;
  display.size.height = raw_display.size.height;
  display.work_area.x = raw_display.workArea.x;
  display.work_area.y = raw_display.workArea.y;
  display.work_area.width = raw_display.workArea.width;
  display.work_area.height = raw_display.workArea.height;
  display.scale_factor = raw_display.scaleFactor;
  return display;
}

DisplayManager g_display_manager = DisplayManager();

FFI_PLUGIN_EXPORT
native_display_list_t native_display_manager_get_all() {
  auto displays = g_display_manager.GetAll();
  native_display_list_t list;
  list.count = displays.size();
  for (size_t i = 0; i < displays.size(); i++) {
    list.displays[i] = to_native_display(displays[i]);
  }
  return list;
}

FFI_PLUGIN_EXPORT
native_display_t native_display_manager_get_primary() {
  auto display = g_display_manager.GetPrimary();
  return to_native_display(display);
}

FFI_PLUGIN_EXPORT
native_point_t native_display_manager_get_cursor_position() {
  auto cursor_position = g_display_manager.GetCursorPosition();
  native_point_t point;
  point.x = cursor_position.x;
  point.y = cursor_position.y;
  return point;
}
