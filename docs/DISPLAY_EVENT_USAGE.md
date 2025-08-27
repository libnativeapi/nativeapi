# DisplayManager Event System Usage Examples

This document demonstrates how to use both the legacy DisplayListener interface and the new EventDispatcher-based event system.

## Legacy API (Backward Compatibility)

```cpp
#include "nativeapi.h"
using namespace nativeapi;

// Option 1: Implement DisplayListener interface
class MyDisplayListener : public DisplayListener {
public:
    void OnDisplayAdded(const Display& display) override {
        std::cout << "Display added: " << display.name << std::endl;
    }
    
    void OnDisplayRemoved(const Display& display) override {
        std::cout << "Display removed: " << display.name << std::endl;
    }
};

void useLegacyAPI() {
    DisplayManager displayManager;
    MyDisplayListener listener;
    
    // Register listener
    displayManager.AddListener(&listener);
    
    // Events will be received via OnDisplayAdded/OnDisplayRemoved
    
    // Don't forget to remove when done
    displayManager.RemoveListener(&listener);
}

// Option 2: Use DisplayEventHandler with callbacks
void useLegacyCallbackAPI() {
    DisplayManager displayManager;
    
    DisplayEventHandler handler(
        [](const Display& display) {
            std::cout << "Display added: " << display.name << std::endl;
        },
        [](const Display& display) {
            std::cout << "Display removed: " << display.name << std::endl;
        }
    );
    
    displayManager.AddListener(&handler);
    // ... use the handler
    displayManager.RemoveListener(&handler);
}
```

## New Event API (Recommended)

```cpp
#include "nativeapi.h"
using namespace nativeapi;

// Option 1: Use callback functions (simplest)
void useNewEventAPI() {
    DisplayManager displayManager;
    
    // Register event listeners with callbacks
    auto addedListenerId = displayManager.AddEventListener<DisplayAddedEvent>(
        [](const DisplayAddedEvent& event) {
            const Display& display = event.GetDisplay();
            std::cout << "Display added: " << display.name << std::endl;
        });
        
    auto removedListenerId = displayManager.AddEventListener<DisplayRemovedEvent>(
        [](const DisplayRemovedEvent& event) {
            const Display& display = event.GetDisplay();
            std::cout << "Display removed: " << display.name << std::endl;
        });
    
    // Events will be dispatched automatically
    
    // Remove listeners when done
    displayManager.RemoveEventListener(addedListenerId);
    displayManager.RemoveEventListener(removedListenerId);
}

// Option 2: Implement TypedEventListener (for complex logic)
class MyDisplayEventListener : public TypedEventListener<DisplayAddedEvent> {
public:
    void OnTypedEvent(const DisplayAddedEvent& event) override {
        const Display& display = event.GetDisplay();
        std::cout << "New display detected: " << display.name << std::endl;
        // Complex logic here...
    }
};

void useNewEventAPIWithListener() {
    DisplayManager displayManager;
    MyDisplayEventListener listener;
    
    auto listenerId = displayManager.AddEventListener<DisplayAddedEvent>(&listener);
    
    // ... use
    
    displayManager.RemoveEventListener(listenerId);
}

// Option 3: Use EventDispatcher directly
void useEventDispatcherDirectly() {
    DisplayManager displayManager;
    EventDispatcher& dispatcher = displayManager.GetEventDispatcher();
    
    auto listenerId = dispatcher.AddListener<DisplayAddedEvent>(
        [](const DisplayAddedEvent& event) {
            // Handle event
        });
    
    // You can also dispatch custom events if needed
    Display customDisplay;
    customDisplay.name = "Custom Display";
    dispatcher.DispatchSync<DisplayAddedEvent>(customDisplay);
    
    dispatcher.RemoveListener(listenerId);
}
```

## Migration Guide

### From Legacy to New API

```cpp
// OLD WAY
class OldListener : public DisplayListener {
    void OnDisplayAdded(const Display& display) override {
        handleDisplayAdded(display);
    }
    void OnDisplayRemoved(const Display& display) override {
        handleDisplayRemoved(display);
    }
};

DisplayManager manager;
OldListener listener;
manager.AddListener(&listener);

// NEW WAY
DisplayManager manager;
auto addedId = manager.AddEventListener<DisplayAddedEvent>(
    [](const DisplayAddedEvent& event) {
        handleDisplayAdded(event.GetDisplay());
    });
auto removedId = manager.AddEventListener<DisplayRemovedEvent>(
    [](const DisplayRemovedEvent& event) {
        handleDisplayRemoved(event.GetDisplay());
    });
```

### Benefits of New API

1. **Type Safety**: Compile-time checking of event types
2. **Flexibility**: Can use lambdas, function pointers, or listener classes
3. **Advanced Features**: Async dispatch, listener management by ID
4. **Consistency**: Same event system used across the entire library
5. **Extensibility**: Easy to add new event types without changing interfaces

### Backward Compatibility

The legacy DisplayListener interface is fully supported and will continue to work. Both systems can be used simultaneously - events will be dispatched to both old and new listeners.

## Best Practices

1. **Prefer the new event API** for new code
2. **Use callback functions** for simple event handling
3. **Use TypedEventListener classes** for complex event handling logic
4. **Always remove listeners** to prevent memory leaks
5. **Store listener IDs** returned by AddEventListener for later removal
6. **Consider using RAII** to automatically manage listener lifetimes

## Event Types

- `DisplayAddedEvent`: Fired when a display is connected
- `DisplayRemovedEvent`: Fired when a display is disconnected

Both events provide access to the `Display` object via `GetDisplay()` method.