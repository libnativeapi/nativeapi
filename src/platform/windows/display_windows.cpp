#include "../../display.h"

#include <windows.h>

namespace nativeapi {

// Private implementation class
class Display::Impl {
 public:
  Impl() = default;
  Impl(HMONITOR monitor) : h_monitor_(monitor) {}

  HMONITOR h_monitor_ = nullptr;
};

Display::Display() : pimpl_(std::make_unique<Impl>()) {}

Display::Display(void* display) : pimpl_(std::make_unique<Impl>()) {
  if (display) {
    pimpl_->h_monitor_ = (HMONITOR)display;
  }
}

Display::Display(const Display& other)
    : pimpl_(std::make_unique<Impl>(*other.pimpl_)) {}

Display& Display::operator=(const Display& other) {
  if (this != &other) {
    *pimpl_ = *other.pimpl_;
  }
  return *this;
}

Display::Display(Display&& other) noexcept : pimpl_(std::move(other.pimpl_)) {}

Display& Display::operator=(Display&& other) noexcept {
  if (this != &other) {
    pimpl_ = std::move(other.pimpl_);
  }
  return *this;
}

Display::~Display() = default;

void* Display::GetNativeObjectInternal() const {
  return pimpl_->h_monitor_;
}

// Helper function to get monitor info
MONITORINFOEX GetMonitorInfo(HMONITOR hMonitor) {
  MONITORINFOEX monitorInfo;
  monitorInfo.cbSize = sizeof(MONITORINFOEX);
  GetMonitorInfo(hMonitor, &monitorInfo);
  return monitorInfo;
}

// Getters - directly read from HMONITOR
std::string Display::GetId() const {
  if (!pimpl_->h_monitor_)
    return "";
  return std::to_string(reinterpret_cast<uintptr_t>(pimpl_->h_monitor_));
}

std::string Display::GetName() const {
  if (!pimpl_->h_monitor_)
    return "";
  MONITORINFOEX monitorInfo = GetMonitorInfo(pimpl_->h_monitor_);
  return monitorInfo.szDevice;
}

Point Display::GetPosition() const {
  if (!pimpl_->h_monitor_)
    return {0.0, 0.0};
  MONITORINFOEX monitorInfo = GetMonitorInfo(pimpl_->h_monitor_);
  RECT rect = monitorInfo.rcMonitor;
  return {static_cast<double>(rect.left), static_cast<double>(rect.top)};
}

Size Display::GetSize() const {
  if (!pimpl_->h_monitor_)
    return {0.0, 0.0};
  MONITORINFOEX monitorInfo = GetMonitorInfo(pimpl_->h_monitor_);
  RECT rect = monitorInfo.rcMonitor;
  return {static_cast<double>(rect.right - rect.left),
          static_cast<double>(rect.bottom - rect.top)};
}

Rectangle Display::GetWorkArea() const {
  if (!pimpl_->h_monitor_)
    return {0.0, 0.0, 0.0, 0.0};
  MONITORINFOEX monitorInfo = GetMonitorInfo(pimpl_->h_monitor_);
  RECT workRect = monitorInfo.rcWork;
  return {static_cast<double>(workRect.left), static_cast<double>(workRect.top),
          static_cast<double>(workRect.right - workRect.left),
          static_cast<double>(workRect.bottom - workRect.top)};
}

double Display::GetScaleFactor() const {
  if (!pimpl_->h_monitor_)
    return 1.0;
  HDC hdc = GetDC(nullptr);
  double scaleFactor = 1.0;
  if (hdc) {
    int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
    scaleFactor = dpiX / 96.0;  // 96 DPI is 100% scale
    ReleaseDC(nullptr, hdc);
  }
  return scaleFactor;
}

bool Display::IsPrimary() const {
  if (!pimpl_->h_monitor_)
    return false;
  MONITORINFOEX monitorInfo = GetMonitorInfo(pimpl_->h_monitor_);
  return (monitorInfo.dwFlags & MONITORINFOF_PRIMARY) != 0;
}

DisplayOrientation Display::GetOrientation() const {
  if (!pimpl_->h_monitor_)
    return DisplayOrientation::kPortrait;
  Size size = GetSize();
  return (size.width > size.height) ? DisplayOrientation::kLandscape
                                    : DisplayOrientation::kPortrait;
}

int Display::GetRefreshRate() const {
  return 60;  // Default refresh rate, would need additional Windows APIs to get
              // actual value
}

int Display::GetBitDepth() const {
  return 32;  // Default bit depth for modern displays
}

}  // namespace nativeapi
