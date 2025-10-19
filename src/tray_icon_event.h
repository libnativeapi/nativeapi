#pragma once

#include <string>
#include "foundation/event.h"
#include "foundation/id_allocator.h"

namespace nativeapi {

// Forward declaration for tray icon types
typedef IdAllocator::IdType TrayIconId;

/**
 * @brief Base class for all tray icon-related events.
 *
 * This class provides common functionality for tray icon events.
 */
class TrayIconEvent : public Event {
 public:
  virtual ~TrayIconEvent() = default;

  std::string GetTypeName() const override { return "TrayIconEvent"; }
};

/**
 * @brief Tray icon clicked event.
 *
 * This event is fired when a tray icon is clicked (left-clicked).
 */
class TrayIconClickedEvent : public TrayIconEvent {
 public:
  TrayIconClickedEvent(TrayIconId tray_icon_id) : tray_icon_id_(tray_icon_id) {}

  TrayIconId GetTrayIconId() const { return tray_icon_id_; }

  std::string GetTypeName() const override { return "TrayIconClickedEvent"; }

 private:
  TrayIconId tray_icon_id_;
};

/**
 * @brief Tray icon right-clicked event.
 *
 * This event is fired when a tray icon is right-clicked.
 */
class TrayIconRightClickedEvent : public TrayIconEvent {
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
class TrayIconDoubleClickedEvent : public TrayIconEvent {
 public:
  TrayIconDoubleClickedEvent(TrayIconId tray_icon_id) : tray_icon_id_(tray_icon_id) {}

  TrayIconId GetTrayIconId() const { return tray_icon_id_; }

  std::string GetTypeName() const override { return "TrayIconDoubleClickedEvent"; }

 private:
  TrayIconId tray_icon_id_;
};

}  // namespace nativeapi
