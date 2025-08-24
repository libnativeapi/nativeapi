#include <gtk/gtk.h>
#include <iostream>
#include "../../display_manager.h"

namespace nativeapi {

static Display CreateDisplayFromGdkMonitor(GdkMonitor* monitor,
                                           bool isFirstScreen) {
  Display display;

  display.id = "";
  display.name = gdk_monitor_get_model(monitor);

  GdkRectangle frame;
  gdk_monitor_get_geometry(monitor, &frame);

  display.width = frame.width;
  display.height = frame.height;

  GdkRectangle workarea_rect;
  gdk_monitor_get_workarea(monitor, &workarea_rect);

  display.visibleSizeWidth = workarea_rect.width;
  display.visibleSizeHeight = workarea_rect.height;
  display.visiblePositionX = workarea_rect.x;
  display.visiblePositionY = workarea_rect.y;

  display.scaleFactor = gdk_monitor_get_scale_factor(monitor);

  return display;
}

DisplayManager::DisplayManager() {
  gtk_init(nullptr, nullptr);
  // Constructor implementation
  std::cout << "DisplayManager initialized" << std::endl;
}

std::vector<Display> DisplayManager::GetAll() {
  // Empty implementation
  std::vector<Display> displayList;
  displayList.push_back(GetPrimary());
  return displayList;
}

Display DisplayManager::GetPrimary() {
  GdkDisplay* display = gdk_display_get_default();
  GdkMonitor* monitor = gdk_display_get_primary_monitor(display);

  // opt: fallback if there's no primary monitor
  if (monitor == nullptr) {
    int monitor_count = gdk_display_get_n_monitors(display);
    if (monitor_count > 0) {
      monitor = gdk_display_get_monitor(display, 0);
    }
  }

  return CreateDisplayFromGdkMonitor(monitor, true);
}

Point DisplayManager::GetCursorPosition() {
  GdkDisplay* display = gdk_display_get_default();
  GdkSeat* seat = gdk_display_get_default_seat(display);
  GdkDevice* pointer = gdk_seat_get_pointer(seat);

  int x, y;
  gdk_device_get_position(pointer, NULL, &x, &y);

  // Empty implementation
  Point point;
  point.x = x;
  point.y = y;
  return point;
}

}  // namespace nativeapi
