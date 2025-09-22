#include "../../display.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>

namespace nativeapi {

// Private implementation class
class Display::Impl {
 public:
  Impl() = default;
  Impl(GdkMonitor* monitor) : gdk_monitor_(monitor) {}
  
  GdkMonitor* gdk_monitor_ = nullptr;
};

Display::Display() : pimpl_(std::make_unique<Impl>()) {}

Display::Display(void* display) : pimpl_(std::make_unique<Impl>()) {
  if (display) {
    pimpl_->gdk_monitor_ = (GdkMonitor*)display;
  }
}

Display::Display(const Display& other) : pimpl_(std::make_unique<Impl>(*other.pimpl_)) {}

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
  return pimpl_->gdk_monitor_;
}

// Getters - directly read from GdkMonitor
std::string Display::GetId() const {
  if (!pimpl_->gdk_monitor_) return "";
  // Use monitor pointer as ID since GDK doesn't provide direct monitor IDs
  return std::to_string(reinterpret_cast<uintptr_t>(pimpl_->gdk_monitor_));
}

std::string Display::GetName() const {
  if (!pimpl_->gdk_monitor_) return "";
  const char* model = gdk_monitor_get_model(pimpl_->gdk_monitor_);
  return model ? model : "Unknown";
}

Point Display::GetPosition() const {
  if (!pimpl_->gdk_monitor_) return {0.0, 0.0};
  GdkRectangle geometry;
  gdk_monitor_get_geometry(pimpl_->gdk_monitor_, &geometry);
  return {static_cast<double>(geometry.x), static_cast<double>(geometry.y)};
}

Size Display::GetSize() const {
  if (!pimpl_->gdk_monitor_) return {0.0, 0.0};
  GdkRectangle geometry;
  gdk_monitor_get_geometry(pimpl_->gdk_monitor_, &geometry);
  return {static_cast<double>(geometry.width), static_cast<double>(geometry.height)};
}

Rectangle Display::GetWorkArea() const {
  if (!pimpl_->gdk_monitor_) return {0.0, 0.0, 0.0, 0.0};
  GdkRectangle workarea;
  gdk_monitor_get_workarea(pimpl_->gdk_monitor_, &workarea);
  return {static_cast<double>(workarea.x), static_cast<double>(workarea.y),
          static_cast<double>(workarea.height), static_cast<double>(workarea.width)};
}

double Display::GetScaleFactor() const {
  if (!pimpl_->gdk_monitor_) return 1.0;
  return gdk_monitor_get_scale_factor(pimpl_->gdk_monitor_);
}

bool Display::IsPrimary() const {
  if (!pimpl_->gdk_monitor_) return false;
  GdkDisplay* display = gdk_monitor_get_display(pimpl_->gdk_monitor_);
  GdkMonitor* primary = gdk_display_get_primary_monitor(display);
  return primary == pimpl_->gdk_monitor_;
}

DisplayOrientation Display::GetOrientation() const {
  if (!pimpl_->gdk_monitor_) return DisplayOrientation::kPortrait;
  GdkRectangle geometry;
  gdk_monitor_get_geometry(pimpl_->gdk_monitor_, &geometry);
  return (geometry.width > geometry.height) ? 
    DisplayOrientation::kLandscape : DisplayOrientation::kPortrait;
}

int Display::GetRefreshRate() const {
  if (!pimpl_->gdk_monitor_) return 60;
  int refresh_rate = gdk_monitor_get_refresh_rate(pimpl_->gdk_monitor_);
  return refresh_rate > 0 ? refresh_rate / 1000 : 60; // Convert from millihertz to hertz
}

int Display::GetBitDepth() const {
  return 32; // Default for modern displays
}

std::string Display::GetManufacturer() const {
  if (!pimpl_->gdk_monitor_) return "Unknown";
  const char* manufacturer = gdk_monitor_get_manufacturer(pimpl_->gdk_monitor_);
  return manufacturer ? manufacturer : "Unknown";
}

std::string Display::GetModel() const {
  return GetName();
}

std::string Display::GetSerialNumber() const {
  return ""; // Not available in GDK
}



}  // namespace nativeapi