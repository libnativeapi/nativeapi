#include "../../accessibility_manager.h"

#include <windows.h>
#include <oleacc.h>

namespace nativeapi {

void AccessibilityManager::Enable() {
  // Windows accessibility is typically managed at the system level
  // Applications can enable specific accessibility features through COM
  // For now, this is a placeholder implementation
}

bool AccessibilityManager::IsEnabled() {
  // Check if high contrast is enabled (common accessibility feature)
  HIGHCONTRAST hc = {0};
  hc.cbSize = sizeof(HIGHCONTRAST);
  
  if (SystemParametersInfo(SPI_GETHIGHCONTRAST, sizeof(HIGHCONTRAST), &hc, 0)) {
    return (hc.dwFlags & HCF_HIGHCONTRASTON) != 0;
  }
  
  // If we can't determine high contrast, check for screen reader
  // This is a basic check - in practice you might want to check for specific assistive technologies
  return GetSystemMetrics(SM_SCREENREADER) != 0;
}

}  // namespace nativeapi