#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "tray.h"

namespace nativeapi {

// WindowManager is a singleton that manages all windows on the system.
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

 private:
  // Store tray instances
  std::unordered_map<TrayID, std::shared_ptr<Tray>> trays_;
};

}  // namespace nativeapi