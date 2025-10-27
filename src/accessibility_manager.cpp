#include "accessibility_manager.h"

#ifdef ANDROID
#include <android/log.h>
#define LOG_TAG "NativeApi"
#define ALOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#endif

namespace nativeapi {

AccessibilityManager& AccessibilityManager::GetInstance() {
  static AccessibilityManager instance;
  return instance;
}

AccessibilityManager::AccessibilityManager() : enabled_(false) {}

AccessibilityManager::~AccessibilityManager() {}

void AccessibilityManager::Enable() {
#ifdef ANDROID
  ALOGW("AccessibilityManager::Enable requires AccessibilityService on Android");
#endif
  enabled_ = true;
}

bool AccessibilityManager::IsEnabled() {
  return enabled_;
}

}  // namespace nativeapi
