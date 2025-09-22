#include <windows.h>
#include <vector>
#include <string>
#include <set>

#include "../../display.h"
#include "../../display_manager.h"
#include "../../display_event.h"

namespace nativeapi {

static Display CreateDisplayFromMonitorInfo(HMONITOR hMonitor, MONITORINFOEX* monitorInfo, bool isPrimary) {
  // Simply create Display with HMONITOR - all properties will be read directly from the monitor
  return Display(hMonitor);
}

static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
  auto* displays = reinterpret_cast<std::vector<Display>*>(dwData);
  
  MONITORINFOEX monitorInfo;
  monitorInfo.cbSize = sizeof(MONITORINFOEX);
  
  if (GetMonitorInfo(hMonitor, &monitorInfo)) {
    bool isPrimary = (monitorInfo.dwFlags & MONITORINFOF_PRIMARY) != 0;
    displays->push_back(CreateDisplayFromMonitorInfo(hMonitor, &monitorInfo, isPrimary));
  }
  
  return TRUE;
}

DisplayManager::DisplayManager() {
  displays_ = GetAll();
  // TODO: Set up display configuration change monitoring
  // On Windows, you would typically register for WM_DISPLAYCHANGE messages
}

DisplayManager::~DisplayManager() {
  // TODO: Clean up display change monitoring
}

std::vector<Display> DisplayManager::GetAll() {
  std::vector<Display> displays;
  EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, reinterpret_cast<LPARAM>(&displays));
  return displays;
}

Display DisplayManager::GetPrimary() {
  std::vector<Display> displays = GetAll();
  for (const auto& display : displays) {
    if (display.isPrimary) {
      return display;
    }
  }
  // If no primary display found, return the first one
  if (!displays.empty()) {
    return displays[0];
  }
  // Return empty display if no displays found
  return Display{};
}

Point DisplayManager::GetCursorPosition() {
  POINT cursorPos;
  if (GetCursorPos(&cursorPos)) {
    return {static_cast<double>(cursorPos.x), static_cast<double>(cursorPos.y)};
  }
  return {0.0, 0.0};
}

}  // namespace nativeapi
