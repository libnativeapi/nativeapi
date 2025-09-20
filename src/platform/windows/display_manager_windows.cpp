#include <windows.h>
#include <vector>
#include <string>
#include <set>

#include "../../display.h"
#include "../../display_manager.h"
#include "../../display_event.h"

namespace nativeapi {

static Display CreateDisplayFromMonitorInfo(HMONITOR hMonitor, MONITORINFOEX* monitorInfo, bool isPrimary) {
  Display display;
  
  // Set unique identifier using monitor handle
  display.id = std::to_string(reinterpret_cast<uintptr_t>(hMonitor));
  
  // Set display name from device name
  std::string deviceName = monitorInfo->szDevice;
  display.name = deviceName;
  
  // Get monitor rectangle
  RECT rect = monitorInfo->rcMonitor;
  RECT workRect = monitorInfo->rcWork;
  
  // Set position and size
  display.position = {static_cast<double>(rect.left), static_cast<double>(rect.top)};
  display.size = {static_cast<double>(rect.right - rect.left), static_cast<double>(rect.bottom - rect.top)};
  display.workArea = {static_cast<double>(workRect.left), static_cast<double>(workRect.top), 
                      static_cast<double>(workRect.bottom - workRect.top), static_cast<double>(workRect.right - workRect.left)};
  
  // Get scale factor
  HDC hdc = GetDC(nullptr);
  if (hdc) {
    int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
    display.scaleFactor = dpiX / 96.0; // 96 DPI is 100% scale
    ReleaseDC(nullptr, hdc);
  } else {
    display.scaleFactor = 1.0;
  }
  
  display.isPrimary = isPrimary;
  
  // Determine orientation
  if (display.size.width > display.size.height) {
    display.orientation = DisplayOrientation::kLandscape;
  } else {
    display.orientation = DisplayOrientation::kPortrait;
  }
  
  // Set default values
  display.refreshRate = 60; // Default refresh rate
  display.bitDepth = 32;    // Default bit depth
  display.manufacturer = "Unknown";
  display.model = display.name;
  display.serialNumber = "";
  
  return display;
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
