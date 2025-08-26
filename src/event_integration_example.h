#pragma once

/**
 * Integration Example: Event-Aware Display Manager
 * 
 * This example shows how to integrate the generic event system
 * with existing components like DisplayManager.
 */

#include "display_manager.h"
#include "event_system.h"
#include "common_events.h"

namespace nativeapi {

/**
 * Extended DisplayManager that uses the generic event system
 * in addition to the existing listener pattern.
 * 
 * This demonstrates how existing code can be enhanced with the
 * new event system without breaking existing functionality.
 */
class EventAwareDisplayManager : public DisplayManager {
 public:
  explicit EventAwareDisplayManager(EventDispatcher* dispatcher = nullptr) 
      : dispatcher_(dispatcher ? dispatcher : &GetGlobalEventDispatcher()) {}

  // Override methods to dispatch events
  void AddListener(DisplayListener* listener) override {
    DisplayManager::AddListener(listener);
    
    // Also dispatch an event when listeners are added
    // This allows other parts of the system to react to listener changes
    if (dispatcher_) {
      // We could define a ListenerAddedEvent if needed
    }
  }

  void RemoveListener(DisplayListener* listener) override {
    DisplayManager::RemoveListener(listener);
    
    // Dispatch event for listener removal if needed
  }

 protected:
  /**
   * Override the notification method to also dispatch generic events.
   * This is called internally when displays change.
   */
  void NotifyListeners(std::function<void(DisplayListener*)> callback) override {
    // Call the base implementation first
    DisplayManager::NotifyListeners(callback);
    
    // If this was a display added/removed notification, we could
    // dispatch the corresponding event here. For example:
    // dispatcher_->DispatchAsync<DisplayChangedEvent>(...);
  }

  /**
   * Example method that would be called when a display is actually added.
   * This would normally be called by the platform-specific implementation.
   */
  void OnSystemDisplayAdded(const Display& display) {
    // Update internal state
    // ... existing logic ...
    
    // Notify existing listeners
    NotifyListeners([&display](DisplayListener* listener) {
      listener->OnDisplayAdded(display);
    });
    
    // Dispatch generic event
    if (dispatcher_) {
      dispatcher_->DispatchAsync<DisplayAddedEvent>(display);
    }
  }

  /**
   * Example method for display removal
   */
  void OnSystemDisplayRemoved(const Display& display) {
    // Update internal state
    // ... existing logic ...
    
    // Notify existing listeners
    NotifyListeners([&display](DisplayListener* listener) {
      listener->OnDisplayRemoved(display);
    });
    
    // Dispatch generic event
    if (dispatcher_) {
      dispatcher_->DispatchAsync<DisplayRemovedEvent>(display);
    }
  }

 private:
  EventDispatcher* dispatcher_;
};

/**
 * Example of a unified event handler that can handle both
 * keyboard and display events using the generic event system.
 */
class UnifiedSystemEventHandler : public EventListener {
 public:
  void OnEvent(const Event& event) override {
    // Handle different event types
    if (auto display_added = dynamic_cast<const DisplayAddedEvent*>(&event)) {
      HandleDisplayAdded(*display_added);
    } else if (auto display_removed = dynamic_cast<const DisplayRemovedEvent*>(&event)) {
      HandleDisplayRemoved(*display_removed);
    } else if (auto key_pressed = dynamic_cast<const KeyPressedEvent*>(&event)) {
      HandleKeyPressed(*key_pressed);
    } else if (auto key_released = dynamic_cast<const KeyReleasedEvent*>(&event)) {
      HandleKeyReleased(*key_released);
    } else if (auto app_started = dynamic_cast<const ApplicationStartedEvent*>(&event)) {
      HandleApplicationStarted(*app_started);
    }
  }

 private:
  void HandleDisplayAdded(const DisplayAddedEvent& event) {
    std::cout << "System: Display added" << std::endl;
    // Handle display addition...
  }

  void HandleDisplayRemoved(const DisplayRemovedEvent& event) {
    std::cout << "System: Display removed" << std::endl;
    // Handle display removal...
  }

  void HandleKeyPressed(const KeyPressedEvent& event) {
    std::cout << "System: Key " << event.GetKeycode() << " pressed" << std::endl;
    // Handle key press...
  }

  void HandleKeyReleased(const KeyReleasedEvent& event) {
    std::cout << "System: Key " << event.GetKeycode() << " released" << std::endl;
    // Handle key release...
  }

  void HandleApplicationStarted(const ApplicationStartedEvent& event) {
    std::cout << "System: Application started, setting up..." << std::endl;
    // Initialize system components...
  }
};

}  // namespace nativeapi