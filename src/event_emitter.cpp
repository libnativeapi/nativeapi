#include "event_emitter.h"

namespace nativeapi {

EventEmitter::EventEmitter() = default;

EventEmitter::~EventEmitter() = default;

bool EventEmitter::RemoveListener(size_t listener_id) {
  return dispatcher_.RemoveListener(listener_id);
}

void EventEmitter::RemoveAllListeners() {
  dispatcher_.RemoveAllListeners();
}

size_t EventEmitter::GetTotalListenerCount() const {
  return dispatcher_.GetTotalListenerCount();
}

void EventEmitter::EmitSync(const Event& event) {
  dispatcher_.DispatchSync(event);
}

void EventEmitter::EmitAsync(std::unique_ptr<Event> event) {
  dispatcher_.DispatchAsync(std::move(event));
}

EventDispatcher& EventEmitter::GetEventDispatcher() {
  return dispatcher_;
}

const EventDispatcher& EventEmitter::GetEventDispatcher() const {
  return dispatcher_;
}

}  // namespace nativeapi