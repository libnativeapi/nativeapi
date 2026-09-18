#include "../../display.h"

#include <gdk/gdk.h>
#include <gtk/gtk.h>

namespace nativeapi {

// Private implementation class
class Display::Impl {
 public:
  Impl() = default;
  Impl(GdkMonitor* monitor) { SetMonitor(monitor); }

  ~Impl() {
    if (gdk_monitor_) {
      g_object_remove_weak_pointer(G_OBJECT(gdk_monitor_), (gpointer*)&gdk_monitor_);
    }
  }

  // A GdkMonitor is owned by its GdkDisplay, which drops and recreates monitors
  // on hotplug and whenever the compositor turns the outputs off and on again.
  // Let GDK clear the pointer so the getters below report defaults instead of
  // reading freed memory.
  void SetMonitor(GdkMonitor* monitor) {
    if (!monitor) {
      return;
    }
    gdk_monitor_ = monitor;
    g_object_add_weak_pointer(G_OBJECT(gdk_monitor_), (gpointer*)&gdk_monitor_);
  }

  const DisplayId id_ = IdAllocator::Allocate<Display>();
  GdkMonitor* gdk_monitor_ = nullptr;
};

Display::Display(void* display) : pimpl_(std::make_unique<Impl>()) {
  pimpl_->SetMonitor((GdkMonitor*)display);
}

Display::~Display() = default;

void* Display::GetNativeObjectInternal() const {
  return pimpl_->gdk_monitor_;
}

// Getters - directly read from GdkMonitor
DisplayId Display::GetId() const {
  return pimpl_->id_;
}

std::string Display::GetName() const {
  if (!pimpl_->gdk_monitor_)
    return "";
  const char* model = gdk_monitor_get_model(pimpl_->gdk_monitor_);
  return model ? model : "Unknown";
}

Point Display::GetPosition() const {
  if (!pimpl_->gdk_monitor_)
    return {0.0, 0.0};
  GdkRectangle geometry;
  gdk_monitor_get_geometry(pimpl_->gdk_monitor_, &geometry);
  return {static_cast<double>(geometry.x), static_cast<double>(geometry.y)};
}

Size Display::GetSize() const {
  if (!pimpl_->gdk_monitor_)
    return {0.0, 0.0};
  GdkRectangle geometry;
  gdk_monitor_get_geometry(pimpl_->gdk_monitor_, &geometry);
  return {static_cast<double>(geometry.width), static_cast<double>(geometry.height)};
}

Rectangle Display::GetWorkArea() const {
  if (!pimpl_->gdk_monitor_)
    return {0.0, 0.0, 0.0, 0.0};
  GdkRectangle workarea;
  gdk_monitor_get_workarea(pimpl_->gdk_monitor_, &workarea);
  return {static_cast<double>(workarea.x), static_cast<double>(workarea.y),
          static_cast<double>(workarea.width), static_cast<double>(workarea.height)};
}

double Display::GetScaleFactor() const {
  if (!pimpl_->gdk_monitor_)
    return 1.0;
  return gdk_monitor_get_scale_factor(pimpl_->gdk_monitor_);
}

bool Display::IsPrimary() const {
  if (!pimpl_->gdk_monitor_)
    return false;
  GdkDisplay* display = gdk_monitor_get_display(pimpl_->gdk_monitor_);
  GdkMonitor* primary = gdk_display_get_primary_monitor(display);
  if (!primary) {
    // Wayland has no notion of a primary monitor; match the first-monitor
    // convention DisplayManager::EnumerateNativeDisplays() uses.
    primary = gdk_display_get_monitor(display, 0);
  }
  return primary == pimpl_->gdk_monitor_;
}

DisplayOrientation Display::GetOrientation() const {
  if (!pimpl_->gdk_monitor_)
    return DisplayOrientation::kPortrait;
  GdkRectangle geometry;
  gdk_monitor_get_geometry(pimpl_->gdk_monitor_, &geometry);
  return (geometry.width > geometry.height) ? DisplayOrientation::kLandscape
                                            : DisplayOrientation::kPortrait;
}

int Display::GetRefreshRate() const {
  if (!pimpl_->gdk_monitor_)
    return 60;
  int refresh_rate = gdk_monitor_get_refresh_rate(pimpl_->gdk_monitor_);
  return refresh_rate > 0 ? refresh_rate / 1000 : 60;  // Convert from millihertz to hertz
}

int Display::GetBitDepth() const {
  return 32;  // Default for modern displays
}

}  // namespace nativeapi