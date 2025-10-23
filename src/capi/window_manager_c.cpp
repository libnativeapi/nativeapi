#include "window_manager_c.h"
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>
#include "../window_manager.h"

using namespace nativeapi;

// Global state for event callbacks
struct EventCallbackInfo {
  native_window_event_callback_t callback;
  void* user_data;
  int id;
};

static std::mutex g_callback_mutex;
static std::unordered_map<int, EventCallbackInfo> g_event_callbacks;
static int g_next_callback_id = 1;

// Helper function to convert C++ WindowOptions to C options
static WindowOptions ConvertToWindowOptions(const native_window_options_t* options) {
  WindowOptions cpp_options;

  if (options->title) {
    cpp_options.title = std::string(options->title);
  }

  cpp_options.size.width = options->size.width;
  cpp_options.size.height = options->size.height;
  cpp_options.minimum_size.width = options->minimum_size.width;
  cpp_options.minimum_size.height = options->minimum_size.height;
  cpp_options.maximum_size.width = options->maximum_size.width;
  cpp_options.maximum_size.height = options->maximum_size.height;
  cpp_options.centered = options->centered;

  return cpp_options;
}

// Helper function to create native_window_t from shared_ptr<Window>
static native_window_t CreateNativeWindowHandle(std::shared_ptr<Window> window) {
  if (!window)
    return nullptr;
  // Cast shared_ptr to void* - the WindowManager maintains the actual
  // shared_ptr
  return static_cast<void*>(window.get());
}

// Helper function to dispatch events to registered callbacks
static void DispatchEvent(const native_window_event_t& event) {
  std::lock_guard<std::mutex> lock(g_callback_mutex);

  for (const auto& [id, callback_info] : g_event_callbacks) {
    try {
      callback_info.callback(&event, callback_info.user_data);
    } catch (...) {
      // Ignore exceptions from callbacks
    }
  }
}

// Event listener class to bridge C++ events to C callbacks
class CEventListener {
 public:
  CEventListener() {
    auto& manager = WindowManager::GetInstance();

    // Register for various window events
    manager.AddListener<WindowCreatedEvent>([this](const WindowCreatedEvent& e) {
      native_window_event_t event;
      event.type = NATIVE_WINDOW_EVENT_CREATED;
      event.window_id = e.GetWindowId();
      DispatchEvent(event);
    });

    manager.AddListener<WindowClosedEvent>([this](const WindowClosedEvent& e) {
      native_window_event_t event;
      event.type = NATIVE_WINDOW_EVENT_CLOSED;
      event.window_id = e.GetWindowId();
      DispatchEvent(event);
    });

    manager.AddListener<WindowFocusedEvent>([this](const WindowFocusedEvent& e) {
      native_window_event_t event;
      event.type = NATIVE_WINDOW_EVENT_FOCUSED;
      event.window_id = e.GetWindowId();
      DispatchEvent(event);
    });

    manager.AddListener<WindowBlurredEvent>([this](const WindowBlurredEvent& e) {
      native_window_event_t event;
      event.type = NATIVE_WINDOW_EVENT_BLURRED;
      event.window_id = e.GetWindowId();
      DispatchEvent(event);
    });

    manager.AddListener<WindowMinimizedEvent>([this](const WindowMinimizedEvent& e) {
      native_window_event_t event;
      event.type = NATIVE_WINDOW_EVENT_MINIMIZED;
      event.window_id = e.GetWindowId();
      DispatchEvent(event);
    });

    manager.AddListener<WindowMaximizedEvent>([this](const WindowMaximizedEvent& e) {
      native_window_event_t event;
      event.type = NATIVE_WINDOW_EVENT_MAXIMIZED;
      event.window_id = e.GetWindowId();
      DispatchEvent(event);
    });

    manager.AddListener<WindowRestoredEvent>([this](const WindowRestoredEvent& e) {
      native_window_event_t event;
      event.type = NATIVE_WINDOW_EVENT_RESTORED;
      event.window_id = e.GetWindowId();
      DispatchEvent(event);
    });

    manager.AddListener<WindowMovedEvent>([this](const WindowMovedEvent& e) {
      native_window_event_t event;
      event.type = NATIVE_WINDOW_EVENT_MOVED;
      event.window_id = e.GetWindowId();
      event.data.moved.position.x = e.GetNewPosition().x;
      event.data.moved.position.y = e.GetNewPosition().y;
      DispatchEvent(event);
    });

    manager.AddListener<WindowResizedEvent>([this](const WindowResizedEvent& e) {
      native_window_event_t event;
      event.type = NATIVE_WINDOW_EVENT_RESIZED;
      event.window_id = e.GetWindowId();
      event.data.resized.size.width = e.GetNewSize().width;
      event.data.resized.size.height = e.GetNewSize().height;
      DispatchEvent(event);
    });
  }
};

static std::unique_ptr<CEventListener> g_event_listener;

// Window manager operations
FFI_PLUGIN_EXPORT
native_window_t native_window_manager_create(const native_window_options_t* options) {
  if (!options)
    return nullptr;

  try {
    WindowOptions cpp_options = ConvertToWindowOptions(options);
    auto& manager = WindowManager::GetInstance();
    auto window = manager.Create(cpp_options);

    return CreateNativeWindowHandle(window);
  } catch (...) {
    return nullptr;
  }
}

FFI_PLUGIN_EXPORT
native_window_t native_window_manager_get(native_window_id_t window_id) {
  try {
    auto& manager = WindowManager::GetInstance();
    auto window = manager.Get(window_id);

    return CreateNativeWindowHandle(window);
  } catch (...) {
    return nullptr;
  }
}

FFI_PLUGIN_EXPORT
native_window_list_t native_window_manager_get_all(void) {
  native_window_list_t result = {nullptr, 0};

  try {
    auto& manager = WindowManager::GetInstance();
    auto windows = manager.GetAll();

    if (windows.empty()) {
      return result;
    }

    result.windows = new (std::nothrow) native_window_t[windows.size()];
    if (!result.windows) {
      return result;
    }

    result.count = windows.size();

    for (size_t i = 0; i < windows.size(); ++i) {
      result.windows[i] = CreateNativeWindowHandle(windows[i]);
    }

    return result;
  } catch (...) {
    if (result.windows) {
      delete[] result.windows;
      result.windows = nullptr;
      result.count = 0;
    }
    return result;
  }
}

FFI_PLUGIN_EXPORT
native_window_t native_window_manager_get_current(void) {
  try {
    auto& manager = WindowManager::GetInstance();
    auto current_window = manager.GetCurrent();

    return CreateNativeWindowHandle(current_window);
  } catch (...) {
    return nullptr;
  }
}

FFI_PLUGIN_EXPORT
bool native_window_manager_destroy(native_window_id_t window_id) {
  try {
    auto& manager = WindowManager::GetInstance();
    return manager.Destroy(window_id);
  } catch (...) {
    return false;
  }
}

FFI_PLUGIN_EXPORT
int native_window_manager_register_event_callback(native_window_event_callback_t callback,
                                                  void* user_data) {
  if (!callback)
    return -1;

  std::lock_guard<std::mutex> lock(g_callback_mutex);

  int callback_id = g_next_callback_id++;
  EventCallbackInfo info;
  info.callback = callback;
  info.user_data = user_data;
  info.id = callback_id;

  g_event_callbacks[callback_id] = info;

  // Initialize event listener if this is the first callback
  if (!g_event_listener) {
    try {
      g_event_listener = std::make_unique<CEventListener>();
    } catch (...) {
      g_event_callbacks.erase(callback_id);
      return -1;
    }
  }

  return callback_id;
}

FFI_PLUGIN_EXPORT
bool native_window_manager_unregister_event_callback(int registration_id) {
  std::lock_guard<std::mutex> lock(g_callback_mutex);

  auto it = g_event_callbacks.find(registration_id);
  if (it == g_event_callbacks.end()) {
    return false;
  }

  g_event_callbacks.erase(it);

  // Clean up event listener if no callbacks remain
  if (g_event_callbacks.empty()) {
    g_event_listener.reset();
  }

  return true;
}

FFI_PLUGIN_EXPORT
void native_window_manager_shutdown(void) {
  std::lock_guard<std::mutex> lock(g_callback_mutex);

  // Clear all callbacks
  g_event_callbacks.clear();

  // Clean up event listener
  g_event_listener.reset();

  // Note: We don't explicitly destroy the WindowManager singleton
  // as it will be cleaned up automatically when the application exits
}