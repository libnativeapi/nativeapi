#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <vector>

#include "display.h"
#include "geometry.h"
#include "event.h"
#include "event_dispatcher.h"

namespace nativeapi {

// Event classes for display changes
class DisplayAddedEvent : public TypedEvent<DisplayAddedEvent> {
 public:
  explicit DisplayAddedEvent(const Display& display) : display_(display) {}
  
  const Display& GetDisplay() const { return display_; }

 private:
  Display display_;
};

class DisplayRemovedEvent : public TypedEvent<DisplayRemovedEvent> {
 public:
  explicit DisplayRemovedEvent(const Display& display) : display_(display) {}
  
  const Display& GetDisplay() const { return display_; }

 private:
  Display display_;
};

// DisplayListener is an interface that can be implemented by classes that want
// to listen for display events.
class DisplayListener {
 public:
  virtual void OnDisplayAdded(const Display& display) = 0;
  virtual void OnDisplayRemoved(const Display& display) = 0;
};

// DisplayManager is a singleton that manages all displays on the system.
class DisplayManager {
 public:
  DisplayManager();
  virtual ~DisplayManager();

  // Get all displays information
  std::vector<Display> GetAll();

  // Get the primary display information
  Display GetPrimary();

  // Get the current cursor position
  Point GetCursorPosition();

  // Add a listener to the display manager
  void AddListener(DisplayListener* listener);

  // Remove a listener from the display manager
  void RemoveListener(DisplayListener* listener);

  // Event dispatcher methods for the new system
  template<typename EventType>
  size_t AddEventListener(TypedEventListener<EventType>* listener) {
    return event_dispatcher_.AddListener<EventType>(listener);
  }

  template<typename EventType>
  size_t AddEventListener(std::function<void(const EventType&)> callback) {
    return event_dispatcher_.AddListener<EventType>(std::move(callback));
  }

  bool RemoveEventListener(size_t listener_id) {
    return event_dispatcher_.RemoveListener(listener_id);
  }

  // Get the event dispatcher (for advanced usage)
  EventDispatcher& GetEventDispatcher() { return event_dispatcher_; }

 private:
  std::vector<Display> displays_;
  std::vector<DisplayListener*> listeners_;  // For backward compatibility
  EventDispatcher event_dispatcher_;  // New event system
  
  void NotifyListeners(std::function<void(DisplayListener*)> callback);
  
  // New event dispatch methods
  void DispatchDisplayAddedEvent(const Display& display);
  void DispatchDisplayRemovedEvent(const Display& display);
};

// DisplayEventHandler is an implementation of DisplayListener that uses
// callbacks to handle display events.
class DisplayEventHandler : public DisplayListener {
 public:
  // Constructor that takes callbacks for display events
  DisplayEventHandler(
      std::function<void(const Display&)> onDisplayAddedCallback,
      std::function<void(const Display&)> onDisplayRemovedCallback);

  // Implementation of OnDisplayAdded from DisplayListener interface
  void OnDisplayAdded(const Display& display) override;

  // Implementation of OnDisplayRemoved from DisplayListener interface
  void OnDisplayRemoved(const Display& display) override;

 private:
  std::function<void(const Display&)> onDisplayAddedCallback_;
  std::function<void(const Display&)> onDisplayRemovedCallback_;
};

}  // namespace nativeapi
