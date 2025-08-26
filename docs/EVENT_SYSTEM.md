# Generic Event Handling System

## Overview

The Generic Event Handling System provides a type-safe, flexible, and thread-safe mechanism for handling events in C++. It supports both observer patterns and callback-based event handling, making it suitable for various use cases in native API development.

## Key Features

- **Type Safety**: Template-based design ensures compile-time type checking
- **Flexible Patterns**: Supports both observer pattern and callback-based handling
- **Thread Safety**: Safe for multi-threaded environments with proper synchronization
- **Asynchronous Processing**: Built-in support for async event dispatch with background thread
- **Automatic Cleanup**: RAII-based listener management prevents memory leaks
- **Exception Safety**: Handles exceptions in event listeners gracefully
- **Event Timestamping**: All events include creation timestamps
- **Generic Design**: Works with any event type through templates

## Core Components

### Event Base Classes

- `Event`: Base class for all events, provides timestamping and type information
- `TypedEvent<T>`: Template base class that provides automatic type identification
- `EventListener`: Generic event listener interface
- `TypedEventListener<T>`: Type-safe event listener for specific event types
- `CallbackEventListener<T>`: Wrapper for std::function callbacks

### Event Dispatcher

- `EventDispatcher`: Main class that manages event distribution
- Supports both synchronous and asynchronous event dispatch
- Thread-safe listener management
- Background thread for async event processing

### Utility Classes

- `EventListenerGuard`: RAII helper for automatic listener cleanup
- `GetGlobalEventDispatcher()`: Singleton access to global event dispatcher

## Usage Examples

### 1. Define Custom Events

```cpp
#include "event_system.h"

// Define a custom event
class UserLoginEvent : public nativeapi::TypedEvent<UserLoginEvent> {
 public:
  UserLoginEvent(const std::string& username, bool success)
      : username_(username), success_(success) {}
  
  const std::string& GetUsername() const { return username_; }
  bool IsSuccess() const { return success_; }
  
 private:
  std::string username_;
  bool success_;
};
```

### 2. Observer Pattern

```cpp
// Implement an observer
class LoginObserver : public nativeapi::TypedEventListener<UserLoginEvent> {
 public:
  void OnTypedEvent(const UserLoginEvent& event) override {
    std::cout << "User " << event.GetUsername() 
              << (event.IsSuccess() ? " logged in" : " login failed") << std::endl;
  }
};

// Register the observer
nativeapi::EventDispatcher dispatcher;
LoginObserver observer;
auto listener_id = dispatcher.AddListener<UserLoginEvent>(&observer);

// Dispatch an event
dispatcher.DispatchSync<UserLoginEvent>("john_doe", true);

// Clean up
dispatcher.RemoveListener(listener_id);
```

### 3. Callback Pattern

```cpp
// Register a callback function
auto callback_id = dispatcher.AddListener<UserLoginEvent>(
    [](const UserLoginEvent& event) {
        // Handle the event
        std::cout << "Callback: " << event.GetUsername() << std::endl;
    });

// Or use RAII for automatic cleanup
auto guard = nativeapi::AddScopedListener<UserLoginEvent>(dispatcher,
    [](const UserLoginEvent& event) {
        std::cout << "Scoped callback: " << event.GetUsername() << std::endl;
    });
// Listener is automatically removed when guard goes out of scope
```

### 4. Multi-Event Observer

```cpp
// Handle multiple event types with one observer
class MultiEventObserver : public nativeapi::EventListener {
 public:
  void OnEvent(const nativeapi::Event& event) override {
    if (auto login_event = dynamic_cast<const UserLoginEvent*>(&event)) {
      HandleLogin(*login_event);
    } else if (auto logout_event = dynamic_cast<const UserLogoutEvent*>(&event)) {
      HandleLogout(*logout_event);
    }
  }
  
 private:
  void HandleLogin(const UserLoginEvent& event) { /* ... */ }
  void HandleLogout(const UserLogoutEvent& event) { /* ... */ }
};
```

### 5. Asynchronous Event Processing

```cpp
// Start background thread for async processing
dispatcher.Start();

// Dispatch events asynchronously
dispatcher.DispatchAsync<UserLoginEvent>("alice", true);
dispatcher.DispatchAsync<UserLoginEvent>("bob", false);

// Events will be processed on background thread
// Stop background processing when done
dispatcher.Stop();
```

### 6. Global Event System

```cpp
// Use the global event dispatcher
auto& global_dispatcher = nativeapi::GetGlobalEventDispatcher();

global_dispatcher.AddListener<UserLoginEvent>([](const UserLoginEvent& event) {
    // This listener is globally available
});
```

## Thread Safety

The event system is designed to be thread-safe:

- Listener registration/removal is protected by mutexes
- Event dispatch creates snapshots of listeners to avoid holding locks during dispatch
- Async event processing uses condition variables for efficient waiting
- Exception handling prevents one listener from affecting others

## Performance Considerations

- **Synchronous dispatch**: Direct function calls, minimal overhead
- **Asynchronous dispatch**: Events are queued and processed on background thread
- **Memory management**: Automatic cleanup of callback listeners
- **Exception handling**: Try-catch blocks around listener calls

## Integration with Existing Code

The event system can be integrated with existing components:

```cpp
// Example: Extend DisplayManager to use events
class EventAwareDisplayManager : public DisplayManager {
 public:
  EventAwareDisplayManager(nativeapi::EventDispatcher& dispatcher) 
      : dispatcher_(dispatcher) {}
  
 protected:
  void OnDisplayAdded(const Display& display) {
    // Existing logic...
    
    // Dispatch event
    dispatcher_.DispatchAsync<nativeapi::DisplayAddedEvent>(display);
  }
  
 private:
  nativeapi::EventDispatcher& dispatcher_;
};
```

## Best Practices

1. **Use RAII**: Prefer `EventListenerGuard` or scoped listeners for automatic cleanup
2. **Handle exceptions**: Event listeners should handle their own exceptions
3. **Avoid blocking**: Keep event handlers fast to avoid blocking other listeners
4. **Use appropriate dispatch**: Sync for immediate handling, async for background processing
5. **Clean up**: Remove listeners when they're no longer needed
6. **Type safety**: Prefer `TypedEventListener` over raw `EventListener` when possible

## Common Event Types

The system includes predefined events for common scenarios:

- `DisplayAddedEvent`, `DisplayRemovedEvent`: Display management
- `KeyPressedEvent`, `KeyReleasedEvent`: Keyboard input
- `MouseMovedEvent`, `MouseClickedEvent`: Mouse input
- `WindowCreatedEvent`, `WindowClosedEvent`: Window management
- `ApplicationStartedEvent`, `ApplicationExitingEvent`: Application lifecycle

These can be used directly or serve as examples for creating custom events.