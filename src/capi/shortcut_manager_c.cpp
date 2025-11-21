#include "shortcut_manager_c.h"
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include "../shortcut_manager.h"

using namespace nativeapi;

// Global state for shortcut callbacks
struct ShortcutCallbackInfo {
  native_shortcut_callback_t callback;
  void* user_data;
};

static std::mutex g_shortcut_callback_mutex;
static std::unordered_map<native_shortcut_id_t, ShortcutCallbackInfo> g_shortcut_callbacks;

// Global state for event callbacks
struct ShortcutEventCallbackInfo {
  native_shortcut_event_callback_t callback;
  void* user_data;
  int id;
};

static std::mutex g_shortcut_event_callback_mutex;
static std::unordered_map<int, ShortcutEventCallbackInfo> g_shortcut_event_callbacks;
static int g_shortcut_next_callback_id = 1;

// Helper function to create native_shortcut_t from shared_ptr<Shortcut>
static native_shortcut_t CreateNativeShortcutHandle(std::shared_ptr<Shortcut> shortcut) {
  if (!shortcut)
    return nullptr;
  // Return raw pointer - the ShortcutManager maintains the actual shared_ptr
  return static_cast<void*>(shortcut.get());
}

// Helper function to dispatch events to registered callbacks
static void DispatchEvent(const native_shortcut_event_t& event) {
  std::lock_guard<std::mutex> lock(g_shortcut_event_callback_mutex);

  for (const auto& [id, callback_info] : g_shortcut_event_callbacks) {
    try {
      callback_info.callback(&event, callback_info.user_data);
    } catch (...) {
      // Ignore exceptions from callbacks
    }
  }
}

// Event listener class to bridge C++ events to C callbacks
class ShortcutCEventListener {
 public:
  ShortcutCEventListener() {
    auto& manager = ShortcutManager::GetInstance();

    // Register for shortcut events
    manager.AddListener<ShortcutActivatedEvent>([this](const ShortcutActivatedEvent& e) {
      native_shortcut_event_t event;
      event.type = NATIVE_SHORTCUT_EVENT_ACTIVATED;
      event.shortcut_id = e.GetShortcutId();
      event.accelerator = e.GetAccelerator().c_str();
      DispatchEvent(event);
    });

    manager.AddListener<ShortcutRegisteredEvent>([this](const ShortcutRegisteredEvent& e) {
      native_shortcut_event_t event;
      event.type = NATIVE_SHORTCUT_EVENT_REGISTERED;
      event.shortcut_id = e.GetShortcutId();
      event.accelerator = e.GetAccelerator().c_str();
      DispatchEvent(event);
    });

    manager.AddListener<ShortcutUnregisteredEvent>([this](const ShortcutUnregisteredEvent& e) {
      native_shortcut_event_t event;
      event.type = NATIVE_SHORTCUT_EVENT_UNREGISTERED;
      event.shortcut_id = e.GetShortcutId();
      event.accelerator = e.GetAccelerator().c_str();
      DispatchEvent(event);
    });

    manager.AddListener<ShortcutRegistrationFailedEvent>(
        [this](const ShortcutRegistrationFailedEvent& e) {
          native_shortcut_event_t event;
          event.type = NATIVE_SHORTCUT_EVENT_REGISTRATION_FAILED;
          event.shortcut_id = e.GetShortcutId();
          event.accelerator = e.GetAccelerator().c_str();
          event.data.registration_failed.error_message = e.GetErrorMessage().c_str();
          DispatchEvent(event);
        });
  }
};

// Singleton event listener
static ShortcutCEventListener* GetEventListener() {
  static ShortcutCEventListener listener;
  return &listener;
}

void native_shortcut_list_free(native_shortcut_list_t list) {
  if (list.shortcuts) {
    delete[] list.shortcuts;
  }
}

bool native_shortcut_manager_is_supported(void) {
  return ShortcutManager::GetInstance().IsSupported();
}

native_shortcut_t native_shortcut_manager_register(const char* accelerator,
                                                   native_shortcut_callback_t callback,
                                                   void* user_data) {
  if (!accelerator || !callback)
    return nullptr;

  auto& manager = ShortcutManager::GetInstance();

  // Create C++ callback that calls the C callback
  auto cpp_callback = [callback, user_data](ShortcutId id) {
    try {
      callback(id, user_data);
    } catch (...) {
      // Ignore exceptions from callbacks
    }
  };

  auto shortcut = manager.Register(accelerator, [callback, user_data]() {
    // We need to get the shortcut ID from somewhere...
    // For now, we'll store the callback info and invoke it when needed
  });

  if (shortcut) {
    // Store callback info
    std::lock_guard<std::mutex> lock(g_shortcut_callback_mutex);
    g_shortcut_callbacks[shortcut->GetId()] = {callback, user_data};

    // Update the shortcut's callback to call our C callback
    shortcut->SetCallback([callback, user_data, id = shortcut->GetId()]() {
      try {
        callback(id, user_data);
      } catch (...) {
        // Ignore exceptions from callbacks
      }
    });
  }

  return CreateNativeShortcutHandle(shortcut);
}

native_shortcut_t native_shortcut_manager_register_with_options(
    const native_shortcut_options_t* options,
    native_shortcut_callback_t callback,
    void* user_data) {
  if (!options || !options->accelerator || !callback)
    return nullptr;

  auto& manager = ShortcutManager::GetInstance();

  // Convert C options to C++ options
  ShortcutOptions cpp_options;
  cpp_options.accelerator = options->accelerator;
  cpp_options.description = options->description ? options->description : "";
  cpp_options.scope = options->scope == NATIVE_SHORTCUT_SCOPE_GLOBAL ? ShortcutScope::Global
                                                                     : ShortcutScope::Application;
  cpp_options.enabled = options->enabled;

  // Placeholder callback - will be replaced below
  cpp_options.callback = []() {};

  auto shortcut = manager.Register(cpp_options);

  if (shortcut) {
    // Store callback info
    std::lock_guard<std::mutex> lock(g_shortcut_callback_mutex);
    g_shortcut_callbacks[shortcut->GetId()] = {callback, user_data};

    // Update the shortcut's callback to call our C callback
    shortcut->SetCallback([callback, user_data, id = shortcut->GetId()]() {
      try {
        callback(id, user_data);
      } catch (...) {
        // Ignore exceptions from callbacks
      }
    });
  }

  return CreateNativeShortcutHandle(shortcut);
}

bool native_shortcut_manager_unregister_by_id(native_shortcut_id_t shortcut_id) {
  auto& manager = ShortcutManager::GetInstance();

  // Remove callback info
  {
    std::lock_guard<std::mutex> lock(g_shortcut_callback_mutex);
    g_shortcut_callbacks.erase(shortcut_id);
  }

  return manager.Unregister(shortcut_id);
}

bool native_shortcut_manager_unregister_by_accelerator(const char* accelerator) {
  if (!accelerator)
    return false;

  auto& manager = ShortcutManager::GetInstance();

  // Get the shortcut to find its ID
  auto shortcut = manager.Get(accelerator);
  if (shortcut) {
    // Remove callback info
    std::lock_guard<std::mutex> lock(g_shortcut_callback_mutex);
    g_shortcut_callbacks.erase(shortcut->GetId());
  }

  return manager.Unregister(accelerator);
}

int native_shortcut_manager_unregister_all(void) {
  auto& manager = ShortcutManager::GetInstance();

  // Clear all callback info
  {
    std::lock_guard<std::mutex> lock(g_shortcut_callback_mutex);
    g_shortcut_callbacks.clear();
  }

  return manager.UnregisterAll();
}

native_shortcut_t native_shortcut_manager_get_by_id(native_shortcut_id_t shortcut_id) {
  auto& manager = ShortcutManager::GetInstance();
  auto shortcut = manager.Get(shortcut_id);
  return CreateNativeShortcutHandle(shortcut);
}

native_shortcut_t native_shortcut_manager_get_by_accelerator(const char* accelerator) {
  if (!accelerator)
    return nullptr;

  auto& manager = ShortcutManager::GetInstance();
  auto shortcut = manager.Get(accelerator);
  return CreateNativeShortcutHandle(shortcut);
}

native_shortcut_list_t native_shortcut_manager_get_all(void) {
  auto& manager = ShortcutManager::GetInstance();
  auto shortcuts = manager.GetAll();

  native_shortcut_list_t list;
  list.count = shortcuts.size();

  if (list.count > 0) {
    list.shortcuts = new native_shortcut_t[list.count];
    for (size_t i = 0; i < list.count; i++) {
      list.shortcuts[i] = CreateNativeShortcutHandle(shortcuts[i]);
    }
  } else {
    list.shortcuts = nullptr;
  }

  return list;
}

native_shortcut_list_t native_shortcut_manager_get_by_scope(native_shortcut_scope_t scope) {
  auto& manager = ShortcutManager::GetInstance();

  ShortcutScope cpp_scope =
      scope == NATIVE_SHORTCUT_SCOPE_GLOBAL ? ShortcutScope::Global : ShortcutScope::Application;

  auto shortcuts = manager.GetByScope(cpp_scope);

  native_shortcut_list_t list;
  list.count = shortcuts.size();

  if (list.count > 0) {
    list.shortcuts = new native_shortcut_t[list.count];
    for (size_t i = 0; i < list.count; i++) {
      list.shortcuts[i] = CreateNativeShortcutHandle(shortcuts[i]);
    }
  } else {
    list.shortcuts = nullptr;
  }

  return list;
}

bool native_shortcut_manager_is_available(const char* accelerator) {
  if (!accelerator)
    return false;

  auto& manager = ShortcutManager::GetInstance();
  return manager.IsAvailable(accelerator);
}

bool native_shortcut_manager_is_valid_accelerator(const char* accelerator) {
  if (!accelerator)
    return false;

  auto& manager = ShortcutManager::GetInstance();
  return manager.IsValidAccelerator(accelerator);
}

void native_shortcut_manager_set_enabled(bool enabled) {
  auto& manager = ShortcutManager::GetInstance();
  manager.SetEnabled(enabled);
}

bool native_shortcut_manager_is_enabled(void) {
  auto& manager = ShortcutManager::GetInstance();
  return manager.IsEnabled();
}

int native_shortcut_manager_register_event_callback(native_shortcut_event_callback_t callback,
                                                    void* user_data) {
  if (!callback)
    return -1;

  // Ensure event listener is initialized
  GetEventListener();

  std::lock_guard<std::mutex> lock(g_shortcut_event_callback_mutex);

  int id = g_shortcut_next_callback_id++;
  g_shortcut_event_callbacks[id] = {callback, user_data, id};

  return id;
}

bool native_shortcut_manager_unregister_event_callback(int registration_id) {
  std::lock_guard<std::mutex> lock(g_shortcut_event_callback_mutex);

  auto it = g_shortcut_event_callbacks.find(registration_id);
  if (it == g_shortcut_event_callbacks.end()) {
    return false;
  }

  g_shortcut_event_callbacks.erase(it);
  return true;
}
