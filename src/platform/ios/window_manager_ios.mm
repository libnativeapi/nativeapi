#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>
#include "../../window_manager.h"

namespace nativeapi {

class WindowManager::Impl {
 public:
  Impl(WindowManager* manager) : manager_(manager) {}
  WindowManager* manager_;
};

WindowManager::WindowManager() : pimpl_(std::make_unique<Impl>(this)) {
  SetupEventMonitoring();
}

WindowManager::~WindowManager() {
  CleanupEventMonitoring();
}

std::shared_ptr<Window> WindowManager::Create(const WindowOptions& options) {
  // On iOS, windows are created through the UIApplication scene lifecycle
  // This is typically handled by the app delegate, not directly
  
  CGRect screenBounds = [UIScreen mainScreen].bounds;
  UIWindow* uiWindow = [[UIWindow alloc] initWithFrame:screenBounds];
  
  auto window = std::make_shared<Window>((__bridge void*)uiWindow);
  
  if (window) {
    windows_[window->GetId()] = window;
    Emit<WindowCreatedEvent>(window->GetId());
  }
  
  return window;
}

std::shared_ptr<Window> WindowManager::Get(WindowId id) {
  auto it = windows_.find(id);
  return (it != windows_.end()) ? it->second : nullptr;
}

std::vector<std::shared_ptr<Window>> WindowManager::GetAll() {
  std::vector<std::shared_ptr<Window>> result;
  result.reserve(windows_.size());
  
  for (const auto& [id, window] : windows_) {
    result.push_back(window);
  }
  
  return result;
}

std::shared_ptr<Window> WindowManager::GetCurrent() {
  // Find the first key window
  for (const auto& [id, window] : windows_) {
    if (window->IsFocused()) {
      return window;
    }
  }
  return nullptr;
}

bool WindowManager::Destroy(WindowId id) {
  auto it = windows_.find(id);
  if (it == windows_.end()) {
    return false;
  }
  
  windows_.erase(it);
  Emit<WindowClosedEvent>(id);
  
  return true;
}

void WindowManager::SetupEventMonitoring() {
  // iOS manages window events through UIKit
}

void WindowManager::CleanupEventMonitoring() {
  // No cleanup needed
}

void WindowManager::DispatchWindowEvent(const WindowEvent& event) {
  Emit(event);
}

}  // namespace nativeapi

