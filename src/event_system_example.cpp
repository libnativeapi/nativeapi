#include "event_system.h"
#include "common_events.h"
#include <iostream>
#include <thread>
#include <chrono>

namespace nativeapi {

/**
 * Example demonstrating various ways to use the generic event system
 */

// Example 1: Observer pattern implementation
class DisplayEventObserver : public TypedEventListener<DisplayAddedEvent> {
 public:
  void OnTypedEvent(const DisplayAddedEvent& event) override {
    std::cout << "Observer: Display added at " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                   event.GetTimestamp().time_since_epoch()).count()
              << "ms" << std::endl;
  }
};

// Example 2: Multi-event observer
class KeyboardEventObserver : public EventListener {
 public:
  void OnEvent(const Event& event) override {
    if (auto key_pressed = dynamic_cast<const KeyPressedEvent*>(&event)) {
      OnKeyPressed(*key_pressed);
    } else if (auto key_released = dynamic_cast<const KeyReleasedEvent*>(&event)) {
      OnKeyReleased(*key_released);
    }
  }

 private:
  void OnKeyPressed(const KeyPressedEvent& event) {
    std::cout << "Observer: Key pressed - " << event.GetKeycode() << std::endl;
  }

  void OnKeyReleased(const KeyReleasedEvent& event) {
    std::cout << "Observer: Key released - " << event.GetKeycode() << std::endl;
  }
};

void RunEventSystemExample() {
  std::cout << "=== Generic Event System Example ===" << std::endl;

  // Get the global event dispatcher
  auto& dispatcher = GetGlobalEventDispatcher();

  // Example 1: Register an observer
  DisplayEventObserver display_observer;
  auto display_listener_id = dispatcher.AddListener<DisplayAddedEvent>(&display_observer);
  
  // Example 2: Register callback functions
  auto key_pressed_guard = AddScopedListener<KeyPressedEvent>(dispatcher,
      [](const KeyPressedEvent& event) {
        std::cout << "Lambda: Key pressed - " << event.GetKeycode() << std::endl;
      });

  auto key_released_guard = AddScopedListener<KeyReleasedEvent>(dispatcher,
      [](const KeyReleasedEvent& event) {
        std::cout << "Lambda: Key released - " << event.GetKeycode() << std::endl;
      });

  // Example 3: Register multiple event handler
  KeyboardEventObserver keyboard_observer;
  auto key_pressed_id2 = dispatcher.AddListener<KeyPressedEvent>(&keyboard_observer);
  auto key_released_id2 = dispatcher.AddListener<KeyReleasedEvent>(&keyboard_observer);

  // Example 4: Register application events
  auto app_started_guard = AddScopedListener<ApplicationStartedEvent>(dispatcher,
      [](const ApplicationStartedEvent& event) {
        std::cout << "Application started!" << std::endl;
      });

  auto app_exiting_guard = AddScopedListener<ApplicationExitingEvent>(dispatcher,
      [](const ApplicationExitingEvent& event) {
        std::cout << "Application exiting with code: " << event.GetExitCode() << std::endl;
      });

  std::cout << "Total listeners registered: " << dispatcher.GetTotalListenerCount() << std::endl;
  std::cout << "KeyPressed listeners: " << dispatcher.GetListenerCount<KeyPressedEvent>() << std::endl;

  // Start async processing
  dispatcher.Start();

  // Example 5: Dispatch some synchronous events
  std::cout << "\n--- Synchronous Events ---" << std::endl;
  
  // Create a dummy display (this would normally come from the system)
  Display dummy_display; // Assuming Display has a default constructor
  dispatcher.DispatchSync<DisplayAddedEvent>(dummy_display);
  
  dispatcher.DispatchSync<KeyPressedEvent>(65); // 'A' key
  dispatcher.DispatchSync<KeyReleasedEvent>(65);
  
  dispatcher.DispatchSync<ApplicationStartedEvent>();

  // Example 6: Dispatch some asynchronous events
  std::cout << "\n--- Asynchronous Events ---" << std::endl;
  
  dispatcher.DispatchAsync<KeyPressedEvent>(66); // 'B' key
  dispatcher.DispatchAsync<KeyReleasedEvent>(66);
  
  dispatcher.DispatchAsync<ApplicationExitingEvent>(0);

  // Wait a bit for async events to process
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Example 7: Remove specific listeners
  dispatcher.RemoveListener(display_listener_id);
  dispatcher.RemoveListener(key_pressed_id2);
  dispatcher.RemoveListener(key_released_id2);

  std::cout << "\nAfter removing some listeners: " << dispatcher.GetTotalListenerCount() << std::endl;

  // Dispatch one more event to show remaining listeners
  dispatcher.DispatchSync<KeyPressedEvent>(67); // 'C' key

  // Stop async processing
  dispatcher.Stop();

  std::cout << "\n=== Example Complete ===" << std::endl;
}

}  // namespace nativeapi

// If building as a standalone example
#ifdef EVENT_SYSTEM_EXAMPLE_MAIN
int main() {
  nativeapi::RunEventSystemExample();
  return 0;
}
#endif