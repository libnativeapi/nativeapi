#include <android/log.h>
#include <android/native_activity.h>
#include <android/native_window.h>
#include <iostream>
#include <mutex>
#include <unordered_map>
#include "../../window.h"
#include "../../window_manager.h"

#define LOG_TAG "NativeApi"
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ALOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

namespace nativeapi {

// Helper function to manage mapping between ANativeWindow pointers and WindowIds
static WindowId GetOrCreateWindowId(ANativeWindow* native_window) {
  if (!native_window) {
    return IdAllocator::kInvalidId;
  }

  static std::unordered_map<ANativeWindow*, WindowId> window_id_map;
  static std::mutex map_mutex;

  std::lock_guard<std::mutex> lock(map_mutex);
  auto it = window_id_map.find(native_window);
  if (it != window_id_map.end()) {
    return it->second;
  }

  // Allocate new ID using the IdAllocator
  WindowId new_id = IdAllocator::Allocate<Window>();
  if (new_id != IdAllocator::kInvalidId) {
    window_id_map[native_window] = new_id;
  }
  return new_id;
}

// Helper function to find ANativeWindow by WindowId
static ANativeWindow* FindNativeWindowById(WindowId id) {
  static std::unordered_map<ANativeWindow*, WindowId> window_id_map;
  static std::mutex map_mutex;

  std::lock_guard<std::mutex> lock(map_mutex);
  for (const auto& pair : window_id_map) {
    if (pair.second == id) {
      return pair.first;
    }
  }
  return nullptr;
}

// Private implementation for Android
class WindowManager::Impl {
 public:
  Impl(WindowManager* manager) : manager_(manager) {}
  ~Impl() {}

  void SetupEventMonitoring() {
    // On Android, event monitoring is done through Activity callbacks
    // Setup will be handled by the Activity lifecycle
    ALOGI("Window event monitoring setup");
  }

  void CleanupEventMonitoring() { ALOGI("Window event monitoring cleanup"); }

 private:
  WindowManager* manager_;
};

WindowManager::WindowManager() : pimpl_(std::make_unique<Impl>(this)) {
  SetupEventMonitoring();
}

WindowManager::~WindowManager() {
  CleanupEventMonitoring();
}

void WindowManager::SetupEventMonitoring() {
  pimpl_->SetupEventMonitoring();
}

void WindowManager::CleanupEventMonitoring() {
  pimpl_->CleanupEventMonitoring();
}

void WindowManager::DispatchWindowEvent(const WindowEvent& event) {
  Emit(event);
}

std::shared_ptr<Window> WindowManager::Get(WindowId id) {
  auto it = windows_.find(id);
  if (it != windows_.end()) {
    return it->second;
  }

  // Try to find the window by ID
  ANativeWindow* native_window = FindNativeWindowById(id);
  if (native_window) {
    auto window = std::make_shared<Window>(static_cast<void*>(native_window));
    windows_[id] = window;
    return window;
  }

  return nullptr;
}

std::vector<std::shared_ptr<Window>> WindowManager::GetAll() {
  std::vector<std::shared_ptr<Window>> windows;

  for (const auto& [id, window] : windows_) {
    windows.push_back(window);
  }

  return windows;
}

std::shared_ptr<Window> WindowManager::GetCurrent() {
  // On Android, the current window is typically the Activity's native window
  // This would need to be set by the Activity lifecycle callbacks
  if (!windows_.empty()) {
    return windows_.begin()->second;
  }
  return nullptr;
}

bool WindowManager::Destroy(WindowId id) {
  auto it = windows_.find(id);
  if (it != windows_.end()) {
    windows_.erase(it);
    Emit<WindowClosedEvent>(id);
    return true;
  }
  return false;
}

std::shared_ptr<Window> WindowManager::Create(const WindowOptions& options) {
  // On Android, window creation is handled by the Activity lifecycle
  // This function would typically be called from onNativeWindowCreated callback
  ALOGI("Window creation requested (Android handles this through Activity lifecycle)");

  // Return nullptr as windows are created by the Android system
  return nullptr;
}

}  // namespace nativeapi
