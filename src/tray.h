#pragma once
#include <string>
#include "geometry.h"
#include "menu.h"
#include "event_dispatcher.h"

namespace nativeapi {

typedef long TrayID;

// Forward declarations
class TrayListener;
class TrayClickedEvent;
class TrayRightClickedEvent;
class TrayDoubleClickedEvent;

class Tray {
 public:
  Tray();
  Tray(void* tray);
  virtual ~Tray();

  TrayID id;

  void SetIcon(std::string icon);

  void SetTitle(std::string title);
  std::string GetTitle();

  void SetTooltip(std::string tooltip);
  std::string GetTooltip();

  void SetContextMenu(Menu menu);
  Menu GetContextMenu();

  Rectangle GetBounds();

  // Event handling methods
  /**
   * Add a callback function as a listener for tray clicked events.
   * @param callback Function to call when the tray is clicked
   * @return A unique listener ID that can be used to remove the listener
   */
  size_t AddClickedListener(std::function<void(const TrayClickedEvent&)> callback);

  /**
   * Add a callback function as a listener for tray right-clicked events.
   * @param callback Function to call when the tray is right-clicked
   * @return A unique listener ID that can be used to remove the listener
   */
  size_t AddRightClickedListener(std::function<void(const TrayRightClickedEvent&)> callback);

  /**
   * Add a callback function as a listener for tray double-clicked events.
   * @param callback Function to call when the tray is double-clicked
   * @return A unique listener ID that can be used to remove the listener
   */
  size_t AddDoubleClickedListener(std::function<void(const TrayDoubleClickedEvent&)> callback);

  /**
   * Remove a listener by its ID.
   * @param listener_id The ID returned by AddListener
   * @return true if the listener was found and removed, false otherwise
   */
  bool RemoveListener(size_t listener_id);

  /**
   * Remove all listeners.
   */
  void RemoveAllListeners();

 private:
  class Impl;
  Impl* pimpl_;
  
  // Event dispatcher for tray events
  EventDispatcher event_dispatcher_;

  // Methods for internal event dispatching (used by platform implementations)
  void DispatchClickedEvent();
  void DispatchRightClickedEvent();
  void DispatchDoubleClickedEvent();
};

}  // namespace nativeapi