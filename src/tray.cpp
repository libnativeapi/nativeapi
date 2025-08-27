#include "tray.h"
#include "tray_events.h"
#include <functional>

namespace nativeapi {

size_t Tray::AddClickedListener(std::function<void(const TrayClickedEvent&)> callback) {
  return event_dispatcher_.AddListener<TrayClickedEvent>(std::move(callback));
}

size_t Tray::AddRightClickedListener(std::function<void(const TrayRightClickedEvent&)> callback) {
  return event_dispatcher_.AddListener<TrayRightClickedEvent>(std::move(callback));
}

size_t Tray::AddDoubleClickedListener(std::function<void(const TrayDoubleClickedEvent&)> callback) {
  return event_dispatcher_.AddListener<TrayDoubleClickedEvent>(std::move(callback));
}

bool Tray::RemoveListener(size_t listener_id) {
  return event_dispatcher_.RemoveListener(listener_id);
}

void Tray::RemoveAllListeners() {
  event_dispatcher_.RemoveAllListeners();
}

void Tray::DispatchClickedEvent() {
  TrayClickedEvent event(id);
  event_dispatcher_.DispatchSync(event);
}

void Tray::DispatchRightClickedEvent() {
  TrayRightClickedEvent event(id);
  event_dispatcher_.DispatchSync(event);
}

void Tray::DispatchDoubleClickedEvent() {
  TrayDoubleClickedEvent event(id);
  event_dispatcher_.DispatchSync(event);
}

}  // namespace nativeapi