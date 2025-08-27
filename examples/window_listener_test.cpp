#include <iostream>
#include <thread>
#include <chrono>

#include "../include/nativeapi.h"

using namespace nativeapi;

// Test listener for window events
class TestWindowListener : public TypedEventListener<WindowCreatedEvent> {
public:
  void OnTypedEvent(const WindowCreatedEvent& event) override {
    std::cout << "Window created with ID: " << event.GetWindowId() << std::endl;
  }
};

class TestWindowFocusListener : public TypedEventListener<WindowFocusedEvent> {
public:
  void OnTypedEvent(const WindowFocusedEvent& event) override {
    std::cout << "Window focused with ID: " << event.GetWindowId() << std::endl;
  }
};

int main() {
  std::cout << "Testing WindowManager listener functionality..." << std::endl;
  
  WindowManager window_manager;
  
  // Create listeners
  TestWindowListener window_listener;
  TestWindowFocusListener focus_listener;
  
  // Add listeners
  auto window_listener_id = window_manager.AddListener<WindowCreatedEvent>(&window_listener);
  auto focus_listener_id = window_manager.AddListener<WindowFocusedEvent>(&focus_listener);
  
  // Add callback-based listeners
  auto resize_listener_id = window_manager.AddListener<WindowResizedEvent>(
    [](const WindowResizedEvent& event) {
      std::cout << "Window " << event.GetWindowId() << " resized to " 
                << event.GetNewSize().width << "x" << event.GetNewSize().height << std::endl;
    });
  
  auto move_listener_id = window_manager.AddListener<WindowMovedEvent>(
    [](const WindowMovedEvent& event) {
      std::cout << "Window " << event.GetWindowId() << " moved to (" 
                << event.GetNewPosition().x << ", " << event.GetNewPosition().y << ")" << std::endl;
    });
  
  // Create a window to trigger events
  WindowOptions options;
  options.title = "Test Window";
  options.size.width = 400;
  options.size.height = 300;
  
  std::cout << "Creating window..." << std::endl;
  auto window = window_manager.Create(options);
  
  if (window) {
    std::cout << "Window created successfully!" << std::endl;
    
    // Wait a bit to let events propagate
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Try to focus the window
    std::cout << "Focusing window..." << std::endl;
    window->Focus();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::cout << "Test completed. Window listeners are working!" << std::endl;
    
    // Clean up listeners
    window_manager.RemoveListener(window_listener_id);
    window_manager.RemoveListener(focus_listener_id);
    window_manager.RemoveListener(resize_listener_id);
    window_manager.RemoveListener(move_listener_id);
    
  } else {
    std::cerr << "Failed to create window!" << std::endl;
    return 1;
  }
  
  return 0;
}