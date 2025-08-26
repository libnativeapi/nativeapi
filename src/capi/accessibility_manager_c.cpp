#include <string.h>
#include <iostream>

#include "../accessibility_manager.h"
#include "accessibility_manager_c.h"

using namespace nativeapi;

static AccessibilityManager g_accessibility_manager = AccessibilityManager();

FFI_PLUGIN_EXPORT
void native_accessibility_manager_enable() {
  g_accessibility_manager.Enable();
}

FFI_PLUGIN_EXPORT
bool native_accessibility_manager_is_enabled() {
  return g_accessibility_manager.IsEnabled();
}
