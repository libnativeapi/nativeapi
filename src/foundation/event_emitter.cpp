#include "event_emitter.h"

#include <algorithm>
#include <iostream>

namespace nativeapi {

EventEmitter::EventEmitter()
    : running_(false), stop_requested_(false), next_listener_id_(1) {}

EventEmitter::~EventEmitter() {
  StopAsyncProcessing();
}

size_t EventEmitter::AddListener(std::type_index event_type,
                                 std::unique_ptr<EventListenerBase> listener) {
  if (!listener) {
    return 0;  // Invalid listener
  }

  std::lock_guard<std::mutex> lock(listeners_mutex_);
  size_t listener_id = next_listener_id_.fetch_add(1);

  listeners_[event_type].push_back({std::move(listener), listener_id});

  return listener_id;
}

bool EventEmitter::RemoveListener(size_t listener_id) {
  std::lock_guard<std::mutex> lock(listeners_mutex_);

  for (auto& [event_type, listener_list] : listeners_) {
    auto it = std::find_if(listener_list.begin(), listener_list.end(),
                           [listener_id](const ListenerInfo& info) {
                             return info.id == listener_id;
                           });

    if (it != listener_list.end()) {
      listener_list.erase(it);
      return true;
    }
  }

  return false;
}

void EventEmitter::RemoveAllListeners(std::type_index event_type) {
  std::lock_guard<std::mutex> lock(listeners_mutex_);
  listeners_[event_type].clear();
}

void EventEmitter::RemoveAllListeners() {
  std::lock_guard<std::mutex> lock(listeners_mutex_);
  listeners_.clear();
}

void EventEmitter::Emit(const Event& event) {
  std::type_index event_type = typeid(event);
  std::vector<EventListenerBase*> listeners_copy;

  // Copy listeners to avoid holding the lock during dispatch
  {
    std::lock_guard<std::mutex> lock(listeners_mutex_);
    auto it = listeners_.find(event_type);
    if (it != listeners_.end()) {
      listeners_copy.reserve(it->second.size());
      for (const auto& info : it->second) {
        listeners_copy.push_back(info.listener.get());
      }
    }
  }

  // Dispatch to all listeners
  for (auto* listener : listeners_copy) {
    try {
      listener->OnEvent(event);
    } catch (const std::exception& e) {
      std::cerr << "Exception in event listener: " << e.what() << std::endl;
    } catch (...) {
      std::cerr << "Unknown exception in event listener" << std::endl;
    }
  }
}

void EventEmitter::EmitAsync(std::unique_ptr<Event> event) {
  if (!event) {
    return;
  }

  // Start the worker thread if not already running
  if (!running_.load()) {
    StartAsyncProcessing();
  }

  {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    event_queue_.push(std::move(event));
  }

  queue_condition_.notify_one();
}

void EventEmitter::StartAsyncProcessing() {
  if (running_.load()) {
    return;  // Already running
  }

  stop_requested_.store(false);
  running_.store(true);

  worker_thread_ = std::thread(&EventEmitter::ProcessAsyncEvents, this);
}

void EventEmitter::StopAsyncProcessing() {
  if (!running_.load()) {
    return;  // Not running
  }

  stop_requested_.store(true);
  queue_condition_.notify_all();

  if (worker_thread_.joinable()) {
    worker_thread_.join();
  }

  running_.store(false);

  // Clear any remaining events in the queue
  std::lock_guard<std::mutex> lock(queue_mutex_);
  while (!event_queue_.empty()) {
    event_queue_.pop();
  }
}

bool EventEmitter::IsAsyncProcessing() const {
  return running_.load();
}

size_t EventEmitter::GetListenerCount(std::type_index event_type) const {
  std::lock_guard<std::mutex> lock(listeners_mutex_);
  auto it = listeners_.find(event_type);
  return (it != listeners_.end()) ? it->second.size() : 0;
}

size_t EventEmitter::GetTotalListenerCount() const {
  std::lock_guard<std::mutex> lock(listeners_mutex_);
  size_t total = 0;
  for (const auto& [event_type, listener_list] : listeners_) {
    total += listener_list.size();
  }
  return total;
}

void EventEmitter::ProcessAsyncEvents() {
  while (running_.load()) {
    std::unique_ptr<Event> event;

    // Wait for an event or stop signal
    {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      queue_condition_.wait(lock, [this] {
        return !event_queue_.empty() || stop_requested_.load();
      });

      if (stop_requested_.load()) {
        break;
      }

      if (!event_queue_.empty()) {
        event = std::move(event_queue_.front());
        event_queue_.pop();
      }
    }

    // Dispatch the event if we have one
    if (event) {
      Emit(*event);
    }
  }
}

}  // namespace nativeapi
