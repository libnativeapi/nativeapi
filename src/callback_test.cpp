#include <iostream>
#include <cassert>

#include "event_system.h"

// Simple test event
class SimpleEvent : public nativeapi::TypedEvent<SimpleEvent> {
 public:
  explicit SimpleEvent(int value) : value_(value) {}
  int GetValue() const { return value_; }
 private:
  int value_;
};

int main() {
  std::cout << "=== Callback Test ===" << std::endl;
  
  nativeapi::EventDispatcher dispatcher;
  
  // Test callback
  bool callback_called = false;
  int callback_value = 0;
  
  std::cout << "Adding callback listener..." << std::endl;
  auto callback_id = dispatcher.AddListener<SimpleEvent>([&](const SimpleEvent& event) {
    std::cout << "Callback called with value: " << event.GetValue() << std::endl;
    callback_value = event.GetValue();
    callback_called = true;
  });
  
  std::cout << "Callback ID: " << callback_id << std::endl;
  
  SimpleEvent event(123);
  std::cout << "Dispatching event..." << std::endl;
  dispatcher.DispatchSync(event);
  
  std::cout << "Callback called: " << callback_called << std::endl;
  std::cout << "Callback value: " << callback_value << std::endl;
  
  if (callback_called && callback_value == 123) {
    std::cout << "✓ Test passed!" << std::endl;
  } else {
    std::cout << "✗ Test failed!" << std::endl;
    return 1;
  }
  
  return 0;
}