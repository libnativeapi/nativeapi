#include <cstring>
#include <iostream>
#include <string>
#include <mutex>

#include "../../tray_icon.h"
#include "../../tray_manager.h"

// Import Cocoa headers
#import <Cocoa/Cocoa.h>

namespace nativeapi {

TrayManager::TrayManager() : next_tray_id_(1) {}

TrayManager::~TrayManager() {
  std::lock_guard<std::mutex> lock(mutex_);
  // Clean up all managed tray icons
  for (auto& pair : trays_) {
    auto tray = pair.second;
    if (tray) {
      // The TrayIcon destructor will handle cleanup of the NSStatusItem
    }
  }
  trays_.clear();
}

bool TrayManager::IsSupported() {
  return true;
}

std::shared_ptr<TrayIcon> TrayManager::Create() {
  std::lock_guard<std::mutex> lock(mutex_);

  NSStatusBar* status_bar = [NSStatusBar systemStatusBar];
  NSStatusItem* status_item = [status_bar statusItemWithLength:NSVariableStatusItemLength];

  auto tray = std::make_shared<TrayIcon>((__bridge void*)status_item);
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
