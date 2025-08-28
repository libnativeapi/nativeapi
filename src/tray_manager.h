#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "tray_icon.h"

namespace nativeapi {

// TrayManager is a singleton that manages all system tray icons.
class TrayManager {
 public:
  TrayManager();
  virtual ~TrayManager();

  // Check if system tray is supported.
  bool IsSupported();

  // Create a new tray.
  std::shared_ptr<TrayIcon> Create();

  // Get a tray by its ID. Returns nullptr if tray not found.
  std::shared_ptr<TrayIcon> Get(TrayIconID id);

  // Get all trays.
  std::vector<std::shared_ptr<TrayIcon>> GetAll();

  // Destroy a tray by its ID. Returns true if tray was found and destroyed.
  bool Destroy(TrayIconID id);

 private:
  // Store tray instances
  std::unordered_map<TrayIconID, std::shared_ptr<TrayIcon>> trays_;
  // ID generator for new trays
  TrayIconID next_tray_id_;
};

}  // namespace nativeapi
