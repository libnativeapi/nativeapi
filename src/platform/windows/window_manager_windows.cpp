#include <windows.h>
#include <iostream>
#include <string>

#include "../../window.h"
#include "../../window_event.h"
#include "../../window_manager.h"
#include "string_utils_windows.h"

namespace nativeapi {

// Custom window procedure to handle window messages
static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
    case WM_CLOSE:
      // User clicked the close button
      DestroyWindow(hwnd);
      return 0;

    case WM_DESTROY:
      // Window is being destroyed
      // For now, we'll exit the application when any window is destroyed
      // In a more sophisticated implementation, we might want to check
      // if this is the last window before calling PostQuitMessage
      PostQuitMessage(0);
      return 0;

    default:
      return DefWindowProc(hwnd, uMsg, wParam, lParam);
  }
}

// Private implementation to hide Windows-specific details
class WindowManager::Impl {
 public:
  Impl(WindowManager* manager) : manager_(manager) {}
  ~Impl() {}

  void SetupEventMonitoring() {
    // Windows event monitoring would typically be done through:
    // - SetWinEventHook for system-wide window events
    // - Window subclassing for specific window events
    // This is a placeholder implementation
  }

  void CleanupEventMonitoring() {
    // Clean up any event hooks or monitoring
  }

  void OnWindowEvent(HWND hwnd, const std::string& event_type) {
    WindowID window_id = reinterpret_cast<WindowID>(hwnd);

    if (event_type == "focused") {
      WindowFocusedEvent event(window_id);
      manager_->DispatchWindowEvent(event);
    } else if (event_type == "blurred") {
      WindowBlurredEvent event(window_id);
      manager_->DispatchWindowEvent(event);
    } else if (event_type == "minimized") {
      WindowMinimizedEvent event(window_id);
      manager_->DispatchWindowEvent(event);
    } else if (event_type == "restored") {
      WindowRestoredEvent event(window_id);
      manager_->DispatchWindowEvent(event);
    } else if (event_type == "resized") {
      RECT rect;
      GetWindowRect(hwnd, &rect);
      Size new_size = {static_cast<double>(rect.right - rect.left),
                       static_cast<double>(rect.bottom - rect.top)};
      WindowResizedEvent event(window_id, new_size);
      manager_->DispatchWindowEvent(event);
    } else if (event_type == "moved") {
      RECT rect;
      GetWindowRect(hwnd, &rect);
      Point new_position = {static_cast<double>(rect.left), static_cast<double>(rect.top)};
      WindowMovedEvent event(window_id, new_position);
      manager_->DispatchWindowEvent(event);
    } else if (event_type == "closing") {
      WindowClosedEvent event(window_id);
      manager_->DispatchWindowEvent(event);
    }
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

// Create a new window with the given options
std::shared_ptr<Window> WindowManager::Create(const WindowOptions& options) {
  HINSTANCE hInstance = GetModuleHandle(nullptr);

  // Register window class if not already registered
  static bool class_registered = false;
  static std::wstring wclass_name = StringToWString("NativeAPIWindow");

  if (!class_registered) {
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = wclass_name.c_str();
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    if (RegisterClassW(&wc)) {
      class_registered = true;
    } else {
      DWORD error = GetLastError();
      if (error != ERROR_CLASS_ALREADY_EXISTS) {
        std::cerr << "Failed to register window class. Error: " << error << std::endl;
        return nullptr;
      }
      class_registered = true;
    }
  }

  // Create the window
  DWORD style = WS_OVERLAPPEDWINDOW;
  DWORD exStyle = 0;
  std::wstring wtitle = StringToWString(options.title);

  HWND hwnd =
      CreateWindowExW(exStyle, wclass_name.c_str(), wtitle.c_str(), style, CW_USEDEFAULT,
                      CW_USEDEFAULT, static_cast<int>(options.size.width),
                      static_cast<int>(options.size.height), nullptr, nullptr, hInstance, nullptr);

  if (!hwnd) {
    std::cerr << "Failed to create window. Error: " << GetLastError() << std::endl;
    return nullptr;
  }

  ShowWindow(hwnd, SW_SHOW);
  UpdateWindow(hwnd);

  WindowID window_id = reinterpret_cast<WindowID>(hwnd);
  auto window = std::make_shared<Window>(hwnd);
  windows_[window_id] = window;

  // Dispatch window created event
  WindowCreatedEvent created_event(window_id);
  DispatchWindowEvent(created_event);

  return window;
}

// Destroy a window by its ID. Returns true if window was destroyed.
bool WindowManager::Destroy(WindowID id) {
  auto it = windows_.find(id);
  if (it != windows_.end()) {
    HWND hwnd = reinterpret_cast<HWND>(id);
    if (IsWindow(hwnd)) {
      DestroyWindow(hwnd);
    }
    windows_.erase(it);
    return true;
  }
  return false;
}

std::shared_ptr<Window> WindowManager::Get(WindowID id) {
  auto it = windows_.find(id);
  if (it != windows_.end()) {
    return it->second;
  }

  // Check if the window still exists in the system
  HWND hwnd = reinterpret_cast<HWND>(id);
  if (IsWindow(hwnd)) {
    auto window = std::make_shared<Window>(hwnd);
    windows_[id] = window;
    return window;
  }

  return nullptr;
}

std::vector<std::shared_ptr<Window>> WindowManager::GetAll() {
  std::vector<std::shared_ptr<Window>> windows;

  // Enumerate all windows and add them to our collection
  // This is a simplified implementation
  for (auto& window_pair : windows_) {
    windows.push_back(window_pair.second);
  }

  return windows;
}

std::shared_ptr<Window> WindowManager::GetCurrent() {
  HWND hwnd = GetForegroundWindow();
  if (hwnd) {
    WindowID window_id = reinterpret_cast<WindowID>(hwnd);
    return Get(window_id);
  }
  return nullptr;
}

}  // namespace nativeapi
