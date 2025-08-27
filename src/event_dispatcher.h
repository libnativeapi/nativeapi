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
 * Generic event dispatcher that supports both synchronous and asynchronous
 * event dispatching. It maintains listeners for different event types and
 * can dispatch events either immediately or queue them for later processing.
 */
class EventDispatcher {
 public:
  EventDispatcher();
  ~EventDispatcher();

  /**
   * Add a listener for a specific event type.
   * The listener will be called whenever an event of that type is dispatched.
   *
   * @param listener Pointer to the event listener (must remain valid until
   * removed)
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
   * Remove all listeners.
   */
  void RemoveAllListeners();

  /**
   * Dispatch an event synchronously to all registered listeners.
   * This will call all listeners immediately on the current thread.
   *
   * @param event The event to dispatch
   */
  void DispatchSync(const Event& event);

  /**
   * Dispatch an event synchronously using perfect forwarding.
   * This creates the event object and dispatches it immediately.
   */
  template <typename EventType, typename... Args>
  void DispatchSync(Args&&... args) {
    EventType event(std::forward<Args>(args)...);
    DispatchSync(event);
  }

  /**
   * Queue an event for asynchronous dispatch.
   * The event will be dispatched on the background thread.
   *
   * @param event The event to queue (will be copied)
   */
  void DispatchAsync(std::unique_ptr<Event> event);

  /**
   * Queue an event for asynchronous dispatch using perfect forwarding.
   */
  template <typename EventType, typename... Args>
  void DispatchAsync(Args&&... args) {
    auto event = std::make_unique<EventType>(std::forward<Args>(args)...);
    DispatchAsync(std::move(event));
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
 * Convenience function to create a global event dispatcher instance.
 * This is useful for applications that need a single, shared event system.
 */
EventDispatcher& GetGlobalEventDispatcher();
}  // namespace nativeapi
