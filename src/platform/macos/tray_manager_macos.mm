#include <cstring>
#include <iostream>
#include <mutex>
#include <string>

#include "../../menu.h"
#include "../../tray_icon.h"
#include "../../tray_manager.h"

// Import Cocoa headers
#import <Cocoa/Cocoa.h>

namespace nativeapi {

class TrayManager::Impl {
 public:
  Impl() {}
  ~Impl() {}
};

TrayManager::TrayManager() : next_tray_id_(1), pimpl_(std::make_unique<Impl>()) {}

TrayManager::~TrayManager() {
  std::lock_guard<std::mutex> lock(mutex_);

  // First, hide all tray icons to prevent further UI interactions
  for (auto& pair : trays_) {
    auto tray = pair.second;
    if (tray) {
      try {
        tray->SetVisible(false);
      } catch (...) {
        // Ignore exceptions during cleanup
      }
    }
  }

  // Then, clean up all tray icon menu references to prevent circular references
  for (auto& pair : trays_) {
    auto tray = pair.second;
    if (tray) {
      try {
        // Explicitly clear menu references
        tray->SetContextMenu(nullptr);
      } catch (...) {
        // Ignore exceptions during cleanup
      }
    }
  }

  // Finally, clear the container
  trays_.clear();
}

bool TrayManager::IsSupported() {
  return true;
}

std::shared_ptr<TrayIcon> TrayManager::Create() {
  std::lock_guard<std::mutex> lock(mutex_);

  // Create tray icon with platform-specific initialization handled internally
  auto tray = std::make_shared<TrayIcon>();
  tray->id = next_tray_id_++;
  trays_[tray->id] = tray;

  return tray;
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
