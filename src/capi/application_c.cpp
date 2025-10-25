#include "application_c.h"
#include <memory>
#include <string>
#include <vector>

#include "../application.h"
#include "../window_manager.h"

using namespace nativeapi;

// Event listener class to bridge C++ events to C callbacks
class CApplicationEventListener {
 public:
  CApplicationEventListener(native_application_event_callback_t callback) : callback_(callback) {
    auto& app = Application::GetInstance();

    // Register for various application events
    app.AddListener<ApplicationStartedEvent>([this](const ApplicationStartedEvent& e) {
      native_application_event_t event;
      event.type = NATIVE_APPLICATION_EVENT_STARTED;
      event.exit_code = 0;
      DispatchEvent(event);
    });

    app.AddListener<ApplicationExitingEvent>([this](const ApplicationExitingEvent& e) {
      native_application_event_t event;
      event.type = NATIVE_APPLICATION_EVENT_EXITING;
      event.exit_code = e.GetExitCode();
      DispatchEvent(event);
    });

    app.AddListener<ApplicationActivatedEvent>([this](const ApplicationActivatedEvent& e) {
      native_application_event_t event;
      event.type = NATIVE_APPLICATION_EVENT_ACTIVATED;
      event.exit_code = 0;
      DispatchEvent(event);
    });

    app.AddListener<ApplicationDeactivatedEvent>([this](const ApplicationDeactivatedEvent& e) {
      native_application_event_t event;
      event.type = NATIVE_APPLICATION_EVENT_DEACTIVATED;
      event.exit_code = 0;
      DispatchEvent(event);
    });

    app.AddListener<ApplicationQuitRequestedEvent>([this](const ApplicationQuitRequestedEvent& e) {
      native_application_event_t event;
      event.type = NATIVE_APPLICATION_EVENT_QUIT_REQUESTED;
      event.exit_code = 0;
      DispatchEvent(event);
    });
  }

 private:
  native_application_event_callback_t callback_;

  void DispatchEvent(const native_application_event_t& event) {
    if (callback_) {
      callback_(&event);
    }
  }
};

// Global registry for event listeners
static std::unordered_map<size_t, std::unique_ptr<CApplicationEventListener>> g_listeners;
static size_t g_next_listener_id = 1;

extern "C" {

native_application_t native_application_get_instance(void) {
  return &Application::GetInstance();
}

int native_application_run(native_application_t app) {
  if (!app) {
    return -1;
  }

  Application* cpp_app = static_cast<Application*>(app);
  return cpp_app->Run();
}

int native_application_run_with_window(native_application_t app, native_window_t window) {
  if (!app || !window) {
    return -1;
  }

  Application* cpp_app = static_cast<Application*>(app);

  // Convert native_window_t back to shared_ptr<Window>
  Window* window_ptr = static_cast<Window*>(window);
  WindowId window_id = window_ptr->GetId();

  // Retrieve the shared_ptr from WindowManager
  auto& manager = WindowManager::GetInstance();
  auto window_shared = manager.Get(window_id);

  if (!window_shared) {
    return -1;
  }

  return cpp_app->Run(window_shared);
}

void native_application_quit(native_application_t app, int exit_code) {
  if (!app) {
    return;
  }

  Application* cpp_app = static_cast<Application*>(app);
  cpp_app->Quit(exit_code);
}

bool native_application_is_running(native_application_t app) {
  if (!app) {
    return false;
  }

  Application* cpp_app = static_cast<Application*>(app);
  return cpp_app->IsRunning();
}

bool native_application_is_single_instance(native_application_t app) {
  if (!app) {
    return false;
  }

  Application* cpp_app = static_cast<Application*>(app);
  return cpp_app->IsSingleInstance();
}

bool native_application_set_icon(native_application_t app, const char* icon_path) {
  if (!app || !icon_path) {
    return false;
  }

  Application* cpp_app = static_cast<Application*>(app);
  return cpp_app->SetIcon(icon_path);
}

bool native_application_set_dock_icon_visible(native_application_t app, bool visible) {
  if (!app) {
    return false;
  }

  Application* cpp_app = static_cast<Application*>(app);
  return cpp_app->SetDockIconVisible(visible);
}

size_t native_application_add_event_listener(native_application_t app,
                                             native_application_event_callback_t callback) {
  if (!app || !callback) {
    return 0;
  }

  size_t listener_id = g_next_listener_id++;
  g_listeners[listener_id] = std::make_unique<CApplicationEventListener>(callback);

  return listener_id;
}

bool native_application_remove_event_listener(native_application_t app, size_t listener_id) {
  if (!app || listener_id == 0) {
    return false;
  }

  auto it = g_listeners.find(listener_id);
  if (it != g_listeners.end()) {
    g_listeners.erase(it);
    return true;
  }

  return false;
}

int native_run_app(native_window_t window) {
  native_application_t app = native_application_get_instance();
  return native_application_run_with_window(app, window);
}

}  // extern "C"
