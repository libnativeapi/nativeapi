#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "tray.h"

namespace nativeapi {

// TrayManager is a singleton that manages all system tray icons.
class TrayManager {
 public:
  TrayManager();
  virtual ~TrayManager();

  // Create a new tray.
  std::shared_ptr<Tray> Create();

  // Get a tray by its ID. Returns nullptr if tray not found.
  std::shared_ptr<Tray> Get(TrayID id);

  // Get all trays.
  std::vector<std::shared_ptr<Tray>> GetAll();

  // Destroy a tray by its ID. Returns true if tray was found and destroyed.
  bool Destroy(TrayID id);

 private:
  // Store tray instances
  std::unordered_map<TrayID, std::shared_ptr<Tray>> trays_;
  // ID generator for new trays
  TrayID next_tray_id_;
};

}  // namespace nativeapi
