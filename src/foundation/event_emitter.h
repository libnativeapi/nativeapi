#pragma once

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "event.h"

namespace nativeapi {

/**
 * Base class that provides event emission capabilities.
 * Classes that inherit from EventEmitter can easily add listener management
 * and event dispatching functionality. Supports both synchronous and asynchronous
 * event dispatching with thread-safe listener management.
 *
 * Example usage:
 *
 * class MyClass : public EventEmitter {
 * public:
 *   void DoSomething() {
 *     // Emit an event synchronously
 *     EmitSync<MyEvent>("some data");
 *
 *     // Or emit asynchronously
 *     EmitAsync<MyEvent>("some data");
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
    return AddListener(TypedEvent<EventType>::GetStaticType(), listener);
  }

  /**
   * Add a callback function as a listener for a specific event type.
   *
   * @param callback Function to call when the event occurs
   * @return A unique listener ID that can be used to remove the listener
   */
  template <typename EventType>
  size_t AddListener(std::function<void(const EventType&)> callback) {
    auto callback_listener =
        std::make_unique<CallbackEventListener<EventType>>(std::move(callback));
    auto listener_ptr = callback_listener.get();

    // Store the callback listener first, then add it
    {
      std::lock_guard<std::mutex> lock(listeners_mutex_);
      callback_listeners_.emplace_back(std::move(callback_listener));
    }

    // Use the type-erased method to avoid infinite recursion
    return AddListener(TypedEvent<EventType>::GetStaticType(), listener_ptr);
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
    RemoveAllListeners(TypedEvent<EventType>::GetStaticType());
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
    return GetListenerCount(TypedEvent<EventType>::GetStaticType());
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

  /**
   * Start the background thread for asynchronous event processing.
   * This is called automatically when needed, but can be called explicitly.
   */
  void Start();

  /**
   * Stop the background thread and clear the event queue.
   */
  void Stop();

  /**
   * Check if the background thread is running.
   */
  bool IsRunning() const;

  /**
   * Emit an event synchronously to all registered listeners.
   * This is a public method for internal use by platform implementations.
   *
   * @param event The event to emit
   */
  void EmitSync(const Event& event);

 protected:

  /**
   * Emit an event synchronously using perfect forwarding.
   * This creates the event object and emits it immediately.
   */
  template <typename EventType, typename... Args>
  void EmitSync(Args&&... args) {
    EventType event(std::forward<Args>(args)...);
    EmitSync(event);
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
    auto event = std::make_unique<EventType>(std::forward<Args>(args)...);
    EmitAsync(std::move(event));
  }

 private:
  struct ListenerInfo {
    EventListener* listener;
    size_t id;
  };

  // Type-erased methods for internal use
  size_t AddListener(std::type_index event_type, EventListener* listener);
  void RemoveAllListeners(std::type_index event_type);
  size_t GetListenerCount(std::type_index event_type) const;

  // Background thread function for processing async events
  void ProcessAsyncEvents();

  // Member variables
  mutable std::mutex listeners_mutex_;
  std::unordered_map<std::type_index, std::vector<ListenerInfo>> listeners_;

  // Storage for callback listeners to manage their lifetime
  std::vector<std::unique_ptr<EventListener>> callback_listeners_;

  // Async event processing
  std::mutex queue_mutex_;
  std::queue<std::unique_ptr<Event>> event_queue_;
  std::condition_variable queue_condition_;
  std::thread worker_thread_;
  std::atomic<bool> running_;
  std::atomic<bool> stop_requested_;

  // Listener ID generation
  std::atomic<size_t> next_listener_id_;
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
