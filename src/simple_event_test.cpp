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
  std::cout << "=== Simple Event Test ===" << std::endl;
  
  nativeapi::EventDispatcher dispatcher;
  
  // Test 1: Observer pattern
  class SimpleObserver : public nativeapi::TypedEventListener<SimpleEvent> {
   public:
    SimpleObserver() : called_(false), value_(0) {}
    
    void OnTypedEvent(const SimpleEvent& event) override {
      std::cout << "Observer called with value: " << event.GetValue() << std::endl;
      value_ = event.GetValue();
      called_ = true;
    }
    
    bool IsCalled() const { return called_; }
    int GetValue() const { return value_; }
    
   private:
    bool called_;
    int value_;
  };
  
  SimpleObserver observer;
  auto listener_id = dispatcher.AddListener<SimpleEvent>(&observer);
  std::cout << "Listener ID: " << listener_id << std::endl;
  
  SimpleEvent event(42);
  std::cout << "Dispatching event..." << std::endl;
  dispatcher.DispatchSync(event);
  
  std::cout << "Observer called: " << observer.IsCalled() << std::endl;
  std::cout << "Observer value: " << observer.GetValue() << std::endl;
  
  assert(observer.IsCalled());
  assert(observer.GetValue() == 42);
  
  std::cout << "✓ Test passed!" << std::endl;
  return 0;
}