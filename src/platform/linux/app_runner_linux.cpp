#include <iostream>

#include "../../app_runner.h"
#include "../../window.h"

// Import GTK headers
#include <gdk/gdk.h>
#include <gtk/gtk.h>

namespace nativeapi {

// Private implementation for Linux using GTK
class AppRunner::Impl {
 public:
  Impl() {
    // GTK initialization is handled by WindowManager
    // No need to initialize again here
  }

  ~Impl() {
    // GTK cleanup is handled automatically
  }

  int Run(std::shared_ptr<Window> window) {
    if (!window) {
      std::cerr << "Cannot run application with null window" << std::endl;
      return -1;
    }

    // Check if GTK is available
    GdkDisplay* display = gdk_display_get_default();
    if (!display) {
      std::cerr << "No display available - cannot run application" << std::endl;
      return -1;
    }

    std::cout << "Starting application with window ID: " << window->GetId()
              << std::endl;

    // Connect to the window destroy signal to quit the main loop
    // when the window is closed
    if (window->GetId() != 0) {
      // Find the GTK window corresponding to our Window object
      GList* toplevels = gtk_window_list_toplevels();
      for (GList* l = toplevels; l != nullptr; l = l->next) {
        GtkWindow* gtk_window = GTK_WINDOW(l->data);
        GdkWindow* gdk_window = gtk_widget_get_window(GTK_WIDGET(gtk_window));

        if (gdk_window && (WindowID)gdk_window == window->GetId()) {
          // Connect destroy signal to quit the main loop
          g_signal_connect(gtk_window, "destroy", G_CALLBACK(gtk_main_quit),
                           nullptr);
          break;
        }
      }
      g_list_free(toplevels);
    }

    // Run the GTK main loop
    std::cout << "Starting GTK main loop..." << std::endl;
    gtk_main();

    std::cout << "Application finished" << std::endl;
    return 0;
  }
};

// AppRunner implementation
AppRunner::AppRunner() : pimpl_(std::make_unique<Impl>()) {}

AppRunner::~AppRunner() = default;

int AppRunner::Run(std::shared_ptr<Window> window) {
  return pimpl_->Run(window);
}

}  // namespace nativeapi
