#pragma once

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "event.h"

namespace nativeapi {

/**
 * Base class that provides event emission capabilities with type constraints.
 * Classes that inherit from EventEmitter must specify the base event type they work with,
 * and then only emit events of that type or its subclasses. This provides compile-time
 * type safety and makes the API more explicit about what events a class can produce.
 *
 * Template Parameters:
 *   BaseEventType - The base event type this emitter can handle. All emitted events
 *                   must be of this type or inherit from it.
 *
 * Example usage:
 *
 * // Define your event hierarchy
 * class MyEvent : public Event {
 * public:
 *   std::string data;
 *   MyEvent(std::string d) : data(std::move(d)) {}
 *   std::string GetTypeName() const override { return "MyEvent"; }
 * };
 *
 * class MySpecificEvent : public MyEvent {
 * public:
 *   int value;
 *   MySpecificEvent(std::string d, int v) : MyEvent(std::move(d)), value(v) {}
 *   std::string GetTypeName() const override { return "MySpecificEvent"; }
 * };
 *
 * // Create a class that only emits MyEvent and its subclasses
 * class MyClass : public EventEmitter<MyEvent> {
 * public:
 *   void DoSomething() {
 *     // Emit a MyEvent - OK
 *     Emit<MyEvent>("some data");
 *
 *     // Emit a MySpecificEvent (subclass of MyEvent) - OK
 *     EmitAsync<MySpecificEvent>("data", 42);
 *
 *     // Emit<SomeOtherEvent>() - Compile error! Not a subclass of MyEvent
 *   }
 * };
 *
 * MyClass obj;
 * // Listen for base event type
 * obj.AddListener<MyEvent>([](const MyEvent& event) {
 *   std::cout << "MyEvent: " << event.data << std::endl;
 * });
 *
 * // Listen for specific event type
 * obj.AddListener<MySpecificEvent>([](const MySpecificEvent& event) {
 *   std::cout << "MySpecificEvent: " << event.data << ", " << event.value << std::endl;
 * });
 */
template <typename BaseEventType>
class EventEmitter {
  // Ensure BaseEventType is derived from Event
  static_assert(std::is_base_of<Event, BaseEventType>::value,
                "BaseEventType must be derived from Event");

 public:
  EventEmitter() : running_(false), stop_requested_(false), next_listener_id_(1) {}

  virtual ~EventEmitter() { StopAsyncProcessing(); }

  /**
   * Add a typed event listener for a specific event type.
   * The event type must be BaseEventType or a subclass of it.
   *
   * @param listener Pointer to the event listener (must remain valid until removed)
   * @return A unique listener ID that can be used to remove the listener
   */
  template <typename EventType>
  size_t AddListener(EventListener<EventType>* listener) {
    static_assert(std::is_base_of<BaseEventType, EventType>::value,
                  "EventType must be derived from the EventEmitter's BaseEventType");

    // Create a wrapper that handles the type conversion
    struct TypedListenerWrapper : public EventListenerBase {
      EventListener<EventType>* listener_;

      TypedListenerWrapper(EventListener<EventType>* listener) : listener_(listener) {}

      void OnEvent(const BaseEventType& event) override {
        if (auto typed_event = dynamic_cast<const EventType*>(&event)) {
          listener_->OnEvent(*typed_event);
        }
      }
    };

    return AddListener(typeid(EventType), std::make_unique<TypedListenerWrapper>(listener));
  }

  /**
   * Add a callback function as a listener for a specific event type.
   * The event type must be BaseEventType or a subclass of it.
   *
   * @param callback Function to call when the event occurs
   * @return A unique listener ID that can be used to remove the listener
   */
  template <typename EventType>
  size_t AddListener(std::function<void(const EventType&)> callback) {
    static_assert(std::is_base_of<BaseEventType, EventType>::value,
                  "EventType must be derived from the EventEmitter's BaseEventType");

    // Create a wrapper that handles the callback and type conversion
    struct CallbackListenerWrapper : public EventListenerBase {
      std::function<void(const EventType&)> callback_;

      CallbackListenerWrapper(std::function<void(const EventType&)> callback)
          : callback_(std::move(callback)) {}

      void OnEvent(const BaseEventType& event) override {
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
  bool RemoveListener(size_t listener_id) {
    std::lock_guard<std::mutex> lock(listeners_mutex_);

    for (auto& [type, listeners] : listeners_) {
      auto it =
          std::find_if(listeners.begin(), listeners.end(),
                       [listener_id](const ListenerInfo& info) { return info.id == listener_id; });

      if (it != listeners.end()) {
        listeners.erase(it);

        // Clean up empty vector
        if (listeners.empty()) {
          listeners_.erase(type);
        }

        // Check if this was the last listener
        if (GetTotalListenerCountUnlocked() == 0) {
          StopEventListening();
        }

        return true;
      }
    }

    return false;
  }

  /**
   * Remove all listeners for a specific event type.
   * The event type must be BaseEventType or a subclass of it.
   */
  template <typename EventType>
  void RemoveAllListeners() {
    static_assert(std::is_base_of<BaseEventType, EventType>::value,
                  "EventType must be derived from the EventEmitter's BaseEventType");
    RemoveAllListeners(typeid(EventType));
  }

  /**
   * Remove all listeners for all event types.
   */
  void RemoveAllListeners() {
    std::lock_guard<std::mutex> lock(listeners_mutex_);

    bool had_listeners = GetTotalListenerCountUnlocked() > 0;

    listeners_.clear();

    if (had_listeners) {
      StopEventListening();
    }
  }

  /**
   * Get the number of listeners registered for a specific event type.
   * The event type must be BaseEventType or a subclass of it.
   */
  template <typename EventType>
  size_t GetListenerCount() const {
    static_assert(std::is_base_of<BaseEventType, EventType>::value,
                  "EventType must be derived from the EventEmitter's BaseEventType");
    return GetListenerCount(typeid(EventType));
  }

  /**
   * Get the total number of registered listeners.
   */
  size_t GetTotalListenerCount() const {
    std::lock_guard<std::mutex> lock(listeners_mutex_);
    return GetTotalListenerCountUnlocked();
  }

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
  void StartAsyncProcessing() {
    std::lock_guard<std::mutex> lock(queue_mutex_);

    if (!running_.load()) {
      stop_requested_.store(false);
      running_.store(true);
      worker_thread_ = std::thread(&EventEmitter::ProcessAsyncEvents, this);
    }
  }

  /**
   * Stop the background thread and clear the event queue.
   */
  void StopAsyncProcessing() {
    {
      std::lock_guard<std::mutex> lock(queue_mutex_);
      if (!running_.load()) {
        return;
      }
      stop_requested_.store(true);
    }

    queue_condition_.notify_all();

    if (worker_thread_.joinable()) {
      worker_thread_.join();
    }

    running_.store(false);

    // Clear any remaining events
    std::lock_guard<std::mutex> lock(queue_mutex_);
    while (!event_queue_.empty()) {
      event_queue_.pop();
    }
  }

  /**
   * Check if the background thread is running.
   */
  bool IsAsyncProcessing() const { return running_.load(); }

  /**
   * Emit an event synchronously to all registered listeners.
   * This is a public method for internal use by platform implementations.
   * The event must be of BaseEventType or a subclass.
   *
   * @param event The event to emit
   */
  void Emit(const BaseEventType& event) {
    std::lock_guard<std::mutex> lock(listeners_mutex_);

    // Find listeners for the exact event type
    auto it = listeners_.find(typeid(event));
    if (it != listeners_.end()) {
      for (const auto& listener_info : it->second) {
        listener_info.listener->OnEvent(event);
      }
    }

    // Also notify listeners registered for base types
    // This allows a listener for BaseEvent to receive DerivedEvent
    for (auto& [type, listener_list] : listeners_) {
      if (type == typeid(event)) {
        continue;  // Already handled above
      }

      // Try to dispatch to listeners of potential base types
      for (const auto& listener_info : listener_list) {
        listener_info.listener->OnEvent(event);
      }
    }
  }

 protected:
  /**
   * Called when the first listener is added.
   * Subclasses can override this to start platform-specific event monitoring.
   * This is called while holding the listeners_mutex_ lock.
   */
  virtual void StartEventListening() {}

  /**
   * Called when the last listener is removed.
   * Subclasses can override this to stop platform-specific event monitoring.
   * This is called while holding the listeners_mutex_ lock.
   */
  virtual void StopEventListening() {}

  /**
   * Emit an event synchronously using perfect forwarding.
   * This creates the event object and emits it immediately.
   * The event type must be BaseEventType or a subclass of it.
   */
  template <typename EventType, typename... Args>
  void Emit(Args&&... args) {
    static_assert(std::is_base_of<BaseEventType, EventType>::value,
                  "EventType must be derived from the EventEmitter's BaseEventType");

    EventType event(std::forward<Args>(args)...);
    Emit(event);
  }

  /**
   * Emit an event asynchronously.
   * The event will be dispatched on a background thread.
   * The event must be of BaseEventType or a subclass.
   *
   * @param event The event to emit (will be moved)
   */
  void EmitAsync(std::unique_ptr<BaseEventType> event) {
    {
      std::lock_guard<std::mutex> lock(queue_mutex_);

      if (!running_.load()) {
        StartAsyncProcessing();
      }

      event_queue_.push(std::move(event));
    }

    queue_condition_.notify_one();
  }

  /**
   * Emit an event asynchronously using perfect forwarding.
   * The event type must be BaseEventType or a subclass of it.
   */
  template <typename EventType, typename... Args>
  void EmitAsync(Args&&... args) {
    static_assert(std::is_base_of<BaseEventType, EventType>::value,
                  "EventType must be derived from the EventEmitter's BaseEventType");

    auto event = std::make_unique<EventType>(std::forward<Args>(args)...);
    EmitAsync(std::move(event));
  }

 private:
  // Base interface for type-erased listeners
  struct EventListenerBase {
    virtual ~EventListenerBase() = default;
    virtual void OnEvent(const BaseEventType& event) = 0;
  };

  struct ListenerInfo {
    std::unique_ptr<EventListenerBase> listener;
    size_t id;
  };

  // Type-erased methods for internal use
  size_t AddListener(std::type_index event_type, std::unique_ptr<EventListenerBase> listener) {
    std::lock_guard<std::mutex> lock(listeners_mutex_);

    bool was_empty = GetTotalListenerCountUnlocked() == 0;

    size_t listener_id = next_listener_id_.fetch_add(1);
    listeners_[event_type].push_back({std::move(listener), listener_id});

    if (was_empty) {
      StartEventListening();
    }

    return listener_id;
  }

  void RemoveAllListeners(std::type_index event_type) {
    std::lock_guard<std::mutex> lock(listeners_mutex_);

    auto it = listeners_.find(event_type);
    if (it != listeners_.end()) {
      listeners_.erase(it);

      // Check if this was the last listener
      if (GetTotalListenerCountUnlocked() == 0) {
        StopEventListening();
      }
    }
  }

  size_t GetListenerCount(std::type_index event_type) const {
    std::lock_guard<std::mutex> lock(listeners_mutex_);

    auto it = listeners_.find(event_type);
    if (it != listeners_.end()) {
      return it->second.size();
    }

    return 0;
  }

  size_t GetTotalListenerCountUnlocked() const {
    size_t count = 0;
    for (const auto& [type, listeners] : listeners_) {
      count += listeners.size();
    }
    return count;
  }

  // Background thread function for processing async events
  void ProcessAsyncEvents() {
    while (true) {
      std::unique_ptr<BaseEventType> event;

      {
        std::unique_lock<std::mutex> lock(queue_mutex_);

        queue_condition_.wait(lock,
                              [this] { return stop_requested_.load() || !event_queue_.empty(); });

        if (stop_requested_.load()) {
          break;
        }

        if (!event_queue_.empty()) {
          event = std::move(event_queue_.front());
          event_queue_.pop();
        }
      }

      if (event) {
        Emit(*event);
      }
    }
  }

  // Member variables
  mutable std::mutex listeners_mutex_;
  std::unordered_map<std::type_index, std::vector<ListenerInfo>> listeners_;

  // Async event processing
  std::mutex queue_mutex_;
  std::queue<std::unique_ptr<BaseEventType>> event_queue_;
  std::condition_variable queue_condition_;
  std::thread worker_thread_;
  std::atomic<bool> running_;
  std::atomic<bool> stop_requested_;

  // Listener ID generation
  std::atomic<size_t> next_listener_id_;
};

}  // namespace nativeapi