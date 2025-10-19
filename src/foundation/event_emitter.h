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
 *     Emit<MyEvent>("some data");
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
  size_t AddListener(EventListener<EventType>* listener) {
    // Create a wrapper that handles the type conversion
    struct TypedListenerWrapper : public EventListenerBase {
      EventListener<EventType>* listener_;

      TypedListenerWrapper(EventListener<EventType>* listener) : listener_(listener) {}

      void OnEvent(const Event& event) override {
        if (auto typed_event = dynamic_cast<const EventType*>(&event)) {
          listener_->OnEvent(*typed_event);
        }
      }
    };

    return AddListener(typeid(EventType), std::make_unique<TypedListenerWrapper>(listener));
  }

  /**
   * Add a callback function as a listener for a specific event type.
   *
   * @param callback Function to call when the event occurs
   * @return A unique listener ID that can be used to remove the listener
   */
  template <typename EventType>
  size_t AddListener(std::function<void(const EventType&)> callback) {
    // Create a wrapper that handles the callback and type conversion
    struct CallbackListenerWrapper : public EventListenerBase {
      std::function<void(const EventType&)> callback_;

      CallbackListenerWrapper(std::function<void(const EventType&)> callback)
          : callback_(std::move(callback)) {}

      void OnEvent(const Event& event) override {
        if (auto typed_event = dynamic_cast<const EventType*>(&event)) {
          callback_(*typed_event);
        }
      }
    };

    return AddListener(typeid(EventType),
                       std::make_unique<CallbackListenerWrapper>(std::move(callback)));
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
    RemoveAllListeners(typeid(EventType));
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
    return GetListenerCount(typeid(EventType));
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
  void Emit(const Event& event);

 protected:
  /**
   * Emit an event synchronously using perfect forwarding.
   * This creates the event object and emits it immediately.
   */
  template <typename EventType, typename... Args>
  void Emit(Args&&... args) {
    EventType event(std::forward<Args>(args)...);
    Emit(event);
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
  // Base interface for type-erased listeners
  struct EventListenerBase {
    virtual ~EventListenerBase() = default;
    virtual void OnEvent(const Event& event) = 0;
  };

  struct ListenerInfo {
    std::unique_ptr<EventListenerBase> listener;
    size_t id;
  };

  // Type-erased methods for internal use
  size_t AddListener(std::type_index event_type, std::unique_ptr<EventListenerBase> listener);
  void RemoveAllListeners(std::type_index event_type);
  size_t GetListenerCount(std::type_index event_type) const;

  // Background thread function for processing async events
  void ProcessAsyncEvents();

  // Member variables
  mutable std::mutex listeners_mutex_;
  std::unordered_map<std::type_index, std::vector<ListenerInfo>> listeners_;

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

}  // namespace nativeapi
