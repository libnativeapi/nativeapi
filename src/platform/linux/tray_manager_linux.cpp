#include <cstring>
#include <iostream>
#include <string>

#include "../../tray.h"
#include "../../tray_manager.h"

// Import GTK headers
#include <gtk/gtk.h>

namespace nativeapi {

TrayManager::TrayManager() : next_tray_id_(1) {}

TrayManager::~TrayManager() {}

std::shared_ptr<Tray> TrayManager::Create() {
  // Create a new tray using GTK StatusIcon
  GtkStatusIcon* status_icon = gtk_status_icon_new();
  auto tray = std::make_shared<Tray>((void*)status_icon);
  tray->id = next_tray_id_++;
  trays_[tray->id] = tray;
  return tray;
}

std::shared_ptr<Tray> TrayManager::Get(TrayID id) {
  auto it = trays_.find(id);
  if (it != trays_.end()) {
    return it->second;
  }
  return nullptr;
}

std::vector<std::shared_ptr<Tray>> TrayManager::GetAll() {
  std::vector<std::shared_ptr<Tray>> trays;
  for (auto& tray : trays_) {
    trays.push_back(tray.second);
  }
  return trays;
}

}  // namespace nativeapi