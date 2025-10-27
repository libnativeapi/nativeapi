#ifdef __OHOS__
#include <hilog/log.h>
#endif
#include <iostream>
#include "../../display_manager.h"

// Temporarily disable logging to avoid macro conflicts
#define HILOG_WARN(...) ((void)0)

namespace nativeapi {

DisplayManager::DisplayManager() {}

DisplayManager::~DisplayManager() {}

Display DisplayManager::GetPrimary() {
  return Display();
}

std::vector<Display> DisplayManager::GetAll() {
  std::vector<Display> displays;
  displays.push_back(Display());
  return displays;
}

Point DisplayManager::GetCursorPosition() {
  return Point{0, 0};
}

}  // namespace nativeapi
