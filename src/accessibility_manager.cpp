#include <iostream>

#include "accessibility_manager.h"

namespace nativeapi {

AccessibilityManager::AccessibilityManager() {
  std::cout << "AccessibilityManager::AccessibilityManager()" << std::endl;
};

AccessibilityManager::~AccessibilityManager() {
  std::cout << "AccessibilityManager::~AccessibilityManager()" << std::endl;
};

}  // namespace nativeapi
