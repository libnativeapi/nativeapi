#pragma once

#include <string>
#include "foundation/event.h"
#include "tray_icon.h"

namespace nativeapi {

/**
 * @brief Tray icon clicked event.
 *
 * This event is fired when a tray icon is clicked (left-clicked).
 */
class TrayIconClickedEvent : public Event {
 public:
  TrayIconClickedEvent(TrayIconId tray_icon_id, const std::string& button)
      : tray_icon_id_(tray_icon_id), button_(button) {}

  TrayIconId GetTrayIconId() const { return tray_icon_id_; }
  const std::string& GetButton() const { return button_; }

  std::string GetTypeName() const override { return "TrayIconClickedEvent"; }

 private:
  TrayIconId tray_icon_id_;
  std::string button_;
};

/**
 * @brief Tray icon right-clicked event.
 *
 * This event is fired when a tray icon is right-clicked.
 */
class TrayIconRightClickedEvent : public Event {
 public:
  TrayIconRightClickedEvent(TrayIconId tray_icon_id) : tray_icon_id_(tray_icon_id) {}

  TrayIconId GetTrayIconId() const { return tray_icon_id_; }

  std::string GetTypeName() const override { return "TrayIconRightClickedEvent"; }

 private:
  TrayIconId tray_icon_id_;
};

/**
 * @brief Tray icon double-clicked event.
 *
 * This event is fired when a tray icon is double-clicked.
 */
class TrayIconDoubleClickedEvent : public Event {
 public:
  TrayIconDoubleClickedEvent(TrayIconId tray_icon_id) : tray_icon_id_(tray_icon_id) {}

  TrayIconId GetTrayIconId() const { return tray_icon_id_; }

  std::string GetTypeName() const override { return "TrayIconDoubleClickedEvent"; }

 private:
  TrayIconId tray_icon_id_;
};

}  // namespace nativeapi
