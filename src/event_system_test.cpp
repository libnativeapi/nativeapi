#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>
#include <atomic>

#include "../src/event_system.h"

// Simple test events
class TestEvent : public nativeapi::TypedEvent<TestEvent> {
 public:
  explicit TestEvent(int value) : value_(value) {}
  int GetValue() const { return value_; }
 private:
  int value_;
};

class AnotherTestEvent : public nativeapi::TypedEvent<AnotherTestEvent> {
 public:
  explicit AnotherTestEvent(const std::string& message) : message_(message) {}
  const std::string& GetMessage() const { return message_; }
 private:
  std::string message_;
};

// Test observer
class TestObserver : public nativeapi::TypedEventListener<TestEvent> {
 public:
  TestObserver() : event_count_(0) {}
  
  void OnTypedEvent(const TestEvent& event) override {
    last_value_ = event.GetValue();
    event_count_++;
  }
  
  int GetLastValue() const { return last_value_; }
  int GetEventCount() const { return event_count_; }
  
 private:
  int last_value_ = 0;
  std::atomic<int> event_count_;
};

int main() {
  std::cout << "=== Event System Test ===" << std::endl;
  
  nativeapi::EventDispatcher dispatcher;
  
  // Test 1: Basic observer pattern
  std::cout << "Test 1: Basic observer pattern" << std::endl;
  TestObserver observer;
  auto listener_id = dispatcher.AddListener<TestEvent>(&observer);
  
  dispatcher.DispatchSync<TestEvent>(42);
  assert(observer.GetLastValue() == 42);
  assert(observer.GetEventCount() == 1);
  std::cout << "✓ Observer received event with value: " << observer.GetLastValue() << std::endl;
  
  // Test 2: Callback pattern
  std::cout << "Test 2: Callback pattern" << std::endl;
  std::atomic<bool> callback_called(false);
  std::atomic<int> callback_value(0);
  
  auto callback_id = dispatcher.AddListener<TestEvent>(
      [&](const TestEvent& event) {
        callback_value = event.GetValue();
        callback_called = true;
      });
  
  dispatcher.DispatchSync<TestEvent>(123);
  assert(callback_called.load());
  assert(callback_value.load() == 123);
  assert(observer.GetEventCount() == 2); // Observer should also get this event
  std::cout << "✓ Callback received event with value: " << callback_value.load() << std::endl;
  
  // Test 3: Multiple event types
  std::cout << "Test 3: Multiple event types" << std::endl;
  std::atomic<bool> another_callback_called(false);
  std::string received_message;
  
  auto another_callback_id = dispatcher.AddListener<AnotherTestEvent>(
      [&](const AnotherTestEvent& event) {
        received_message = event.GetMessage();
        another_callback_called = true;
      });
  
  dispatcher.DispatchSync<AnotherTestEvent>("Hello World");
  assert(another_callback_called.load());
  assert(received_message == "Hello World");
  std::cout << "✓ Another callback received message: " << received_message << std::endl;
  
  // Test 4: Async dispatch
  std::cout << "Test 4: Asynchronous dispatch" << std::endl;
  dispatcher.Start();
  
  std::atomic<bool> async_callback_called(false);
  auto async_callback_id = dispatcher.AddListener<TestEvent>(
      [&](const TestEvent& event) {
        async_callback_called = true;
      });
  
  dispatcher.DispatchAsync<TestEvent>(999);
  
  // Wait for async event to be processed
  auto timeout = std::chrono::steady_clock::now() + std::chrono::seconds(1);
  while (!async_callback_called.load() && std::chrono::steady_clock::now() < timeout) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  
  assert(async_callback_called.load());
  std::cout << "✓ Async event processed successfully" << std::endl;
  
  // Test 5: Listener removal
  std::cout << "Test 5: Listener removal" << std::endl;
  int initial_count = observer.GetEventCount();
  
  assert(dispatcher.RemoveListener(callback_id));
  dispatcher.DispatchSync<TestEvent>(777);
  
  // Observer should get the event, but the removed callback should not
  assert(observer.GetEventCount() == initial_count + 1);
  std::cout << "✓ Listener removed successfully" << std::endl;
  
  // Test 6: Listener count
  std::cout << "Test 6: Listener count" << std::endl;
  size_t test_event_listeners = dispatcher.GetListenerCount<TestEvent>();
  size_t total_listeners = dispatcher.GetTotalListenerCount();
  std::cout << "✓ TestEvent listeners: " << test_event_listeners << std::endl;
  std::cout << "✓ Total listeners: " << total_listeners << std::endl;
  
  // Test 7: Scoped listener (RAII)
  std::cout << "Test 7: Scoped listener (RAII)" << std::endl;
  std::atomic<bool> scoped_callback_called(false);
  {
    auto scoped_guard = nativeapi::AddScopedListener<TestEvent>(dispatcher,
        [&](const TestEvent& event) {
          scoped_callback_called = true;
        });
    
    dispatcher.DispatchSync<TestEvent>(555);
    assert(scoped_callback_called.load());
    std::cout << "✓ Scoped listener called" << std::endl;
  }
  // Scoped listener should be automatically removed here
  
  scoped_callback_called = false;
  dispatcher.DispatchSync<TestEvent>(666);
  assert(!scoped_callback_called.load());
  std::cout << "✓ Scoped listener automatically removed" << std::endl;
  
  // Cleanup
  dispatcher.Stop();
  dispatcher.RemoveAllListeners();
  
  std::cout << "\n=== All Tests Passed! ===" << std::endl;
  return 0;
}