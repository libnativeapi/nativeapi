#pragma once

#include <functional>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <chrono>

namespace nativeapi {

/**
 * Base class for all events in the generic event system.
 * Events should inherit from this class and provide their own data.
 */
class Event {
 public:
  Event() : timestamp_(std::chrono::steady_clock::now()) {}
  virtual ~Event() = default;

  // Get the time when this event was created
  std::chrono::steady_clock::time_point GetTimestamp() const {
    return timestamp_;
  }

  // Get a string representation of the event type (for debugging)
  virtual std::string GetTypeName() const = 0;

 private:
  std::chrono::steady_clock::time_point timestamp_;
};

/**
 * Template for typed events. This provides type safety and automatic
 * type identification for events.
 */
template<typename T>
class TypedEvent : public Event {
 public:
  static std::type_index GetStaticType() {
    return std::type_index(typeid(T));
  }

  std::type_index GetType() const {
    return GetStaticType();
  }

  std::string GetTypeName() const override {
    return typeid(T).name();
  }
};

/**
 * Generic event listener interface that can handle any event type.
 * This is the base interface for the observer pattern.
 */
class EventListener {
 public:
  virtual ~EventListener() = default;
  
  /**
   * Handle an event. Implementations should check the event type
   * and cast appropriately.
   */
  virtual void OnEvent(const Event& event) = 0;
};

/**
 * Template for typed event listeners. This provides type safety
 * by automatically casting events to the correct type.
 */
template<typename EventType>
class TypedEventListener : public EventListener {
 public:
  virtual ~TypedEventListener() = default;

  void OnEvent(const Event& event) override {
    // Check if this is the correct event type
    if (auto typed_event = dynamic_cast<const EventType*>(&event)) {
      OnTypedEvent(*typed_event);
    }
  }

  /**
   * Handle a typed event. Subclasses should override this method.
   */
  virtual void OnTypedEvent(const EventType& event) = 0;
};

/**
 * Callback-based event handler that wraps std::function callbacks.
 * This allows using lambda functions or function pointers as event handlers.
 */
template<typename EventType>
class CallbackEventListener : public TypedEventListener<EventType> {
 public:
  using CallbackType = std::function<void(const EventType&)>;

  explicit CallbackEventListener(CallbackType callback) 
      : callback_(std::move(callback)) {}

  void OnTypedEvent(const EventType& event) override {
    if (callback_) {
      callback_(event);
    }
  }

 private:
  CallbackType callback_;
};

}  // namespace nativeapi