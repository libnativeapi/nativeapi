#pragma once

#include <functional>
#include <memory>
#include <string>

#include "event_dispatcher.h"
#include "keyboard_events.h"

namespace nativeapi {

enum class ModifierKey : uint32_t {
  None = 0,
  Shift = 1 << 0,
  Ctrl = 1 << 1,
  Alt = 1 << 2,
  Meta = 1 << 3,  // Windows key or Cmd key
  Fn = 1 << 4,
  CapsLock = 1 << 5,
  NumLock = 1 << 6,
  ScrollLock = 1 << 7
};

// Bitwise operators for ModifierKey enum
inline ModifierKey operator|(ModifierKey a, ModifierKey b) {
  return static_cast<ModifierKey>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline ModifierKey operator&(ModifierKey a, ModifierKey b) {
  return static_cast<ModifierKey>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

class KeyboardMonitor {
 public:
  KeyboardMonitor();
  virtual ~KeyboardMonitor();

  // Add a listener for keyboard events
  template<typename EventType>
  size_t AddListener(TypedEventListener<EventType>* listener) {
    return event_dispatcher_.AddListener(listener);
  }

  // Add a callback-based listener for keyboard events
  template<typename EventType>
  size_t AddListener(std::function<void(const EventType&)> callback) {
    return event_dispatcher_.AddListener(std::move(callback));
  }

  // Remove a listener by ID
  bool RemoveListener(size_t listener_id) {
    return event_dispatcher_.RemoveListener(listener_id);
  }

  // Remove all listeners for a specific event type
  template<typename EventType>
  void RemoveAllListeners() {
    event_dispatcher_.RemoveAllListeners<EventType>();
  }

  // Start the keyboard monitor
  void Start();

  // Stop the keyboard monitor
  void Stop();

  // Check if the keyboard monitor is monitoring
  bool IsMonitoring() const;

  // Get access to the event dispatcher for advanced usage
  EventDispatcher& GetEventDispatcher() { return event_dispatcher_; }

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
  EventDispatcher event_dispatcher_;

  // Internal method for dispatching events from platform code
  void DispatchEvent(const Event& event);
  friend class Impl;
};

}  // namespace nativeapi
