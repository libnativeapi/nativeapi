#include <gtk/gtk.h>
#include <iostream>
#include "../../display_manager.h"

namespace nativeapi {

static Display CreateDisplayFromGdkMonitor(GdkMonitor* monitor,
                                           bool isFirstScreen) {
  // Simply create Display with GdkMonitor - all properties will be read
  // directly from the monitor
  return Display(monitor);
}

DisplayManager::DisplayManager() {
  gtk_init(nullptr, nullptr);
  // Constructor implementation
  std::cout << "DisplayManager initialized" << std::endl;
}

DisplayManager::~DisplayManager() {
  // Destructor implementation
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
