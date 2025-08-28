#pragma once

#include <functional>
#include <memory>
#include <typeindex>

#include "event.h"
#include "event_dispatcher.h"

namespace nativeapi {

/**
 * Base class that provides event emission capabilities.
 * Classes that inherit from EventEmitter can easily add listener management
 * and event dispatching functionality.
 * 
 * Example usage:
 * 
 * class MyClass : public EventEmitter {
 * public:
 *   void DoSomething() {
 *     // Emit an event
 *     EmitSync<MyEvent>("some data");
 *   }
 * };
 * 
 * MyClass obj;
 * obj.AddListener<MyEvent>([](const MyEvent& event) {
 *   // Handle the event
 * });
 */
class EventEmitter {
 public:
  EventEmitter();
  virtual ~EventEmitter();

  /**
   * Add a typed event listener for a specific event type.
   * 
   * @param listener Pointer to the event listener (must remain valid until removed)
   * @return A unique listener ID that can be used to remove the listener
   */
  template <typename EventType>
  size_t AddListener(TypedEventListener<EventType>* listener) {
    return dispatcher_.AddListener<EventType>(listener);
  }

  /**
   * Add a callback function as a listener for a specific event type.
   * 
   * @param callback Function to call when the event occurs
   * @return A unique listener ID that can be used to remove the listener
   */
  template <typename EventType>
  size_t AddListener(std::function<void(const EventType&)> callback) {
    return dispatcher_.AddListener<EventType>(std::move(callback));
  }

  /**
   * Remove a listener by its ID.
   * 
   * @param listener_id The ID returned by AddListener
   * @return true if the listener was found and removed, false otherwise
   */
  bool RemoveListener(size_t listener_id);

  /**
   * Remove all listeners for a specific event type.
   */
  template <typename EventType>
  void RemoveAllListeners() {
    dispatcher_.RemoveAllListeners<EventType>();
  }

  /**
   * Remove all listeners for all event types.
   */
  void RemoveAllListeners();

  /**
   * Get the number of listeners registered for a specific event type.
   */
  template <typename EventType>
  size_t GetListenerCount() const {
    return dispatcher_.GetListenerCount<EventType>();
  }

  /**
   * Get the total number of registered listeners.
   */
  size_t GetTotalListenerCount() const;

  /**
   * Check if there are any listeners for a specific event type.
   */
  template <typename EventType>
  bool HasListeners() const {
    return GetListenerCount<EventType>() > 0;
  }

 protected:
  /**
   * Emit an event synchronously to all registered listeners.
   * This will call all listeners immediately on the current thread.
   * 
   * @param event The event to emit
   */
  void EmitSync(const Event& event);

  /**
   * Emit an event synchronously using perfect forwarding.
   * This creates the event object and emits it immediately.
   */
  template <typename EventType, typename... Args>
  void EmitSync(Args&&... args) {
    dispatcher_.DispatchSync<EventType>(std::forward<Args>(args)...);
  }

  /**
   * Emit an event asynchronously.
   * The event will be dispatched on a background thread.
   * 
   * @param event The event to emit (will be copied)
   */
  void EmitAsync(std::unique_ptr<Event> event);

  /**
   * Emit an event asynchronously using perfect forwarding.
   */
  template <typename EventType, typename... Args>
  void EmitAsync(Args&&... args) {
    dispatcher_.DispatchAsync<EventType>(std::forward<Args>(args)...);
  }

  /**
   * Get access to the underlying event dispatcher.
   * This can be useful for advanced use cases.
   */
  EventDispatcher& GetEventDispatcher();
  const EventDispatcher& GetEventDispatcher() const;

 private:
  EventDispatcher dispatcher_;
};

/**
 * Convenience macro to define a simple event class.
 * 
 * Usage:
 * DEFINE_EVENT(MyEvent) {
 *   std::string message;
 *   int code;
 * };
 */
#define DEFINE_EVENT(EventName) \
  class EventName : public TypedEvent<EventName>

/**
 * Helper macro to begin defining an event with custom constructor.
 * 
 * Usage:
 * DEFINE_EVENT_BEGIN(MyEvent)
 *   std::string message;
 *   int code;
 *   MyEvent(std::string msg, int c) : message(std::move(msg)), code(c) {}
 * DEFINE_EVENT_END();
 */
#define DEFINE_EVENT_BEGIN(EventName) \
  class EventName : public TypedEvent<EventName> { \
   public:

#define DEFINE_EVENT_END() \
  };

/**
 * Simpler macro for events with basic data members.
 * Creates a constructor that initializes all members.
 */
#define SIMPLE_EVENT(EventName, ...) \
  class EventName : public TypedEvent<EventName> { \
   public: \
    __VA_ARGS__ \
  };

}  // namespace nativeapi