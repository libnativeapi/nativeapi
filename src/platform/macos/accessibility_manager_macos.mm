#include "../../accessibility_manager.h"

// Import Cocoa headers
#import <Cocoa/Cocoa.h>

namespace nativeapi {

void AccessibilityManager::Enable() {
  NSDictionary* options = @{(__bridge NSString*)kAXTrustedCheckOptionPrompt : @YES};
  AXIsProcessTrustedWithOptions((__bridge CFDictionaryRef)options);
}

bool AccessibilityManager::IsEnabled() {
  return AXIsProcessTrustedWithOptions(nil);
}

}  // namespace nativeapi
