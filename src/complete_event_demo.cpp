#include "event_system.h"
#include "common_events.h"
#include "event_integration_example.h"
#include <iostream>
#include <thread>
#include <chrono>

/**
 * Complete demonstration of the generic event system capabilities
 */
namespace nativeapi {

void DemonstrateGenericEventSystem() {
  std::cout << "=== Generic Event System Demonstration ===" << std::endl;
  
  // Get global event dispatcher
  auto& dispatcher = GetGlobalEventDispatcher();
  
  std::cout << "\n1. Setting up event listeners..." << std::endl;
  
  // Example 1: Type-specific callback listeners
  auto key_listener = AddScopedListener<KeyPressedEvent>(dispatcher, 
      [](const KeyPressedEvent& event) {
        std::cout << "🎹 Key pressed: " << event.GetKeycode() << std::endl;
      });
  
  auto display_listener = AddScopedListener<DisplayAddedEvent>(dispatcher,
      [](const DisplayAddedEvent& event) {
        std::cout << "🖥️ Display added" << std::endl;
      });
  
  // Example 2: Multi-event observer
  UnifiedSystemEventHandler system_handler;
  auto key_pressed_id = dispatcher.AddListener<KeyPressedEvent>(&system_handler);
  auto key_released_id = dispatcher.AddListener<KeyReleasedEvent>(&system_handler);
  auto display_added_id = dispatcher.AddListener<DisplayAddedEvent>(&system_handler);
  auto display_removed_id = dispatcher.AddListener<DisplayRemovedEvent>(&system_handler);
  auto app_started_id = dispatcher.AddListener<ApplicationStartedEvent>(&system_handler);
  
  // Example 3: Application lifecycle listeners
  auto app_exit_listener = AddScopedListener<ApplicationExitingEvent>(dispatcher,
      [](const ApplicationExitingEvent& event) {
        std::cout << "🚪 Application exiting with code: " << event.GetExitCode() << std::endl;
      });
  
  std::cout << "Total listeners registered: " << dispatcher.GetTotalListenerCount() << std::endl;
  
  // Example 4: Start async processing
  std::cout << "\n2. Starting asynchronous event processing..." << std::endl;
  dispatcher.Start();
  
  // Example 5: Synchronous event dispatch
  std::cout << "\n3. Dispatching synchronous events..." << std::endl;
  
  dispatcher.DispatchSync<ApplicationStartedEvent>();
  
  // Create dummy display for demonstration
  Display dummy_display; // Assuming Display has default constructor
  dispatcher.DispatchSync<DisplayAddedEvent>(dummy_display);
  
  dispatcher.DispatchSync<KeyPressedEvent>(65);  // 'A'
  dispatcher.DispatchSync<KeyReleasedEvent>(65);
  
  // Example 6: Asynchronous event dispatch
  std::cout << "\n4. Dispatching asynchronous events..." << std::endl;
  
  dispatcher.DispatchAsync<KeyPressedEvent>(66);  // 'B'
  dispatcher.DispatchAsync<KeyPressedEvent>(67);  // 'C'
  dispatcher.DispatchAsync<KeyReleasedEvent>(66);
  dispatcher.DispatchAsync<KeyReleasedEvent>(67);
  
  dispatcher.DispatchAsync<DisplayRemovedEvent>(dummy_display);
  
  // Wait for async events to process
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  
  // Example 7: Custom event with complex data
  std::cout << "\n5. Custom complex event..." << std::endl;
  
  class CustomUserEvent : public TypedEvent<CustomUserEvent> {
   public:
    CustomUserEvent(const std::string& action, int user_id, bool success)
        : action_(action), user_id_(user_id), success_(success) {}
    
    const std::string& GetAction() const { return action_; }
    int GetUserId() const { return user_id_; }
    bool IsSuccess() const { return success_; }
    
   private:
    std::string action_;
    int user_id_;
    bool success_;
  };
  
  auto custom_listener = AddScopedListener<CustomUserEvent>(dispatcher,
      [](const CustomUserEvent& event) {
        std::cout << "👤 User " << event.GetUserId() 
                  << " " << event.GetAction()
                  << " - " << (event.IsSuccess() ? "success" : "failed") << std::endl;
      });
  
  dispatcher.DispatchSync<CustomUserEvent>("login", 123, true);
  dispatcher.DispatchAsync<CustomUserEvent>("logout", 123, true);
  
  // Wait for final async event
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  
  // Example 8: Event timestamping
  std::cout << "\n6. Event timestamping demonstration..." << std::endl;
  
  auto timestamp_listener = AddScopedListener<ApplicationExitingEvent>(dispatcher,
      [](const ApplicationExitingEvent& event) {
        auto now = std::chrono::steady_clock::now();
        auto event_time = event.GetTimestamp();
        auto diff = std::chrono::duration_cast<std::chrono::microseconds>(now - event_time);
        std::cout << "⏱️ Event processed " << diff.count() << " microseconds after creation" << std::endl;
      });
  
  dispatcher.DispatchSync<ApplicationExitingEvent>(0);
  
  // Example 9: Cleanup demonstration
  std::cout << "\n7. Cleanup and listener management..." << std::endl;
  
  std::cout << "Listeners before cleanup: " << dispatcher.GetTotalListenerCount() << std::endl;
  
  // Remove some specific listeners
  dispatcher.RemoveListener(key_pressed_id);
  dispatcher.RemoveListener(display_added_id);
  
  std::cout << "Listeners after removing 2: " << dispatcher.GetTotalListenerCount() << std::endl;
  
  // Scoped listeners will be automatically removed when they go out of scope
  
  // Stop async processing
  std::cout << "\n8. Stopping async processing..." << std::endl;
  dispatcher.Stop();
  
  std::cout << "\n=== Demonstration Complete ===" << std::endl;
  std::cout << "The generic event system provides:" << std::endl;
  std::cout << "✓ Type-safe event handling" << std::endl;
  std::cout << "✓ Both observer and callback patterns" << std::endl;
  std::cout << "✓ Synchronous and asynchronous dispatch" << std::endl;
  std::cout << "✓ Thread-safe operations" << std::endl;
  std::cout << "✓ Automatic memory management" << std::endl;
  std::cout << "✓ Event timestamping" << std::endl;
  std::cout << "✓ Exception safety" << std::endl;
  std::cout << "✓ RAII-based cleanup" << std::endl;
  
  // Clean up remaining listeners
  dispatcher.RemoveListener(key_released_id);
  dispatcher.RemoveListener(display_removed_id);
  dispatcher.RemoveListener(app_started_id);
  
  std::cout << "\nFinal listener count: " << dispatcher.GetTotalListenerCount() << std::endl;
}

}  // namespace nativeapi

#ifdef DEMO_MAIN
int main() {
  nativeapi::DemonstrateGenericEventSystem();
  return 0;
}
#endif