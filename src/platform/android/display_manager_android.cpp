#include <android/log.h>
#include "../../display_manager.h"

#define LOG_TAG "NativeApi"
#define ALOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)

namespace nativeapi {

DisplayManager::DisplayManager() {}
DisplayManager::~DisplayManager() {}

std::vector<Display> DisplayManager::GetAll() {
  return {Display()};
}

Display DisplayManager::GetPrimary() {
  return Display();
}

Point DisplayManager::GetCursorPosition() {
  return Point{0, 0};
}

}  // namespace nativeapi
