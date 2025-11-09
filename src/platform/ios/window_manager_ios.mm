#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#include "../../window_manager.h"
#include "../../window_registry.h"

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
    WindowRegistry::GetInstance().Add(window->GetId(), window);
    Emit<WindowCreatedEvent>(window->GetId());
  }

  return window;
}

std::shared_ptr<Window> WindowManager::Get(WindowId id) {
  return WindowRegistry::GetInstance().Get(id);
}

std::vector<std::shared_ptr<Window>> WindowManager::GetAll() {
  return WindowRegistry::GetInstance().GetAll();
}

std::shared_ptr<Window> WindowManager::GetCurrent() {
  // Find the first key window
  for (const auto& window : WindowRegistry::GetInstance().GetAll()) {
    if (window->IsFocused()) {
      return window;
    }
  }
  return nullptr;
}

bool WindowManager::Destroy(WindowId id) {
  auto window = WindowRegistry::GetInstance().Get(id);
  if (!window) {
    return false;
  }

  WindowRegistry::GetInstance().Remove(id);
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

void WindowManager::SetWillShowHook(std::optional<WindowWillShowHook> hook) {
  // Empty implementation
}

void WindowManager::SetWillHideHook(std::optional<WindowWillHideHook> hook) {
  // Empty implementation
}

void WindowManager::InvokeWillShowHook(WindowId id) {
  // Empty implementation
}

void WindowManager::InvokeWillHideHook(WindowId id) {
  // Empty implementation
}

}  // namespace nativeapi
