#include <cstring>
#include <iostream>
#include <string>

#include "keyboard_monitor.h"

// Import Cocoa headers
#import <Cocoa/Cocoa.h>
#import <CoreGraphics/CoreGraphics.h>

namespace nativeapi {

class KeyboardMonitor::Impl {
 public:
  Impl(KeyboardMonitor* monitor) : monitor_(monitor) {}

  CFMachPortRef eventTap = nullptr;
  CFRunLoopSourceRef runLoopSource = nullptr;
  KeyboardMonitor* monitor_;
};

KeyboardMonitor::KeyboardMonitor() : impl_(std::make_unique<Impl>(this)), event_handler_(nullptr) {}

KeyboardMonitor::~KeyboardMonitor() {
  Stop();
}

// Callback function for keyboard events
static CGEventRef keyboardEventCallback(CGEventTapProxy proxy,
                                        CGEventType type,
                                        CGEventRef event,
                                        void* refcon) {
  auto* monitor = static_cast<KeyboardMonitor*>(refcon);
  if (!monitor)
    return event;

  auto* eventHandler = monitor->GetEventHandler();
  if (!eventHandler)
    return event;

  // Get the key code
  CGKeyCode keyCode = (CGKeyCode)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);

  // Convert key code to string representation
  UniCharCount actualStringLength = 0;
  UniChar unicodeString[4];
  CGEventKeyboardGetUnicodeString(event, 4, &actualStringLength, unicodeString);

  if (actualStringLength > 0) {
    std::string keyStr(actualStringLength, 0);
    for (UniCharCount i = 0; i < actualStringLength; ++i) {
      keyStr[i] = static_cast<char>(unicodeString[i]);
    }

    if (type == kCGEventKeyDown) {
      eventHandler->OnKeyPressed(keyStr);
    } else if (type == kCGEventKeyUp) {
      eventHandler->OnKeyReleased(keyStr);
    }
  }

  return event;
}

void KeyboardMonitor::Start() {
  if (impl_->eventTap != nullptr) {
    return;  // Already started
  }

  // Create event tap for keyboard events
  impl_->eventTap =
      CGEventTapCreate(kCGSessionEventTap,        // Monitor session-wide events
                       kCGHeadInsertEventTap,     // Insert at the head of the event queue
                       kCGEventTapOptionDefault,  // Default options
                       CGEventMaskBit(kCGEventKeyDown) |
                           CGEventMaskBit(kCGEventKeyUp),  // Monitor key down and up events
                       keyboardEventCallback,
                       this);  // Pass this pointer as user data

  if (impl_->eventTap == nullptr) {
    std::cerr << "Failed to create event tap" << std::endl;
    return;
  }

  // Create a run loop source
  impl_->runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, impl_->eventTap, 0);

  // Add to the current run loop
  CFRunLoopAddSource(CFRunLoopGetCurrent(), impl_->runLoopSource, kCFRunLoopCommonModes);

  // Enable the event tap
  CGEventTapEnable(impl_->eventTap, true);
}

void KeyboardMonitor::Stop() {
  if (impl_->eventTap == nullptr) {
    return;  // Already stopped
  }

  // Disable the event tap
  CGEventTapEnable(impl_->eventTap, false);

  // Remove from run loop
  if (impl_->runLoopSource != nullptr) {
    CFRunLoopRemoveSource(CFRunLoopGetCurrent(), impl_->runLoopSource, kCFRunLoopCommonModes);
    CFRelease(impl_->runLoopSource);
    impl_->runLoopSource = nullptr;
  }

  // Release the event tap
  CFRelease(impl_->eventTap);
  impl_->eventTap = nullptr;
}

bool KeyboardMonitor::IsMonitoring() const {
  return impl_->eventTap != nullptr;
}

}  // namespace nativeapi
