#include "accessibility_manager_c.h"
#include "../accessibility_manager.h"

static nativeapi::AccessibilityManager& AccessibilityManagerInstance() {
  return nativeapi::AccessibilityManager::GetInstance();
}

FFI_PLUGIN_EXPORT
void native_accessibility_manager_enable() {
  AccessibilityManagerInstance().Enable();
}

FFI_PLUGIN_EXPORT
bool native_accessibility_manager_is_enabled() {
  return AccessibilityManagerInstance().IsEnabled();
}
