#include <memory>
#include <mutex>

#include "../../tray_icon.h"
#include "../../tray_manager.h"

// Import GTK headers - these may not be available in all build environments
#ifdef __has_include
  #if __has_include(<gtk/gtk.h>)
    #include <gtk/gtk.h>
    #define HAS_GTK 1
  #else
    #define HAS_GTK 0
  #endif
#else
  // Fallback for older compilers
  #define HAS_GTK 0
#endif

namespace nativeapi {

class TrayManager::Impl {
 public:
  Impl() {}
  ~Impl() {}
};

TrayManager::TrayManager() : next_tray_id_(1), pimpl_(std::make_unique<Impl>()) {}

TrayManager::~TrayManager() {
  std::lock_guard<std::mutex> lock(mutex_);
  // Clean up all managed tray icons
  for (auto& pair : trays_) {
    auto tray = pair.second;
    if (tray) {
      // The TrayIcon destructor will handle cleanup of the GtkStatusIcon
    }
  }
  trays_.clear();
}

bool TrayManager::IsSupported() {
#if HAS_GTK
  // Check if GTK is initialized and system tray is supported
  return gtk_init_check(nullptr, nullptr);
#else
  // If GTK is not available, assume no system tray support
  return false;
#endif
}

std::shared_ptr<TrayIcon> TrayManager::Create() {
  std::lock_guard<std::mutex> lock(mutex_);

#if HAS_GTK
  // Create a new tray using GTK StatusIcon
  GtkStatusIcon* status_icon = gtk_status_icon_new();
  if (!status_icon) {
    return nullptr;
  }

  auto tray = std::make_shared<TrayIcon>((void*)status_icon);
  tray->id = next_tray_id_++;
  trays_[tray->id] = tray;

  return tray;
#else
  // GTK not available, return nullptr
  return nullptr;
#endif
}

std::shared_ptr<TrayIcon> TrayManager::Get(TrayIconID id) {
  std::lock_guard<std::mutex> lock(mutex_);

  auto it = trays_.find(id);
  if (it != trays_.end()) {
    return it->second;
  }
  return nullptr;
}

std::vector<std::shared_ptr<TrayIcon>> TrayManager::GetAll() {
  std::lock_guard<std::mutex> lock(mutex_);

  std::vector<std::shared_ptr<TrayIcon>> trays;
  for (const auto& pair : trays_) {
    trays.push_back(pair.second);
  }
  return trays;
}

bool TrayManager::Destroy(TrayIconID id) {
  std::lock_guard<std::mutex> lock(mutex_);

  auto it = trays_.find(id);
  if (it != trays_.end()) {
    // Remove the tray icon from our container
    // The shared_ptr will automatically clean up when the last reference is released
    trays_.erase(it);
    return true;
  }
  return false;
}

}  // namespace nativeapi
