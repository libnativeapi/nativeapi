#include <cstring>
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>
#include <set>

#include "../../window.h"
#include "../../window_manager.h"

// Import GTK headers
#include <gdk/gdk.h>
#include <gtk/gtk.h>

namespace nativeapi {

// Shared static variables for window ID mapping
static std::unordered_map<GdkWindow*, WindowId> g_window_id_map;
static std::mutex g_map_mutex;

// Track widgets that have been hooked to avoid duplicate connections
static std::set<GtkWidget*> g_hooked_widgets;
static std::mutex g_hook_mutex;

// Flag to indicate if global swizzling has been installed
static bool g_swizzle_installed = false;

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

// Forward declarations for swizzling functions
static void InstallShowHideHooks(GtkWidget* widget);
static void InstallGlobalSwizzling();

// Signal emission hook for show signal
static gboolean on_show_emission_hook(GSignalInvocationHint* ihint,
                                       guint n_param_values,
                                       const GValue* param_values,
                                       gpointer data) {
  (void)ihint;
  (void)n_param_values;
  (void)data;

  GtkWidget* widget = GTK_WIDGET(g_value_get_object(&param_values[0]));
  if (widget && GTK_IS_WINDOW(widget)) {
    GdkWindow* gdk_window = gtk_widget_get_window(widget);
    if (gdk_window) {
      WindowId id = GetOrCreateWindowId(gdk_window);
      WindowManager::GetInstance().InvokeWillShowHook(id);
    }
  }

  return TRUE;  // Continue emission
}

// Signal emission hook for hide signal
static gboolean on_hide_emission_hook(GSignalInvocationHint* ihint,
                                       guint n_param_values,
                                       const GValue* param_values,
                                       gpointer data) {
  (void)ihint;
  (void)n_param_values;
  (void)data;

  GtkWidget* widget = GTK_WIDGET(g_value_get_object(&param_values[0]));
  if (widget && GTK_IS_WINDOW(widget)) {
    GdkWindow* gdk_window = gtk_widget_get_window(widget);
    if (gdk_window) {
      WindowId id = GetOrCreateWindowId(gdk_window);
      WindowManager::GetInstance().InvokeWillHideHook(id);
    }
  }

  return TRUE;  // Continue emission
}

// GTK signal callbacks to invoke hooks (used as fallback)
static gboolean OnGtkMapEvent(GtkWidget* widget, GdkEvent* event, gpointer user_data) {
  (void)event;
  (void)user_data;

  if (GTK_IS_WINDOW(widget)) {
    auto& manager = WindowManager::GetInstance();
    GdkWindow* gdk_window = gtk_widget_get_window(widget);
    if (gdk_window) {
      WindowId id = GetOrCreateWindowId(gdk_window);
      manager.InvokeWillShowHook(id);
    }
  }
  // Return FALSE to propagate event further
  return FALSE;
}

static gboolean OnGtkUnmapEvent(GtkWidget* widget, GdkEvent* event, gpointer user_data) {
  (void)event;
  (void)user_data;

  if (GTK_IS_WINDOW(widget)) {
    auto& manager = WindowManager::GetInstance();
    GdkWindow* gdk_window = gtk_widget_get_window(widget);
    if (gdk_window) {
      WindowId id = GetOrCreateWindowId(gdk_window);
      manager.InvokeWillHideHook(id);
    }
  }
  // Return FALSE to propagate event further
  return FALSE;
}

// Install hooks for a specific widget
static void InstallShowHideHooks(GtkWidget* widget) {
  if (!widget || !GTK_IS_WINDOW(widget)) {
    return;
  }

  std::lock_guard<std::mutex> lock(g_hook_mutex);

  // Check if already hooked
  if (g_hooked_widgets.find(widget) != g_hooked_widgets.end()) {
    return;
  }

  // Connect map/unmap events as fallback
  g_signal_connect(G_OBJECT(widget), "map-event", G_CALLBACK(OnGtkMapEvent), nullptr);
  g_signal_connect(G_OBJECT(widget), "unmap-event", G_CALLBACK(OnGtkUnmapEvent), nullptr);

  g_hooked_widgets.insert(widget);
}

// Install global swizzling using signal emission hooks
static void InstallGlobalSwizzling() {
  if (g_swizzle_installed) {
    return;
  }

  // Get the show and hide signal IDs for GtkWidget
  guint show_signal_id = g_signal_lookup("show", GTK_TYPE_WIDGET);
  guint hide_signal_id = g_signal_lookup("hide", GTK_TYPE_WIDGET);

  if (show_signal_id != 0) {
    // Add emission hook for show signal
    g_signal_add_emission_hook(show_signal_id, 0, on_show_emission_hook, nullptr, nullptr);
  }

  if (hide_signal_id != 0) {
    // Add emission hook for hide signal
    g_signal_add_emission_hook(hide_signal_id, 0, on_hide_emission_hook, nullptr, nullptr);
  }

  g_swizzle_installed = true;
}

// Private implementation for Linux
class WindowManager::Impl {
 public:
  Impl(WindowManager* manager) : manager_(manager) {}
  ~Impl() {}

  void SetupEventMonitoring() {
    // Install global swizzling for show/hide interception
    InstallGlobalSwizzling();

    // Monitor all existing windows
    GdkDisplay* display = gdk_display_get_default();
    if (display) {
      GList* toplevels = gtk_window_list_toplevels();
      for (GList* l = toplevels; l != nullptr; l = l->next) {
        GtkWindow* gtk_window = GTK_WINDOW(l->data);
        InstallShowHideHooks(GTK_WIDGET(gtk_window));
      }
      g_list_free(toplevels);
    }
  }

  void CleanupEventMonitoring() {
    // Clear hooked widgets set
    std::lock_guard<std::mutex> lock(g_hook_mutex);
    g_hooked_widgets.clear();
  }

 private:
  WindowManager* manager_;
  // Optional pre-show/hide hooks
  std::optional<WindowManager::WindowWillShowHook> will_show_hook_;
  std::optional<WindowManager::WindowWillHideHook> will_hide_hook_;

  friend class WindowManager;
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

  // Install show/hide hooks for this window
  InstallShowHideHooks(gtk_window);

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

  // Realize to ensure GdkWindow exists before show, so we can derive ID and invoke hook
  if (!gtk_widget_get_realized(gtk_window)) {
    gtk_widget_realize(gtk_window);
  }

  // Obtain GdkWindow and compute WindowId
  GdkWindow* gdk_window = gtk_widget_get_window(gtk_window);
  if (!gdk_window) {
    std::cerr << "Failed to get GdkWindow from GTK widget" << std::endl;
    gtk_widget_destroy(gtk_window);
    return nullptr;
  }

  // Create our Window wrapper and cache before showing
  auto window = std::make_shared<Window>((void*)gdk_window);
  WindowId window_id = GetOrCreateWindowId(gdk_window);
  windows_[window_id] = window;

  // Invoke pre-show hook if set
  InvokeWillShowHook(window_id);

  // Show the window after invoking hook
  gtk_widget_show(gtk_window);

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
  pimpl_->will_show_hook_ = std::move(hook);
  if (pimpl_->will_show_hook_) {
    // Ensure global swizzling is installed when hook is set
    InstallGlobalSwizzling();
  }
}

void WindowManager::SetWillHideHook(std::optional<WindowWillHideHook> hook) {
  pimpl_->will_hide_hook_ = std::move(hook);
  if (pimpl_->will_hide_hook_) {
    // Ensure global swizzling is installed when hook is set
    InstallGlobalSwizzling();
  }
}

void WindowManager::InvokeWillShowHook(WindowId id) {
  if (pimpl_->will_show_hook_) {
    (*pimpl_->will_show_hook_)(id);
  }
}

void WindowManager::InvokeWillHideHook(WindowId id) {
  if (pimpl_->will_hide_hook_) {
    (*pimpl_->will_hide_hook_)(id);
  }
}

}  // namespace nativeapi
