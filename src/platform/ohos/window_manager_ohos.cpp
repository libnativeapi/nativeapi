#ifdef __OHOS__
#include <hilog/log.h>
#endif
#include <iostream>
#include <memory>
#include <mutex>
#include <unordered_map>
#include "../../window.h"
#include "../../window_manager.h"

namespace nativeapi {

// Helper function to manage mapping between window pointers and WindowIds
static WindowId GetOrCreateWindowId(void* native_window) {
  if (!native_window) {
    return IdAllocator::kInvalidId;
  }

  static std::unordered_map<void*, WindowId> window_id_map;
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

// Helper function to find window by WindowId
static void* FindNativeWindowById(WindowId id) {
  static std::unordered_map<void*, WindowId> window_id_map;
  static std::mutex map_mutex;

  std::lock_guard<std::mutex> lock(map_mutex);
  for (const auto& pair : window_id_map) {
    if (pair.second == id) {
      return pair.first;
    }
  }
  return nullptr;
}

// Private implementation for OpenHarmony
class WindowManager::Impl {
 public:
  Impl(WindowManager* manager) : manager_(manager) {}
  ~Impl() {}

  void SetupEventMonitoring() {
    // On OpenHarmony, event monitoring is done through Ability callbacks
    // Window event monitoring setup
  }

  void CleanupEventMonitoring() {
    // Window event monitoring cleanup
  }

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
  void* native_window = FindNativeWindowById(id);
  if (native_window) {
    auto window = std::make_shared<Window>(native_window);
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
  // On OpenHarmony, the current window is typically the Ability's window
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
  // On OpenHarmony, window creation is handled by the Ability lifecycle
  // Window creation requested (OpenHarmony handles this through Ability lifecycle)

  // Return nullptr as windows are created by the OpenHarmony system
  return nullptr;
}

}  // namespace nativeapi
