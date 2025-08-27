#pragma once

#include "event.h"
#include "tray.h"

namespace nativeapi {

/**
 * Event class for tray left click
 */
class TrayClickedEvent : public TypedEvent<TrayClickedEvent> {
 public:
  explicit TrayClickedEvent(TrayID tray_id) : tray_id_(tray_id) {}

  TrayID GetTrayId() const { return tray_id_; }

 private:
  TrayID tray_id_;
};

/**
 * Event class for tray right click
 */
class TrayRightClickedEvent : public TypedEvent<TrayRightClickedEvent> {
 public:
  explicit TrayRightClickedEvent(TrayID tray_id) : tray_id_(tray_id) {}

  TrayID GetTrayId() const { return tray_id_; }

 private:
  TrayID tray_id_;
};

/**
 * Event class for tray double click
 */
class TrayDoubleClickedEvent : public TypedEvent<TrayDoubleClickedEvent> {
 public:
  explicit TrayDoubleClickedEvent(TrayID tray_id) : tray_id_(tray_id) {}

  TrayID GetTrayId() const { return tray_id_; }

 private:
  TrayID tray_id_;
};

/**
 * Convenience base class for handling all tray events in one listener
 */
class TrayListener {
 public:
  virtual ~TrayListener() = default;

  /**
   * Called when a tray icon is left-clicked
   */
  virtual void OnTrayClicked(const TrayClickedEvent& event) {}

  /**
   * Called when a tray icon is right-clicked
   */
  virtual void OnTrayRightClicked(const TrayRightClickedEvent& event) {}

  /**
   * Called when a tray icon is double-clicked
   */
  virtual void OnTrayDoubleClicked(const TrayDoubleClickedEvent& event) {}
};

/**
 * Typed tray event listeners for specific event types
 */
using TrayClickedListener = TypedEventListener<TrayClickedEvent>;
using TrayRightClickedListener = TypedEventListener<TrayRightClickedEvent>;
using TrayDoubleClickedListener = TypedEventListener<TrayDoubleClickedEvent>;

}  // namespace nativeapi