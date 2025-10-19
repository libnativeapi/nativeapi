#include <gtk/gtk.h>
#include <glib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <iostream>

#include "../../application.h"
#include "../../menu.h"
#include "../../window_manager.h"

namespace nativeapi {

class Application::Impl {
 public:
  Impl(Application* app) : app_(app), gtk_app_(nullptr), lock_file_handle_(-1) {}
  ~Impl() = default;

  bool Initialize() {
    // Initialize GTK
    gtk_init(nullptr, nullptr);

    // Create GTK application with default ID
    gtk_app_ = gtk_application_new("com.nativeapi.application", G_APPLICATION_FLAGS_NONE);
    
    if (!gtk_app_) {
      return false;
    }

    // Set default application name
    g_object_set(gtk_app_, "application-name", "NativeAPI Application", nullptr);

    // Connect to GTK application signals
    g_signal_connect(gtk_app_, "startup", G_CALLBACK(OnStartup), this);
    g_signal_connect(gtk_app_, "activate", G_CALLBACK(OnActivate), this);
    g_signal_connect(gtk_app_, "shutdown", G_CALLBACK(OnShutdown), this);

    return true;
  }

  int Run() {
    // Run the GTK main loop
    int status = g_application_run(G_APPLICATION(gtk_app_), 0, nullptr);
    
    return status;
  }

  void Quit(int exit_code) {
    g_application_quit(G_APPLICATION(gtk_app_));
  }

  bool SetIcon(const std::string& icon_path) {
    if (icon_path.empty()) {
      return false;
    }

    // Load icon from file
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file(icon_path.c_str(), nullptr);
    if (!pixbuf) {
      return false;
    }

    // Set application icon
    gtk_window_set_default_icon(pixbuf);
    
    g_object_unref(pixbuf);
    return true;
  }

  bool SetDockIconVisible(bool visible) {
    // Linux doesn't have a dock in the same way as macOS
    // This is a no-op for now
    return true;
  }

  bool SetMenuBar(std::shared_ptr<Menu> menu) {
    if (!menu) {
      return false;
    }

    // Get the native menu handle
    GtkWidget* gtk_menu = static_cast<GtkWidget*>(menu->GetNativeObject());
    if (!gtk_menu) {
      return false;
    }

    // Set the application menu
    gtk_application_set_app_menu(gtk_app_, GTK_MENU_MODEL(gtk_menu));
    
    return true;
  }


  void CleanupEventMonitoring() {
    // Clean up Linux-specific event monitoring
    if (lock_file_handle_ != -1) {
      close(lock_file_handle_);
      lock_file_handle_ = -1;
    }
    
    if (gtk_app_) {
      g_object_unref(gtk_app_);
      gtk_app_ = nullptr;
    }
  }

 private:
  Application* app_;
  GtkApplication* gtk_app_;
  int lock_file_handle_;

  static void OnStartup(GApplication* app, gpointer user_data) {
    Impl* impl = static_cast<Impl*>(user_data);
    
    // Emit application started event
    ApplicationStartedEvent event;
    impl->app_->Emit(event);
  }

  static void OnActivate(GApplication* app, gpointer user_data) {
    Impl* impl = static_cast<Impl*>(user_data);
    
    // Emit application activated event
    ApplicationActivatedEvent event;
    impl->app_->Emit(event);
  }

  static void OnShutdown(GApplication* app, gpointer user_data) {
    Impl* impl = static_cast<Impl*>(user_data);
    
    // Emit application exiting event
    ApplicationExitingEvent event(0);
    impl->app_->Emit(event);
  }
};

Application::Application() 
    : initialized_(false), running_(false), exit_code_(0), pimpl_(std::make_unique<Impl>(this)) {
}

Application::~Application() {
  CleanupEventMonitoring();
}

bool Application::Initialize() {
  if (initialized_) {
    return false;  // Already initialized
  }
  
  // Perform platform-specific initialization
  bool success = pimpl_->Initialize();
  
  if (success) {
    initialized_ = true;
    Emit<ApplicationStartedEvent>();
  }
  
  return success;
}

int Application::Run() {
  if (!initialized_) {
    return -1;  // Not initialized
  }

  running_ = true;
  
  // Start the platform-specific main event loop
  int result = pimpl_->Run();
  
  running_ = false;
  
  // Emit exit event
  Emit<ApplicationExitingEvent>(result);
  
  return result;
}

void Application::Quit(int exit_code) {
  exit_code_ = exit_code;
  
  // Emit quit requested event
  Emit<ApplicationQuitRequestedEvent>();
  
  // Request platform-specific quit
  pimpl_->Quit(exit_code);
}

bool Application::IsRunning() const {
  return running_;
}


bool Application::IsSingleInstance() const {
  return false;
}

bool Application::SetIcon(const std::string& icon_path) {
  return pimpl_->SetIcon(icon_path);
}

bool Application::SetDockIconVisible(bool visible) {
  return pimpl_->SetDockIconVisible(visible);
}

bool Application::SetMenuBar(std::shared_ptr<Menu> menu) {
  return pimpl_->SetMenuBar(menu);
}

std::shared_ptr<Window> Application::GetPrimaryWindow() const {
  return primary_window_;
}

void Application::SetPrimaryWindow(std::shared_ptr<Window> window) {
  primary_window_ = window;
}

std::vector<std::shared_ptr<Window>> Application::GetAllWindows() const {
  auto& window_manager = WindowManager::GetInstance();
  return window_manager.GetAll();
}

void Application::CleanupEventMonitoring() {
  pimpl_->CleanupEventMonitoring();
}

}  // namespace nativeapi
