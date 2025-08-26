#pragma once

/**
 * Generic Event Handling System
 * 
 * This header provides a comprehensive, type-safe event handling mechanism
 * that supports both observer patterns and callback-based event handling.
 * 
 * Key Features:
 * - Type-safe event dispatching using templates
 * - Both synchronous and asynchronous event processing
 * - Observer pattern support with EventListener interface
 * - Callback-based handling with std::function
 * - Thread-safe operations
 * - Automatic lifetime management for callback listeners
 * - Event timestamping
 * - Exception safety in event handling
 * 
 * Usage Examples:
 * 
 * 1. Define custom events:
 *    class MyEvent : public TypedEvent<MyEvent> {
 *        // Add event data here
 *    };
 * 
 * 2. Use observer pattern:
 *    class MyListener : public TypedEventListener<MyEvent> {
 *        void OnTypedEvent(const MyEvent& event) override {
 *            // Handle event
 *        }
 *    };
 * 
 * 3. Use callback pattern:
 *    dispatcher.AddListener<MyEvent>([](const MyEvent& event) {
 *        // Handle event
 *    });
 * 
 * 4. Dispatch events:
 *    dispatcher.DispatchSync(MyEvent()); // Synchronous
 *    dispatcher.DispatchAsync(MyEvent()); // Asynchronous
 */

#include "event.h"
#include "event_dispatcher.h"

namespace nativeapi {

/**
 * Convenience function to create a global event dispatcher instance.
 * This is useful for applications that need a single, shared event system.
 */
EventDispatcher& GetGlobalEventDispatcher();

/**
 * RAII helper class for automatically removing event listeners.
 * This ensures listeners are properly cleaned up when they go out of scope.
 */
class EventListenerGuard {
 public:
  EventListenerGuard(EventDispatcher& dispatcher, size_t listener_id)
      : dispatcher_(&dispatcher), listener_id_(listener_id) {}
  
  ~EventListenerGuard() {
    if (dispatcher_ && listener_id_ != 0) {
      dispatcher_->RemoveListener(listener_id_);
    }
  }
  
  // Non-copyable but movable
  EventListenerGuard(const EventListenerGuard&) = delete;
  EventListenerGuard& operator=(const EventListenerGuard&) = delete;
  
  EventListenerGuard(EventListenerGuard&& other) noexcept
      : dispatcher_(other.dispatcher_), listener_id_(other.listener_id_) {
    other.dispatcher_ = nullptr;
    other.listener_id_ = 0;
  }
  
  EventListenerGuard& operator=(EventListenerGuard&& other) noexcept {
    if (this != &other) {
      if (dispatcher_ && listener_id_ != 0) {
        dispatcher_->RemoveListener(listener_id_);
      }
      dispatcher_ = other.dispatcher_;
      listener_id_ = other.listener_id_;
      other.dispatcher_ = nullptr;
      other.listener_id_ = 0;
    }
    return *this;
  }
  
  size_t GetListenerId() const { return listener_id_; }
  
 private:
  EventDispatcher* dispatcher_;
  size_t listener_id_;
};

/**
 * Helper function to create an EventListenerGuard for automatic cleanup.
 */
template<typename EventType>
EventListenerGuard AddScopedListener(
    EventDispatcher& dispatcher,
    std::function<void(const EventType&)> callback) {
  size_t id = dispatcher.AddListener<EventType>(std::move(callback));
  return EventListenerGuard(dispatcher, id);
}

}  // namespace nativeapi