#include <windows.h>
#include <vector>
#include <string>
#include <iostream>
#include "../../display_manager.h"

namespace nativeapi {

const double kBaseDpi = 96.0;

// Helper function to get DPI for a monitor
UINT GetMonitorDpi(HMONITOR monitor) {
  typedef HRESULT(WINAPI* GetDpiForMonitorFunc)(HMONITOR, int, UINT*, UINT*);
  
  // Try to load GetDpiForMonitor from Shcore.dll (Windows 8.1+)
  HMODULE shcore = LoadLibrary(L"Shcore.dll");
  if (shcore) {
    GetDpiForMonitorFunc getDpiForMonitor = 
        (GetDpiForMonitorFunc)GetProcAddress(shcore, "GetDpiForMonitor");
    if (getDpiForMonitor) {
      UINT dpiX = 0, dpiY = 0;
      HRESULT hr = getDpiForMonitor(monitor, 0, &dpiX, &dpiY); // MDT_EFFECTIVE_DPI = 0
      FreeLibrary(shcore);
      if (SUCCEEDED(hr)) {
        return dpiX;
      }
    }
    FreeLibrary(shcore);
  }
  
  // Fallback to system DPI
  HDC hdc = GetDC(nullptr);
  UINT dpi = GetDeviceCaps(hdc, LOGPIXELSX);
  ReleaseDC(nullptr, hdc);
  return dpi;
}

// Helper function to convert wide string to string
std::string WideStringToString(const std::wstring& wstr) {
  if (wstr.empty()) return std::string();
  
  int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), nullptr, 0, nullptr, nullptr);
  std::string strTo(size_needed, 0);
  WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, nullptr, nullptr);
  return strTo;
}

// Helper method to create Display struct from HMONITOR
Display CreateDisplayFromMonitor(HMONITOR monitor, bool isPrimary) {
  MONITORINFOEX info;
  info.cbSize = sizeof(MONITORINFOEX);
  GetMonitorInfo(monitor, &info);
  
  UINT dpi = GetMonitorDpi(monitor);
  
  Display display;
  display.name = WideStringToString(std::wstring(info.szDevice));
  display.scaleFactor = static_cast<double>(dpi) / kBaseDpi;
  display.isPrimary = isPrimary;
  
  // Calculate positions and sizes (convert from physical pixels to logical pixels)
  display.position.x = static_cast<double>(info.rcMonitor.left) / display.scaleFactor;
  display.position.y = static_cast<double>(info.rcMonitor.top) / display.scaleFactor;
  display.size.width = static_cast<double>(info.rcMonitor.right - info.rcMonitor.left) / display.scaleFactor;
  display.size.height = static_cast<double>(info.rcMonitor.bottom - info.rcMonitor.top) / display.scaleFactor;
  
  // Work area (area excluding taskbars, etc.)
  display.workArea.x = static_cast<double>(info.rcWork.left) / display.scaleFactor;
  display.workArea.y = static_cast<double>(info.rcWork.top) / display.scaleFactor;
  display.workArea.width = static_cast<double>(info.rcWork.right - info.rcWork.left) / display.scaleFactor;
  display.workArea.height = static_cast<double>(info.rcWork.bottom - info.rcWork.top) / display.scaleFactor;
  
  // Try to get additional display information
  DISPLAY_DEVICE displayDevice;
  displayDevice.cb = sizeof(DISPLAY_DEVICE);
  if (EnumDisplayDevices(info.szDevice, 0, &displayDevice, 0)) {
    display.id = WideStringToString(std::wstring(displayDevice.DeviceID));
    
    // Get display settings
    DEVMODE devMode;
    devMode.dmSize = sizeof(DEVMODE);
    if (EnumDisplaySettings(info.szDevice, ENUM_CURRENT_SETTINGS, &devMode)) {
      display.refreshRate = devMode.dmDisplayFrequency;
      display.bitDepth = devMode.dmBitsPerPel;
    }
  }
  
  return display;
}

// Callback for EnumDisplayMonitors
BOOL CALLBACK MonitorEnumProc(HMONITOR monitor, HDC hdc, LPRECT clip, LPARAM listParam) {
  std::vector<Display>* displays = reinterpret_cast<std::vector<Display>*>(listParam);
  
  // Check if this is the primary monitor
  POINT pt = {0, 0};
  HMONITOR primaryMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
  bool isPrimary = (monitor == primaryMonitor);
  
  Display display = CreateDisplayFromMonitor(monitor, isPrimary);
  displays->push_back(display);
  
  return TRUE;
}

std::vector<Display> DisplayManager::GetAll() {
  std::vector<Display> displays;
  EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, reinterpret_cast<LPARAM>(&displays));
  return displays;
}

Display DisplayManager::GetPrimary() {
  POINT pt = {0, 0};
  HMONITOR monitor = MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
  return CreateDisplayFromMonitor(monitor, true);
}

Point DisplayManager::GetCursorPosition() {
  POINT cursorPos;
  GetCursorPos(&cursorPos);
  
  Point point;
  point.x = static_cast<double>(cursorPos.x);
  point.y = static_cast<double>(cursorPos.y);
  return point;
}

}  // namespace nativeapi