#include <windows.h>
#include <shellapi.h>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "../../tray_manager.h"
#include "../../tray.h"

namespace nativeapi {

TrayManager::TrayManager() : next_tray_id_(1) {}

TrayManager::~TrayManager() {
    // Clean up all trays
    for (auto& pair : trays_) {
        auto tray = pair.second;
        if (tray) {
            // The tray destructor will handle cleanup
        }
    }
    trays_.clear();
}

std::shared_ptr<Tray> TrayManager::Create() {
    auto tray = std::make_shared<Tray>();
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
    std::vector<std::shared_ptr<Tray>> result;
    for (const auto& pair : trays_) {
        if (auto tray = pair.second) {
            result.push_back(tray);
        }
    }
    return result;
}

}  // namespace nativeapi