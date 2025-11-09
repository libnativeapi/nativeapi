#include <cstring>
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>

#include "../../window.h"
#include "../../window_manager.h"

// Import GTK headers
#include <gdk/gdk.h>
#include <gtk/gtk.h>

namespace nativeapi {

// Shared static variables for window ID mapping
static std::unordered_map<GdkWindow*, WindowId> g_window_id_map;
static std::mutex g_map_mutex;

// Helper function to manage mapping between GdkWindow pointers and WindowIds
static WindowId GetOrCreateWindowId(GdkWindow* gdk_window) {
  if (!gdk_window) {
    return IdAllocator::kInvalidId;
  }

  std::lock_guard<std::mutex> lock(g_map_mutex);
  auto it = g_window_id_map.find(gdk_window);
  if (it != g_window_id_map.end()) {
    return it->second;
  }

  // Allocate new ID using the IdAllocator
  WindowId new_id = IdAllocator::Allocate<Window>();
  if (new_id != IdAllocator::kInvalidId) {
    g_window_id_map[gdk_window] = new_id;
  }
  return new_id;
}

// Helper function to find GdkWindow by WindowId
static GdkWindow* FindGdkWindowById(WindowId id) {
  std::lock_guard<std::mutex> lock(g_map_mutex);
  for (const auto& pair : g_window_id_map) {
    if (pair.second == id) {
      return pair.first;
    }
  }
  return nullptr;
}

// Private implementation for Linux (stub for now)
class WindowManager::Impl {
 public:
  Impl(WindowManager* manager) : manager_(manager) {}
  ~Impl() {}

  void SetupEventMonitoring() {
    // TODO: Implement Linux-specific event monitoring using GTK signals
  }

  void CleanupEventMonitoring() {
    // TODO: Implement Linux-specific cleanup
  }

 private:
  WindowManager* manager_;
};

WindowManager::WindowManager() : pimpl_(std::make_unique<Impl>(this)) {
  // Try to initialize GTK if not already initialized
  // In headless environments, this may fail, which is acceptable
  if (!gdk_display_get_default()) {
    // Temporarily redirect stderr to suppress GTK warnings in headless
    // environments
    FILE* original_stderr = stderr;
    freopen("/dev/null", "w", stderr);

    gboolean gtk_result = gtk_init_check(nullptr, nullptr);

    // Restore stderr
    fflush(stderr);
    freopen("/dev/tty", "w", stderr);
    stderr = original_stderr;

    // gtk_init_check returns FALSE if initialization failed (e.g., no display)
    // This is acceptable for headless environments
  }

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

  // Try to find the window by ID in the current display
  GdkDisplay* display = gdk_display_get_default();
  if (!display) {
    return nullptr;
  }

  // Get all toplevel windows
  GList* toplevels = gtk_window_list_toplevels();
  for (GList* l = toplevels; l != nullptr; l = l->next) {
    GtkWindow* gtk_window = GTK_WINDOW(l->data);
    GdkWindow* gdk_window = gtk_widget_get_window(GTK_WIDGET(gtk_window));

    if (gdk_window && GetOrCreateWindowId(gdk_window) == id) {
      auto window = std::make_shared<Window>((void*)gdk_window);
      windows_[id] = window;
      g_list_free(toplevels);
      return window;
    }
  }
  g_list_free(toplevels);
  return nullptr;
}

std::vector<std::shared_ptr<Window>> WindowManager::GetAll() {
  std::vector<std::shared_ptr<Window>> windows;

  GdkDisplay* display = gdk_display_get_default();
  if (!display) {
    return windows;
  }

  // Get all toplevel windows
  GList* toplevels = gtk_window_list_toplevels();
  for (GList* l = toplevels; l != nullptr; l = l->next) {
    GtkWindow* gtk_window = GTK_WINDOW(l->data);
    GdkWindow* gdk_window = gtk_widget_get_window(GTK_WIDGET(gtk_window));

    if (gdk_window) {
      WindowId window_id = GetOrCreateWindowId(gdk_window);
      auto it = windows_.find(window_id);
      if (it == windows_.end()) {
        auto window = std::make_shared<Window>((void*)gdk_window);
        windows_[window_id] = window;
      }
    }
  }
  g_list_free(toplevels);

  // Return all cached windows
  for (auto& window : windows_) {
    windows.push_back(window.second);
  }
  return windows;
}

std::shared_ptr<Window> WindowManager::GetCurrent() {
  GdkDisplay* display = gdk_display_get_default();
  if (!display) {
    return nullptr;
  }

  // Try to get the focused window
  GdkSeat* seat = gdk_display_get_default_seat(display);
  if (seat) {
    GdkDevice* keyboard = gdk_seat_get_keyboard(seat);
    if (keyboard) {
      GdkWindow* focused_window = gdk_device_get_window_at_position(keyboard, nullptr, nullptr);
      if (focused_window) {
        WindowId window_id = GetOrCreateWindowId(focused_window);
        return Get(window_id);
      }
    }
  }

  // Fallback: get the first visible window
  GList* toplevels = gtk_window_list_toplevels();
  for (GList* l = toplevels; l != nullptr; l = l->next) {
    GtkWindow* gtk_window = GTK_WINDOW(l->data);
    if (gtk_widget_get_visible(GTK_WIDGET(gtk_window))) {
      GdkWindow* gdk_window = gtk_widget_get_window(GTK_WIDGET(gtk_window));
      if (gdk_window) {
        WindowId window_id = GetOrCreateWindowId(gdk_window);
        g_list_free(toplevels);
        return Get(window_id);
      }
    }
  }
  g_list_free(toplevels);

  return nullptr;
}

std::shared_ptr<Window> WindowManager::Create(const WindowOptions& options) {
  // Check if GTK is available
  GdkDisplay* display = gdk_display_get_default();
  if (!display) {
    std::cerr << "No display available for window creation" << std::endl;
    return nullptr;
  }

  // Create a new GTK window
  GtkWidget* gtk_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  if (!gtk_window) {
    std::cerr << "Failed to create GTK window" << std::endl;
    return nullptr;
  }

  // Set window properties from options
  if (!options.title.empty()) {
    gtk_window_set_title(GTK_WINDOW(gtk_window), options.title.c_str());
  }

  // Set window size
  if (options.size.width > 0 && options.size.height > 0) {
    gtk_window_set_default_size(GTK_WINDOW(gtk_window), options.size.width, options.size.height);
  }

  // Set minimum size if specified
  if (options.minimum_size.width > 0 && options.minimum_size.height > 0) {
    GdkGeometry geometry;
    geometry.min_width = options.minimum_size.width;
    geometry.min_height = options.minimum_size.height;
    gtk_window_set_geometry_hints(GTK_WINDOW(gtk_window), nullptr, &geometry, GDK_HINT_MIN_SIZE);
  }

  // Set maximum size if specified
  if (options.maximum_size.width > 0 && options.maximum_size.height > 0) {
    GdkGeometry geometry;
    geometry.max_width = options.maximum_size.width;
    geometry.max_height = options.maximum_size.height;
    gtk_window_set_geometry_hints(GTK_WINDOW(gtk_window), nullptr, &geometry, GDK_HINT_MAX_SIZE);
  }

  // Center the window if requested
  if (options.centered) {
    gtk_window_set_position(GTK_WINDOW(gtk_window), GTK_WIN_POS_CENTER);
  }

  // Show the window
  gtk_widget_show(gtk_window);

  // Get the GdkWindow after the widget is realized
  GdkWindow* gdk_window = gtk_widget_get_window(gtk_window);
  if (!gdk_window) {
    // If window is not yet realized, realize it first
    gtk_widget_realize(gtk_window);
    gdk_window = gtk_widget_get_window(gtk_window);
  }

  if (!gdk_window) {
    std::cerr << "Failed to get GdkWindow from GTK widget" << std::endl;
    gtk_widget_destroy(gtk_window);
    return nullptr;
  }

  // Create our Window wrapper
  auto window = std::make_shared<Window>((void*)gdk_window);
  WindowId window_id = GetOrCreateWindowId(gdk_window);

  // Store in our cache
  windows_[window_id] = window;

  std::cout << "Created window with ID: " << window_id << std::endl;

  return window;
}

bool WindowManager::Destroy(WindowId id) {
  auto it = windows_.find(id);
  if (it != windows_.end()) {
    // TODO: Implement proper GTK window destruction
    windows_.erase(it);
    return true;
  }
  return false;
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
