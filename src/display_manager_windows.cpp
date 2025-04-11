#include <codecvt>
#include <iostream>
#include "display_manager.h"

#include <shellscalingapi.h>
#include <windows.h>

namespace nativeapi {

const double kBaseDpi = 96.0;

UINT GetMonitorDpi(HMONITOR monitor) {
  // UINT dpiX = 0, dpiY = 0;
  // HRESULT hr = GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
  // if (SUCCEEDED(hr)) {
  //   return dpiX;
  // }
  return 96;
}

// Helper method to create Display struct from HMONITOR
Display CreateDisplayFromMonitor(HMONITOR monitor, bool isMainScreen) {
  MONITORINFOEX info;
  info.cbSize = sizeof(MONITORINFOEX);
  ::GetMonitorInfo(monitor, &info);
  UINT dpi = GetMonitorDpi(monitor);

  Display display;

  wchar_t display_name[sizeof(info.szDevice) / sizeof(*info.szDevice) + 1];
  memset(display_name, 0, sizeof(display_name));
  memcpy(display_name, info.szDevice, sizeof(info.szDevice));

  display.name = std::string(display_name, display_name + wcslen(display_name));

  display.scaleFactor = dpi / kBaseDpi;

  display.visibleSizeWidth =
      (info.rcWork.right - info.rcWork.left) / display.scaleFactor;
  display.visibleSizeHeight =
      (info.rcWork.bottom - info.rcWork.top) / display.scaleFactor;

  display.visiblePositionX = (info.rcWork.left) / display.scaleFactor;
  display.visiblePositionY = (info.rcWork.top) / display.scaleFactor;

  display.width =
      info.rcMonitor.right / display.scaleFactor - display.visiblePositionX;
  display.height =
      info.rcMonitor.bottom / display.scaleFactor - display.visiblePositionY;

  DISPLAY_DEVICE displayDevice;
  displayDevice.cb = sizeof(DISPLAY_DEVICE);
  int deviceIndex = 0;
  while (EnumDisplayDevices(info.szDevice, deviceIndex, &displayDevice, 0)) {
    if (displayDevice.StateFlags & DISPLAY_DEVICE_ACTIVE &&
        (displayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)) {
      // std::wstring deviceName(displayDevice.DeviceName);
      // if (deviceName.find(info.szDevice) == 0) {
      //   std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
      //   display.id = converter.to_bytes(displayDevice.DeviceID);
      // }
    }
    deviceIndex++;
  }

  return display;
}

BOOL CALLBACK MonitorRepresentationEnumProc(HMONITOR monitor,
                                            HDC hdc,
                                            LPRECT clip,
                                            LPARAM list_ref) {
  std::vector<Display>* displays =
      reinterpret_cast<std::vector<Display>*>(list_ref);
  Display display = CreateDisplayFromMonitor(monitor, false);
  displays->push_back(display);
  return TRUE;
}

std::vector<Display> DisplayManager::GetAll() {
  std::vector<Display> displayList;
  ::EnumDisplayMonitors(nullptr, nullptr, MonitorRepresentationEnumProc,
                        reinterpret_cast<LPARAM>(&displayList));
  return displayList;
}

Display DisplayManager::GetPrimary() {
  POINT ptZero = {0, 0};
  HMONITOR monitor = MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
  Display display = CreateDisplayFromMonitor(monitor, true);
  display.id = "";
  return display;
}

Point DisplayManager::GetCursorPosition() {
  POINT cursorPos;
  GetCursorPos(&cursorPos);

  Point point;
  point.x = cursorPos.x;
  point.y = cursorPos.y;
  return point;
}
}  // namespace nativeapi
