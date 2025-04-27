#pragma once

namespace nativeapi {

// AccessibilityManager is a singleton that manages all accessibility on the
// system.
class AccessibilityManager {
 public:
  AccessibilityManager();
  virtual ~AccessibilityManager();

  // Enable the accessibility
  void Enable();

  // Whether the accessibility is enabled
  bool IsEnabled();
};

}  // namespace nativeapi
