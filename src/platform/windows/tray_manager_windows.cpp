#include <shellapi.h>
#include <windows.h>
#include <memory>
#include <mutex>

#include "../../tray_icon.h"
#include "../../tray_manager.h"

namespace nativeapi {

TrayManager::TrayManager() : next_tray_id_(1) {}

TrayManager::~TrayManager() {
  std::lock_guard<std::mutex> lock(mutex_);
  // Clean up all managed tray icons
  for (auto& pair : trays_) {
    auto tray = pair.second;
    if (tray) {
      // The TrayIcon destructor will handle cleanup of the Windows tray icon
    }
  }
  trays_.clear();
}

bool TrayManager::IsSupported() {
  // System tray is generally supported on Windows
  // We could add additional checks here if needed
  return true;
}

std::shared_ptr<TrayIcon> TrayManager::Create() {
  std::lock_guard<std::mutex> lock(mutex_);

  // Create a new tray icon instance
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

  std::vector<std::shared_ptr<TrayIcon>> result;
  for (const auto& pair : trays_) {
    result.push_back(pair.second);
  }
  return result;
}

bool TrayManager::Destroy(TrayIconID id) {
  std::lock_guard<std::mutex> lock(mutex_);

  auto it = trays_.find(id);
  if (it != trays_.end()) {
    // Remove the tray icon from our container
    // The shared_ptr will automatically clean up when the last reference is
    // released
    trays_.erase(it);
    return true;
  }
  return false;
}

}  // namespace nativeapi
