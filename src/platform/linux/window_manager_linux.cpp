#include <cstring>
#include <iostream>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>

#include "../../window.h"
#include "../../window_manager.h"
#include "../../window_registry.h"

// Import GTK headers
#include <gdk/gdk.h>
#include <gtk/gtk.h>

namespace nativeapi {

// Key to store/retrieve WindowId on GObjects (must match window_linux.cpp)
static const char* kWindowIdKey = "NativeAPIWindowId";

// Shared static variables for window ID mapping
static std::unordered_map<GdkWindow*, WindowId> g_window_id_map;
static std::mutex g_map_mutex;

// Track widgets that have been hooked to avoid duplicate connections
static std::set<GtkWidget*> g_hooked_widgets;
static std::mutex g_hook_mutex;

// Flag to indicate if global swizzling has been installed
static bool g_swizzle_installed = false;

// Emission hook ids for the toplevel focus signals, so they can be removed again
static gulong g_focus_in_hook_id = 0;
static gulong g_focus_out_hook_id = 0;

// The emission hooks are free functions and cannot name the private
// WindowManager::Impl, so they dispatch through this trampoline, installed by
// Impl::StartEventListening().
using WindowFocusChangedFn = void (*)(void* impl, WindowId id, bool focused);
static WindowFocusChangedFn g_focus_changed_fn = nullptr;
static void* g_focus_changed_context = nullptr;

// Helper function to manage mapping between GdkWindow pointers and WindowIds
static WindowId GetOrCreateWindowId(GdkWindow* gdk_window) {
  if (!gdk_window) {
    return IdAllocator::kInvalidId;
  }

  // First, try to read ID attached to the GObject
  gpointer data = g_object_get_data(G_OBJECT(gdk_window), kWindowIdKey);
  if (data) {
    WindowId id = static_cast<WindowId>(reinterpret_cast<uintptr_t>(data));
    // Cache it in the map for faster lookup next time
    std::lock_guard<std::mutex> lock(g_map_mutex);
    g_window_id_map[gdk_window] = id;
    return id;
  }

  // Fallback to cached map
  {
    std::lock_guard<std::mutex> lock(g_map_mutex);
    auto it = g_window_id_map.find(gdk_window);
    if (it != g_window_id_map.end()) {
      return it->second;
    }
  }

  // Allocate new ID and attach to the GObject for consistency
  WindowId new_id = IdAllocator::Allocate<Window>();
  if (new_id != IdAllocator::kInvalidId) {
    g_object_set_data(G_OBJECT(gdk_window), kWindowIdKey,
                      reinterpret_cast<gpointer>(static_cast<uintptr_t>(new_id)));
    std::lock_guard<std::mutex> lock(g_map_mutex);
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

// GDK delivers focus-in/focus-out to the toplevel GdkWindow, so a GtkWindow
// receives these signals exactly when the window manager gives or takes
// keyboard focus. GTK also emits them for child widgets when the focus moves
// inside a window, hence the toplevel filter.
static gboolean HandleFocusEmission(const GValue* param_values, bool focused) {
  GtkWidget* widget = GTK_WIDGET(g_value_get_object(&param_values[0]));
  if (!widget || !GTK_IS_WINDOW(widget) || !gtk_widget_is_toplevel(widget)) {
    return TRUE;  // Continue emission
  }

  GdkWindow* gdk_window = gtk_widget_get_window(widget);
  if (!gdk_window || !g_focus_changed_fn || !g_focus_changed_context) {
    return TRUE;
  }

  WindowId id = GetOrCreateWindowId(gdk_window);
  if (id != IdAllocator::kInvalidId) {
    g_focus_changed_fn(g_focus_changed_context, id, focused);
  }
  return TRUE;  // Continue emission
}

static gboolean on_focus_in_emission_hook(GSignalInvocationHint* ihint,
                                          guint n_param_values,
                                          const GValue* param_values,
                                          gpointer data) {
  (void)ihint;
  (void)n_param_values;
  (void)data;
  return HandleFocusEmission(param_values, true);
}

static gboolean on_focus_out_emission_hook(GSignalInvocationHint* ihint,
                                           guint n_param_values,
                                           const GValue* param_values,
                                           gpointer data) {
  (void)ihint;
  (void)n_param_values;
  (void)data;
  return HandleFocusEmission(param_values, false);
}

static void InstallFocusHooks() {
  guint focus_in_signal_id = g_signal_lookup("focus-in-event", GTK_TYPE_WIDGET);
  guint focus_out_signal_id = g_signal_lookup("focus-out-event", GTK_TYPE_WIDGET);

  if (focus_in_signal_id != 0 && g_focus_in_hook_id == 0) {
    g_focus_in_hook_id = g_signal_add_emission_hook(focus_in_signal_id, 0,
                                                    on_focus_in_emission_hook, nullptr, nullptr);
  }
  if (focus_out_signal_id != 0 && g_focus_out_hook_id == 0) {
    g_focus_out_hook_id = g_signal_add_emission_hook(focus_out_signal_id, 0,
                                                     on_focus_out_emission_hook, nullptr, nullptr);
  }
}

static void RemoveFocusHooks() {
  guint focus_in_signal_id = g_signal_lookup("focus-in-event", GTK_TYPE_WIDGET);
  guint focus_out_signal_id = g_signal_lookup("focus-out-event", GTK_TYPE_WIDGET);

  if (g_focus_in_hook_id != 0 && focus_in_signal_id != 0) {
    g_signal_remove_emission_hook(focus_in_signal_id, g_focus_in_hook_id);
  }
  if (g_focus_out_hook_id != 0 && focus_out_signal_id != 0) {
    g_signal_remove_emission_hook(focus_out_signal_id, g_focus_out_hook_id);
  }
  g_focus_in_hook_id = 0;
  g_focus_out_hook_id = 0;
}

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
      WindowManager::GetInstance().HandleWillShow(id);
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
      WindowManager::GetInstance().HandleWillHide(id);
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
      manager.HandleWillShow(id);
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
      manager.HandleWillHide(id);
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

  void StartEventListening() {
    // Install global swizzling for show/hide interception
    InstallGlobalSwizzling();

    // Install the toplevel focus hooks that drive focused/blurred events
    g_focus_changed_context = this;
    g_focus_changed_fn = [](void* impl, WindowId id, bool focused) {
      static_cast<Impl*>(impl)->OnWindowFocusChanged(id, focused);
    };
    InstallFocusHooks();

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

  void StopEventListening() {
    RemoveFocusHooks();
    g_focus_changed_fn = nullptr;
    g_focus_changed_context = nullptr;

    // Clear hooked widgets set
    std::lock_guard<std::mutex> lock(g_hook_mutex);
    g_hooked_widgets.clear();
  }

  void OnWindowFocusChanged(WindowId window_id, bool focused) {
    if (focused) {
      WindowFocusedEvent event(window_id);
      manager_->DispatchWindowEvent(event);
    } else {
      WindowBlurredEvent event(window_id);
      manager_->DispatchWindowEvent(event);
    }
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

  StartEventListening();
}

WindowManager::~WindowManager() {
  StopEventListening();
}

std::shared_ptr<Window> WindowManager::Get(WindowId id) {
  auto cached = WindowRegistry::GetInstance().Get(id);
  if (cached) {
    return cached;
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
      WindowRegistry::GetInstance().Add(id, window);
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
      if (!WindowRegistry::GetInstance().Get(window_id)) {
        auto window = std::make_shared<Window>((void*)gdk_window);
        WindowRegistry::GetInstance().Add(window_id, window);
      }
    }
  }
  g_list_free(toplevels);

  // Return all cached windows
  return WindowRegistry::GetInstance().GetAll();
}

namespace {

bool GdkWindowContainsPoint(GdkWindow* gdk_window, gint x, gint y) {
  GdkRectangle frame;
  gdk_window_get_frame_extents(gdk_window, &frame);
  return x >= frame.x && y >= frame.y && x < frame.x + frame.width && y < frame.y + frame.height;
}

bool GdkWindowIsShown(GdkWindow* gdk_window) {
  GdkWindowState state = gdk_window_get_state(gdk_window);
  return (state & (GDK_WINDOW_STATE_WITHDRAWN | GDK_WINDOW_STATE_ICONIFIED)) == 0;
}

// The GtkWindow that owns a toplevel GdkWindow, or nullptr for another
// application's window (foreign GdkWindows carry no user data).
GtkWindow* OwningGtkWindow(GdkWindow* gdk_window) {
  gpointer user_data = nullptr;
  gdk_window_get_user_data(gdk_window, &user_data);
  return user_data && GTK_IS_WINDOW(user_data) ? GTK_WINDOW(user_data) : nullptr;
}

}  // namespace

std::shared_ptr<Window> WindowManager::GetWindowAtPoint(Point point, WindowId excluded_window_id) {
  GdkScreen* screen = gdk_screen_get_default();
  if (!screen) {
    return nullptr;
  }

  GdkWindow* excluded = nullptr;
  if (excluded_window_id != 0) {
    if (auto window = Get(excluded_window_id)) {
      auto* widget = static_cast<GtkWidget*>(window->GetNativeObject());
      excluded = widget ? gtk_widget_get_window(widget) : nullptr;
    }
  }

  const gint x = static_cast<gint>(point.x);
  const gint y = static_cast<gint>(point.y);
  GdkWindow* found = nullptr;

  // X11 publishes the stacking order of every application's windows,
  // bottom-most first.
  GList* stack = gdk_screen_get_window_stack(screen);
  if (stack) {
    for (GList* l = g_list_last(stack); l != nullptr; l = l->prev) {
      GdkWindow* candidate = GDK_WINDOW(l->data);
      if (candidate == excluded || !GdkWindowIsShown(candidate) ||
          !GdkWindowContainsPoint(candidate, x, y)) {
        continue;
      }
      // Only this application's windows are returned; anything else covers
      // the point.
      found = OwningGtkWindow(candidate) ? candidate : nullptr;
      break;
    }
    g_list_free_full(stack, g_object_unref);
  } else {
    // Wayland: no global stacking order. Prefer the focused window, then
    // whichever toplevel comes first.
    GList* toplevels = gtk_window_list_toplevels();
    for (GList* l = toplevels; l != nullptr; l = l->next) {
      GtkWindow* gtk_window = GTK_WINDOW(l->data);
      GdkWindow* candidate = gtk_widget_get_window(GTK_WIDGET(gtk_window));
      if (!candidate || candidate == excluded ||
          gtk_window_get_window_type(gtk_window) != GTK_WINDOW_TOPLEVEL ||
          !gtk_widget_get_visible(GTK_WIDGET(gtk_window)) || !GdkWindowIsShown(candidate) ||
          !GdkWindowContainsPoint(candidate, x, y)) {
        continue;
      }
      if (!found || gtk_window_is_active(gtk_window)) {
        found = candidate;
      }
    }
    g_list_free(toplevels);
  }

  if (!found) {
    return nullptr;
  }
  WindowId window_id = GetOrCreateWindowId(found);
  if (window_id == IdAllocator::kInvalidId) {
    return nullptr;
  }
  return Get(window_id);
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

bool WindowManager::HasWillShowHook() const {
  return pimpl_->will_show_hook_.has_value();
}

bool WindowManager::HasWillHideHook() const {
  return pimpl_->will_hide_hook_.has_value();
}

void WindowManager::HandleWillShow(WindowId id) {
  if (pimpl_->will_show_hook_) {
    (*pimpl_->will_show_hook_)(id);
  }
}

void WindowManager::HandleWillHide(WindowId id) {
  if (pimpl_->will_hide_hook_) {
    (*pimpl_->will_hide_hook_)(id);
  }
}

bool WindowManager::CallOriginalShow(WindowId id) {
  GdkWindow* gdk_window = FindGdkWindowById(id);
  if (!gdk_window) {
    return false;
  }

  // Call the original GDK show function directly
  gdk_window_show(gdk_window);
  return true;
}

bool WindowManager::CallOriginalHide(WindowId id) {
  GdkWindow* gdk_window = FindGdkWindowById(id);
  if (!gdk_window) {
    return false;
  }

  // Call the original GDK hide function directly
  gdk_window_hide(gdk_window);
  return true;
}

void WindowManager::StartEventListening() {
  pimpl_->StartEventListening();
}

void WindowManager::StopEventListening() {
  pimpl_->StopEventListening();
}

void WindowManager::DispatchWindowEvent(const WindowEvent& event) {
  Emit(event);
}

}  // namespace nativeapi
