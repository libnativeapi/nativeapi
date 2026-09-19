#include <iostream>
#include <mutex>
#include <unordered_map>
#include "../../foundation/id_allocator.h"
#include "../../window.h"
#include "../../window_manager.h"
#include "../../window_registry.h"

// Import GTK headers
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#ifdef GDK_WINDOWING_WAYLAND
#include <gdk/gdkwayland.h>
#endif

namespace nativeapi {

// Key to store/retrieve WindowId on GObjects
static const char* kWindowIdKey = "NativeAPIWindowId";

// The window manager draws a title bar and border around the client area, and the
// public geometry is the frame (window.h): positions are the frame's top-left corner
// in root coordinates and sizes include the decorations. GDK measures both, but only
// the GtkWindow API positions a toplevel: gdk_window_move() bypasses GTK's own
// bookkeeping, so the position is lost when the window is mapped and the window
// manager places the window wherever it likes instead.
struct Decorations {
  gint left = 0;
  gint top = 0;
  gint right = 0;
  gint bottom = 0;
};

// Where the frame and the content of a toplevel are, in root coordinates.
struct Layout {
  GdkRectangle frame = {};    // what the user sees as the window: title bar and content
  GdkRectangle content = {};  // the client area the application draws into
};

// A window the window manager decorates is simple: its GdkWindow is the content and
// gdk_window_get_frame_extents() is the frame. A window with client-side decorations is
// not — a header bar set as the titlebar (Flutter's runner does that on GNOME), and
// every GTK toplevel on Wayland. Its GdkWindow also holds the title bar and an
// invisible margin for the shadow, the window manager adds nothing, and both GDK
// answers describe that whole surface: 52 x 99 more than the content on GNOME 46.
// GTK's own window API already speaks in content sizes and shadowless positions
// (gtk_window_resize, gtk_window_move); this makes the measured side agree with it,
// from where GTK allocated the window's child and its title bar.
static Layout GetLayout(GtkWidget* widget, GdkWindow* gdk_window) {
  Layout layout;
  // An unmapped window has no frame yet, and GDK answers with an estimate that is not
  // one: taking it for decorations would misplace everything measured against it.
  if (!gdk_window || !gdk_window_is_viewable(gdk_window)) {
    return layout;
  }
  gint origin_x = 0;
  gint origin_y = 0;
  gdk_window_get_origin(gdk_window, &origin_x, &origin_y);
  layout.content = {origin_x, origin_y, gdk_window_get_width(gdk_window),
                    gdk_window_get_height(gdk_window)};
  gdk_window_get_frame_extents(gdk_window, &layout.frame);

  if (!widget || !GTK_IS_WINDOW(widget)) {
    return layout;
  }
  GtkWidget* child = gtk_bin_get_child(GTK_BIN(widget));
  GtkAllocation child_allocation = {};
  gint child_x = 0;
  gint child_y = 0;
  if (!child || !gtk_widget_get_mapped(child) ||
      !gtk_widget_translate_coordinates(child, widget, 0, 0, &child_x, &child_y)) {
    return layout;
  }
  gtk_widget_get_allocation(child, &child_allocation);
  const bool client_side = child_x > 0 || child_y > 0 ||
                           child_allocation.width < layout.content.width ||
                           child_allocation.height < layout.content.height;
  if (!client_side || child_allocation.width <= 1 || child_allocation.height <= 1) {
    return layout;
  }

  layout.content = {origin_x + child_x, origin_y + child_y, child_allocation.width,
                    child_allocation.height};
  // The frame is the content plus the title bar above it; the shadow is not part of it.
  gint frame_top = child_y;
  GtkWidget* titlebar = gtk_window_get_titlebar(GTK_WINDOW(widget));
  gint titlebar_x = 0;
  gint titlebar_y = 0;
  if (titlebar && gtk_widget_get_mapped(titlebar) &&
      gtk_widget_translate_coordinates(titlebar, widget, 0, 0, &titlebar_x, &titlebar_y) &&
      titlebar_y < frame_top) {
    frame_top = titlebar_y;
  }
  layout.frame = {origin_x + child_x, origin_y + frame_top, child_allocation.width,
                  child_allocation.height + (child_y - frame_top)};
#ifdef GDK_WINDOWING_WAYLAND
  // A Wayland client is not told where it is: its "origin" is the corner of its own
  // surface, shadow included. Report the frame at 0,0 as before, not at the shadow's
  // width — a position that looks real and is not.
  if (GDK_IS_WAYLAND_DISPLAY(gdk_window_get_display(gdk_window))) {
    layout.content.x -= layout.frame.x;
    layout.content.y -= layout.frame.y;
    layout.frame.x = 0;
    layout.frame.y = 0;
  }
#endif
  return layout;
}

// Zero until the window is mapped, and whatever is known on Wayland, where a client
// is not told where it is but still knows its own title bar.
static Decorations GetDecorations(GtkWidget* widget, GdkWindow* gdk_window) {
  const Layout layout = GetLayout(widget, gdk_window);
  Decorations decorations;
  decorations.left = layout.content.x - layout.frame.x;
  decorations.top = layout.content.y - layout.frame.y;
  decorations.right = layout.frame.width - layout.content.width - decorations.left;
  decorations.bottom = layout.frame.height - layout.content.height - decorations.top;
  // Before the window is mapped the answers do not agree yet; nothing is known about
  // the decorations then, rather than a negative thickness.
  if (decorations.left < 0 || decorations.top < 0 || decorations.right < 0 ||
      decorations.bottom < 0) {
    return Decorations{};
  }
  return decorations;
}

// Helper function to find header bar in widget hierarchy
static GtkWidget* FindHeaderBar(GtkWidget* widget) {
  if (!widget)
    return nullptr;

  // Check if this widget is a header bar
  if (GTK_IS_HEADER_BAR(widget))
    return widget;

  // If it's a container, search children
  if (GTK_IS_CONTAINER(widget)) {
    GList* children = gtk_container_get_children(GTK_CONTAINER(widget));
    for (GList* l = children; l != nullptr; l = l->next) {
      GtkWidget* child = GTK_WIDGET(l->data);
      GtkWidget* result = FindHeaderBar(child);
      if (result) {
        g_list_free(children);
        return result;
      }
    }
    g_list_free(children);
  }

  return nullptr;
}

// Private implementation class
class Window::Impl {
 public:
  Impl(GtkWidget* widget, GdkWindow* gdk_window)
      : widget_(widget),
        gdk_window_(gdk_window),
        title_bar_style_(TitleBarStyle::Normal),
        visual_effect_(VisualEffect::None),
        background_color_(Color::White) {}
  GtkWidget* widget_;
  GdkWindow* gdk_window_;
  TitleBarStyle title_bar_style_;
  VisualEffect visual_effect_;
  Color background_color_;
  double aspect_ratio_ = 0.0;
  // What SetContentSize() asked for while the window was not mapped yet. GDK only
  // learns the new size when the window manager confirms it, which is after anything
  // the caller does next — centring the window, say.
  Size requested_content_size_ = {0, 0};
  // Recorded only: keyboard focus is per window on Linux, see Window::SetNonActivating().
  bool non_activating_ = false;
};

Window::Window() {
  // Check if GTK is available
  GdkDisplay* display = gdk_display_get_default();
  if (!display) {
    std::cerr << "No display available for window creation" << std::endl;
    pimpl_ = std::make_unique<Impl>(nullptr, nullptr);
    return;
  }

  // Create a new GTK toplevel window
  GtkWidget* widget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  if (!widget) {
    std::cerr << "Failed to create GTK window" << std::endl;
    pimpl_ = std::make_unique<Impl>(nullptr, nullptr);
    return;
  }

  // Realize to ensure GdkWindow exists
  if (!gtk_widget_get_realized(widget)) {
    gtk_widget_realize(widget);
  }

  // Obtain GdkWindow
  GdkWindow* gdk_window = gtk_widget_get_window(widget);
  if (!gdk_window) {
    std::cerr << "Failed to get GdkWindow from GTK widget" << std::endl;
    gtk_widget_destroy(widget);
    pimpl_ = std::make_unique<Impl>(nullptr, nullptr);
    return;
  }

  // Allocate and attach a stable WindowId to the native objects
  WindowId id = IdAllocator::Allocate<Window>();
  if (id != IdAllocator::kInvalidId) {
    g_object_set_data(G_OBJECT(widget), kWindowIdKey,
                      reinterpret_cast<gpointer>(static_cast<uintptr_t>(id)));
    g_object_set_data(G_OBJECT(gdk_window), kWindowIdKey,
                      reinterpret_cast<gpointer>(static_cast<uintptr_t>(id)));
  }

  // Only create the instance, don't show the window
  pimpl_ = std::make_unique<Impl>(widget, gdk_window);
}

Window::Window(void* native_window) {
  // Wrap existing GdkWindow or GtkWidget
  GtkWidget* widget = nullptr;
  GdkWindow* gdk_window = nullptr;

  // Heuristic: if this looks like a GtkWidget*, use it; otherwise treat as GdkWindow*
  // In our codebase, native Linux window handles should be GtkWidget* (GtkWindow)
  if (native_window && GTK_IS_WIDGET(native_window)) {
    widget = static_cast<GtkWidget*>(native_window);
    if (!gtk_widget_get_realized(widget)) {
      gtk_widget_realize(widget);
    }
    gdk_window = gtk_widget_get_window(widget);
  } else if (native_window && GDK_IS_WINDOW(native_window)) {
    // A GdkWindow*: recover the GtkWidget that owns it, so GetNativeObject()
    // reports the same GtkWindow* whichever handle the wrapper was built from.
    gdk_window = static_cast<GdkWindow*>(native_window);
    gpointer user_data = nullptr;
    gdk_window_get_user_data(gdk_window, &user_data);
    if (user_data && GTK_IS_WIDGET(user_data)) {
      widget = static_cast<GtkWidget*>(user_data);
    }
  }

  // Like the other platforms, a wrapped window gets an ID on first sight, stored
  // on the native objects so every later wrapper and WindowManager agree on it.
  gpointer existing_id = nullptr;
  if (gdk_window) {
    existing_id = g_object_get_data(G_OBJECT(gdk_window), kWindowIdKey);
  }
  if (!existing_id && widget) {
    existing_id = g_object_get_data(G_OBJECT(widget), kWindowIdKey);
  }
  if (existing_id) {
    if (gdk_window) {
      g_object_set_data(G_OBJECT(gdk_window), kWindowIdKey, existing_id);
    }
  } else if (gdk_window || widget) {
    WindowId id = IdAllocator::Allocate<Window>();
    if (id != IdAllocator::kInvalidId) {
      gpointer data = reinterpret_cast<gpointer>(static_cast<uintptr_t>(id));
      if (gdk_window) {
        g_object_set_data(G_OBJECT(gdk_window), kWindowIdKey, data);
      }
      if (widget) {
        g_object_set_data(G_OBJECT(widget), kWindowIdKey, data);
      }
    }
  }

  pimpl_ = std::make_unique<Impl>(widget, gdk_window);
}

Window::~Window() {}

WindowId Window::GetId() const {
  // Prefer reading ID stored on the native objects
  if (pimpl_->gdk_window_) {
    gpointer data = g_object_get_data(G_OBJECT(pimpl_->gdk_window_), kWindowIdKey);
    if (data) {
      return static_cast<WindowId>(reinterpret_cast<uintptr_t>(data));
    }
  }
  if (pimpl_->widget_) {
    gpointer data = g_object_get_data(G_OBJECT(pimpl_->widget_), kWindowIdKey);
    if (data) {
      return static_cast<WindowId>(reinterpret_cast<uintptr_t>(data));
    }
  }
  return IdAllocator::kInvalidId;
}

void Window::Focus() {
  if (pimpl_->widget_) {
    gtk_window_present(GTK_WINDOW(pimpl_->widget_));
  } else if (pimpl_->gdk_window_) {
    gdk_window_focus(pimpl_->gdk_window_, GDK_CURRENT_TIME);
  }
}

void Window::Blur() {
  if (pimpl_->gdk_window_) {
    gdk_window_lower(pimpl_->gdk_window_);
  }
}

bool Window::IsFocused() const {
  // Asking the seat's keyboard for the window at its position is not an option: that
  // call is about pointer position and GDK rejects keyboard devices outright, so it
  // only logs an assertion failure and never finds a window. The toplevel's own
  // state works on both X11 and Wayland.
  if (pimpl_->widget_ && GTK_IS_WINDOW(pimpl_->widget_)) {
    return gtk_window_is_active(GTK_WINDOW(pimpl_->widget_));
  }
  if (!pimpl_->gdk_window_)
    return false;
  return gdk_window_get_state(pimpl_->gdk_window_) & GDK_WINDOW_STATE_FOCUSED;
}

void Window::Show() {
  if (pimpl_->widget_) {
    gtk_widget_show(pimpl_->widget_);
  } else if (pimpl_->gdk_window_) {
    gdk_window_show(pimpl_->gdk_window_);
  }
}

void Window::ShowInactive() {
  if (pimpl_->widget_) {
    gtk_widget_show(pimpl_->widget_);
  } else if (pimpl_->gdk_window_) {
    gdk_window_show_unraised(pimpl_->gdk_window_);
  }
}

void Window::Hide() {
  if (pimpl_->widget_) {
    gtk_widget_hide(pimpl_->widget_);
  } else if (pimpl_->gdk_window_) {
    gdk_window_hide(pimpl_->gdk_window_);
  }
}

bool Window::IsVisible() const {
  if (pimpl_->widget_) {
    return gtk_widget_get_visible(pimpl_->widget_);
  }
  if (pimpl_->gdk_window_) {
    return gdk_window_is_visible(pimpl_->gdk_window_);
  }
  return false;
}

void Window::Maximize() {
  if (pimpl_->gdk_window_) {
    gdk_window_maximize(pimpl_->gdk_window_);
  }
}

void Window::Unmaximize() {
  if (pimpl_->gdk_window_) {
    gdk_window_unmaximize(pimpl_->gdk_window_);
  }
}

bool Window::IsMaximized() const {
  if (!pimpl_->gdk_window_)
    return false;
  GdkWindowState state = gdk_window_get_state(pimpl_->gdk_window_);
  return state & GDK_WINDOW_STATE_MAXIMIZED;
}

void Window::Minimize() {
  if (pimpl_->gdk_window_) {
    gdk_window_iconify(pimpl_->gdk_window_);
  }
}

void Window::Restore() {
  if (pimpl_->gdk_window_) {
    gdk_window_deiconify(pimpl_->gdk_window_);
  }
}

bool Window::IsMinimized() const {
  if (!pimpl_->gdk_window_)
    return false;
  GdkWindowState state = gdk_window_get_state(pimpl_->gdk_window_);
  return state & GDK_WINDOW_STATE_ICONIFIED;
}

void Window::SetFullScreen(bool is_full_screen) {
  if (!pimpl_->gdk_window_)
    return;
  if (is_full_screen) {
    gdk_window_fullscreen(pimpl_->gdk_window_);
  } else {
    gdk_window_unfullscreen(pimpl_->gdk_window_);
  }
}

bool Window::IsFullScreen() const {
  if (!pimpl_->gdk_window_)
    return false;
  GdkWindowState state = gdk_window_get_state(pimpl_->gdk_window_);
  return state & GDK_WINDOW_STATE_FULLSCREEN;
}

void Window::SetBounds(Rectangle bounds) {
  const Decorations decorations = GetDecorations(pimpl_->widget_, pimpl_->gdk_window_);
  SetContentBounds({bounds.x + decorations.left, bounds.y + decorations.top,
                    bounds.width - decorations.left - decorations.right,
                    bounds.height - decorations.top - decorations.bottom});
}

Rectangle Window::GetBounds() const {
  const Point position = GetPosition();
  const Size size = GetSize();
  return {position.x, position.y, size.width, size.height};
}

void Window::SetSize(Size size, bool animate) {
  const Decorations decorations = GetDecorations(pimpl_->widget_, pimpl_->gdk_window_);
  SetContentSize({size.width - decorations.left - decorations.right,
                  size.height - decorations.top - decorations.bottom});
}

Size Window::GetSize() const {
  const Decorations decorations = GetDecorations(pimpl_->widget_, pimpl_->gdk_window_);
  const Size content = GetContentSize();
  return {content.width + decorations.left + decorations.right,
          content.height + decorations.top + decorations.bottom};
}

void Window::SetContentSize(Size size) {
  if (pimpl_->widget_ && !gtk_widget_get_mapped(pimpl_->widget_)) {
    pimpl_->requested_content_size_ = size;
  }
  if (pimpl_->widget_ && GTK_IS_WINDOW(pimpl_->widget_)) {
    GtkWindow* gtk_window = GTK_WINDOW(pimpl_->widget_);
    if (!gtk_widget_get_mapped(pimpl_->widget_)) {
      // Windows are realized as soon as they are created, and GTK then maps them at
      // whatever size the GdkWindow already has: a resize requested in between is
      // forgotten. Set all three, so the size holds whenever it is asked for.
      gtk_window_set_default_size(gtk_window, (gint)size.width, (gint)size.height);
      if (pimpl_->gdk_window_) {
        gdk_window_resize(pimpl_->gdk_window_, (gint)size.width, (gint)size.height);
      }
    }
    gtk_window_resize(gtk_window, (gint)size.width, (gint)size.height);
  } else if (pimpl_->gdk_window_) {
    gdk_window_resize(pimpl_->gdk_window_, (gint)size.width, (gint)size.height);
  }
}

Size Window::GetContentSize() const {
  if (pimpl_->widget_ && !gtk_widget_get_mapped(pimpl_->widget_) &&
      pimpl_->requested_content_size_.width > 0) {
    return pimpl_->requested_content_size_;
  }
  if (!pimpl_->gdk_window_) {
    return {0, 0};
  }
  if (!gdk_window_is_viewable(pimpl_->gdk_window_)) {
    return {static_cast<double>(gdk_window_get_width(pimpl_->gdk_window_)),
            static_cast<double>(gdk_window_get_height(pimpl_->gdk_window_))};
  }
  // Not the GdkWindow's size: with client-side decorations that includes the title bar
  // and the shadow, and would not be what SetContentSize() was given.
  const Layout layout = GetLayout(pimpl_->widget_, pimpl_->gdk_window_);
  return {static_cast<double>(layout.content.width),
          static_cast<double>(layout.content.height)};
}

void Window::SetContentBounds(Rectangle bounds) {
  const Decorations decorations = GetDecorations(pimpl_->widget_, pimpl_->gdk_window_);
  // gtk_window_move() takes the frame's corner, so the content lands where asked.
  SetPosition({bounds.x - decorations.left, bounds.y - decorations.top});
  SetContentSize({bounds.width, bounds.height});
}

Rectangle Window::GetContentBounds() const {
  if (!pimpl_->gdk_window_) {
    return {0, 0, 0, 0};
  }
  if (!gdk_window_is_viewable(pimpl_->gdk_window_)) {
    gint origin_x = 0;
    gint origin_y = 0;
    gdk_window_get_origin(pimpl_->gdk_window_, &origin_x, &origin_y);
    const Size size = GetContentSize();
    return {static_cast<double>(origin_x), static_cast<double>(origin_y), size.width,
            size.height};
  }
  const Layout layout = GetLayout(pimpl_->widget_, pimpl_->gdk_window_);
  return {static_cast<double>(layout.content.x), static_cast<double>(layout.content.y),
          static_cast<double>(layout.content.width),
          static_cast<double>(layout.content.height)};
}

void Window::SetMinimumSize(Size size) {
  // GTK minimum size constraints would need to be set on the widget level
  // For now, we'll provide a basic implementation that doesn't enforce
  // constraints
}

Size Window::GetMinimumSize() const {
  return Size{0, 0};
}

void Window::SetMaximumSize(Size size) {
  // GTK maximum size constraints would need to be set on the widget level
  // For now, we'll provide a basic implementation that doesn't enforce
  // constraints
}

void Window::SetAspectRatio(double aspect_ratio) {
  pimpl_->aspect_ratio_ = aspect_ratio > 0.0 ? aspect_ratio : 0.0;

  GdkGeometry geometry = {};
  GdkWindowHints hints = static_cast<GdkWindowHints>(0);
  if (pimpl_->aspect_ratio_ > 0.0) {
    geometry.min_aspect = pimpl_->aspect_ratio_;
    geometry.max_aspect = pimpl_->aspect_ratio_;
    hints = GDK_HINT_ASPECT;
  }

  // Prefer the GTK-level hints when we own a GtkWindow: GTK merges them with the
  // hints it computes itself instead of overwriting them on the next allocation.
  if (pimpl_->widget_ && GTK_IS_WINDOW(pimpl_->widget_)) {
    gtk_window_set_geometry_hints(GTK_WINDOW(pimpl_->widget_), nullptr, &geometry, hints);
  } else if (pimpl_->gdk_window_) {
    gdk_window_set_geometry_hints(pimpl_->gdk_window_, &geometry, hints);
  }
}

double Window::GetAspectRatio() const {
  return pimpl_->aspect_ratio_;
}

Size Window::GetMaximumSize() const {
  return Size{-1, -1};  // -1 indicates no maximum
}

void Window::SetResizable(bool is_resizable) {
  // This would typically be set at window creation time in GTK
  // For now, provide stub implementation
}

bool Window::IsResizable() const {
  return true;  // Default assumption
}

void Window::SetMovable(bool is_movable) {
  // Window movability is typically a window manager property
  // Provide stub implementation
}

bool Window::IsMovable() const {
  return true;  // Default assumption
}

void Window::SetMinimizable(bool is_minimizable) {
  // This would typically be set via window hints
  // Provide stub implementation
}

bool Window::IsMinimizable() const {
  return true;  // Default assumption
}

void Window::SetMaximizable(bool is_maximizable) {
  // This would typically be set via window hints
  // Provide stub implementation
}

bool Window::IsMaximizable() const {
  return true;  // Default assumption
}

void Window::SetFullScreenable(bool is_full_screenable) {
  // Provide stub implementation
}

bool Window::IsFullScreenable() const {
  return true;  // Default assumption
}

void Window::SetClosable(bool is_closable) {
  // This would typically be set via window hints
  // Provide stub implementation
}

bool Window::IsClosable() const {
  return true;  // Default assumption
}

void Window::SetWindowControlButtonsVisible(bool is_visible) {
  // TODO: Implement for Linux
  // This would involve manipulating GTK window decorations
}

bool Window::IsWindowControlButtonsVisible() const {
  // TODO: Implement for Linux
  return true;  // Default to visible
}

void Window::SetAlwaysOnTop(bool is_always_on_top) {
  if (pimpl_->gdk_window_) {
    gdk_window_set_keep_above(pimpl_->gdk_window_, is_always_on_top);
  }
}

bool Window::IsAlwaysOnTop() const {
  if (!pimpl_->gdk_window_)
    return false;
  GdkWindowState state = gdk_window_get_state(pimpl_->gdk_window_);
  return state & GDK_WINDOW_STATE_ABOVE;
}

void Window::SetAlwaysOnBottom(bool is_always_on_bottom) {
  // GDK clears _NET_WM_STATE_ABOVE when setting BELOW and vice versa, so the two
  // settings are naturally exclusive here.
  if (pimpl_->gdk_window_) {
    gdk_window_set_keep_below(pimpl_->gdk_window_, is_always_on_bottom);
  }
}

bool Window::IsAlwaysOnBottom() const {
  if (!pimpl_->gdk_window_)
    return false;
  GdkWindowState state = gdk_window_get_state(pimpl_->gdk_window_);
  return state & GDK_WINDOW_STATE_BELOW;
}

void Window::SetNonActivating(bool is_non_activating) {
  // Keyboard focus is per window on Linux, so a non-activating window has no
  // observable difference here. Record the flag so IsNonActivating() round-trips.
  pimpl_->non_activating_ = is_non_activating;
}

bool Window::IsNonActivating() const {
  return pimpl_->non_activating_;
}

void Window::SetPosition(Point point) {
  if (pimpl_->widget_ && GTK_IS_WINDOW(pimpl_->widget_)) {
    // gtk_window_move() positions the frame, and remembers the position for a window
    // that is not mapped yet — which gdk_window_move() does not.
    gtk_window_move(GTK_WINDOW(pimpl_->widget_), (gint)point.x, (gint)point.y);
  } else if (pimpl_->gdk_window_) {
    gdk_window_move(pimpl_->gdk_window_, (gint)point.x, (gint)point.y);
  }
}

Point Window::GetPosition() const {
  if (!pimpl_->gdk_window_) {
    return {0, 0};
  }
  if (!gdk_window_is_viewable(pimpl_->gdk_window_)) {
    GdkRectangle frame = {};
    gdk_window_get_frame_extents(pimpl_->gdk_window_, &frame);
    return {static_cast<double>(frame.x), static_cast<double>(frame.y)};
  }
  const Layout layout = GetLayout(pimpl_->widget_, pimpl_->gdk_window_);
  return {static_cast<double>(layout.frame.x), static_cast<double>(layout.frame.y)};
}

void Window::Center() {
  if (!pimpl_->gdk_window_)
    return;

  // The size to centre, decorations included: they are part of the window.
  const Size size = GetSize();
  const gint window_width = (gint)size.width;
  const gint window_height = (gint)size.height;

  // Get the screen size
  GdkDisplay* display = gdk_window_get_display(pimpl_->gdk_window_);
  GdkMonitor* monitor = gdk_display_get_primary_monitor(display);
  if (!monitor) {
    // Fallback to first monitor if no primary monitor is found
    monitor = gdk_display_get_monitor(display, 0);
  }

  if (monitor) {
    GdkRectangle geometry;
    gdk_monitor_get_geometry(monitor, &geometry);

    // Calculate center position
    gint center_x = geometry.x + (geometry.width - window_width) / 2;
    gint center_y = geometry.y + (geometry.height - window_height) / 2;

    // Move the window to center
    SetPosition({static_cast<double>(center_x), static_cast<double>(center_y)});
  }
}

void Window::SetTitle(std::string title) {
  // Prefer setting title via GtkWindow if available
  if (pimpl_->widget_ && GTK_IS_WINDOW(pimpl_->widget_)) {
    gtk_window_set_title(GTK_WINDOW(pimpl_->widget_), title.c_str());
    return;
  }

  // If only GdkWindow is available, try to get associated GtkWindow
  if (pimpl_->gdk_window_) {
    gpointer user_data = nullptr;
    gdk_window_get_user_data(pimpl_->gdk_window_, &user_data);
    if (user_data && GTK_IS_WINDOW(user_data)) {
      gtk_window_set_title(GTK_WINDOW(user_data), title.c_str());
      return;
    }

    // Fallback: set title via GDK for toplevel windows
    gdk_window_set_title(pimpl_->gdk_window_, title.c_str());
  }
}

std::string Window::GetTitle() const {
  // Prefer reading title via GtkWindow if available
  if (pimpl_->widget_ && GTK_IS_WINDOW(pimpl_->widget_)) {
    const gchar* t = gtk_window_get_title(GTK_WINDOW(pimpl_->widget_));
    return t ? std::string(t) : std::string();
  }

  // If only GdkWindow is available, try to get associated GtkWindow
  if (pimpl_->gdk_window_) {
    gpointer user_data = nullptr;
    gdk_window_get_user_data(pimpl_->gdk_window_, &user_data);
    if (user_data && GTK_IS_WINDOW(user_data)) {
      const gchar* t = gtk_window_get_title(GTK_WINDOW(user_data));
      return t ? std::string(t) : std::string();
    }
  }

  // No reliable way to get title directly from GdkWindow
  return std::string();
}

void Window::SetTitleBarStyle(TitleBarStyle style) {
  pimpl_->title_bar_style_ = style;

  if (!pimpl_->widget_ || !GTK_IS_WINDOW(pimpl_->widget_))
    return;

  GtkWindow* gtk_window = GTK_WINDOW(pimpl_->widget_);
  bool show_decorations = (style == TitleBarStyle::Normal);

  // Try to find and toggle header bar visibility
  GtkWidget* header_bar = FindHeaderBar(pimpl_->widget_);
  if (header_bar) {
    gtk_widget_set_visible(header_bar, show_decorations);
  } else {
    // If no header bar found, toggle window decorations
    const gchar* title = gtk_window_get_title(gtk_window);
    if (title != nullptr) {
      gtk_window_set_decorated(gtk_window, show_decorations);
    }
  }

  // When restoring to normal, ensure decorations are shown
  if (show_decorations) {
    gtk_window_set_decorated(gtk_window, TRUE);
  }
}

TitleBarStyle Window::GetTitleBarStyle() const {
  return pimpl_->title_bar_style_;
}

void Window::SetHasShadow(bool has_shadow) {
  // Window shadows are typically managed by the window manager
  // Provide stub implementation
}

bool Window::HasShadow() const {
  return true;  // Default assumption
}

void Window::SetOpacity(float opacity) {
  if (pimpl_->gdk_window_) {
    gdk_window_set_opacity(pimpl_->gdk_window_, opacity);
  }
}

float Window::GetOpacity() const {
  // GDK doesn't provide a direct way to get opacity
  return 1.0f;  // Default assumption
}

void Window::SetVisualEffect(VisualEffect effect) {
  pimpl_->visual_effect_ = effect;
  // TODO: Implement background blur for Linux (GTK/GDK)
  // This typically requires compositor support or specific GTK CSS
}

VisualEffect Window::GetVisualEffect() const {
  return pimpl_->visual_effect_;
}

void Window::SetBackgroundColor(const Color& color) {
  if (!pimpl_->widget_)
    return;

  // Store the color
  pimpl_->background_color_ = color;

  // Create CSS provider for background color
  GtkCssProvider* provider = gtk_css_provider_new();
  
  // Format CSS string with RGBA color
  gchar* css = g_strdup_printf(
    "window { background-color: rgba(%d, %d, %d, %.2f); }",
    color.r, color.g, color.b, color.a / 255.0);
  
  gtk_css_provider_load_from_data(provider, css, -1, nullptr);
  g_free(css);
  
  // Apply CSS to the widget
  GtkStyleContext* context = gtk_widget_get_style_context(pimpl_->widget_);
  gtk_style_context_add_provider(context,
                                 GTK_STYLE_PROVIDER(provider),
                                 GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  
  g_object_unref(provider);
}

Color Window::GetBackgroundColor() const {
  if (!pimpl_->widget_)
    return Color::White;
  
  // Return the stored background color
  // Since we set it via CSS, we track it ourselves to avoid using deprecated APIs
  return pimpl_->background_color_;
}

void Window::SetVisibleOnAllWorkspaces(bool is_visible_on_all_workspaces) {
  if (pimpl_->gdk_window_) {
    gdk_window_stick(pimpl_->gdk_window_);
  }
}

bool Window::IsVisibleOnAllWorkspaces() const {
  if (!pimpl_->gdk_window_)
    return false;
  GdkWindowState state = gdk_window_get_state(pimpl_->gdk_window_);
  return state & GDK_WINDOW_STATE_STICKY;
}

void Window::SetVisibleInTaskbar(bool is_visible_in_taskbar) {
  if (pimpl_->widget_ && GTK_IS_WINDOW(pimpl_->widget_)) {
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(pimpl_->widget_), !is_visible_in_taskbar);
  }
}

bool Window::IsVisibleInTaskbar() const {
  if (!pimpl_->widget_ || !GTK_IS_WINDOW(pimpl_->widget_))
    return true;
  return !gtk_window_get_skip_taskbar_hint(GTK_WINDOW(pimpl_->widget_));
}

void Window::SetIgnoreMouseEvents(bool is_ignore_mouse_events) {
  // This would involve setting input shapes or event masks
  // Provide stub implementation
}

bool Window::IsIgnoreMouseEvents() const {
  return false;  // Default assumption
}

void Window::SetFocusable(bool is_focusable) {
  // This would typically be set via window hints
  // Provide stub implementation
}

bool Window::IsFocusable() const {
  return true;  // Default assumption
}

void Window::StartDragging() {
  if (!pimpl_->gdk_window_) {
    return;
  }

  GdkDisplay* display = gdk_window_get_display(pimpl_->gdk_window_);
  GdkSeat* seat = display ? gdk_display_get_default_seat(display) : nullptr;
  GdkDevice* pointer = seat ? gdk_seat_get_pointer(seat) : nullptr;
  if (!pointer) {
    return;
  }

  gint root_x = 0, root_y = 0;
  gdk_device_get_position(pointer, nullptr, &root_x, &root_y);
  // The window manager moves the window from here on, so this works on Wayland too,
  // where the position above is meaningless and ignored — what matters is the
  // timestamp of the mouse-down we are called from, which is what the compositor
  // matches against the press it delivered.
  gdk_window_begin_move_drag_for_device(pimpl_->gdk_window_, pointer, GDK_BUTTON_PRIMARY,
                                        root_x, root_y, gtk_get_current_event_time());
}

void Window::StartResizing(ResizeEdge edge) {
  if (!pimpl_->gdk_window_) {
    return;
  }

  GdkWindowEdge gdk_edge;
  switch (edge) {
    case ResizeEdge::Top:
      gdk_edge = GDK_WINDOW_EDGE_NORTH;
      break;
    case ResizeEdge::Left:
      gdk_edge = GDK_WINDOW_EDGE_WEST;
      break;
    case ResizeEdge::Right:
      gdk_edge = GDK_WINDOW_EDGE_EAST;
      break;
    case ResizeEdge::Bottom:
      gdk_edge = GDK_WINDOW_EDGE_SOUTH;
      break;
    case ResizeEdge::TopLeft:
      gdk_edge = GDK_WINDOW_EDGE_NORTH_WEST;
      break;
    case ResizeEdge::TopRight:
      gdk_edge = GDK_WINDOW_EDGE_NORTH_EAST;
      break;
    case ResizeEdge::BottomLeft:
      gdk_edge = GDK_WINDOW_EDGE_SOUTH_WEST;
      break;
    case ResizeEdge::BottomRight:
    default:
      gdk_edge = GDK_WINDOW_EDGE_SOUTH_EAST;
      break;
  }

  GdkDisplay* display = gdk_window_get_display(pimpl_->gdk_window_);
  GdkSeat* seat = display ? gdk_display_get_default_seat(display) : nullptr;
  GdkDevice* pointer = seat ? gdk_seat_get_pointer(seat) : nullptr;
  if (!pointer) {
    return;
  }

  gint root_x = 0, root_y = 0;
  gdk_device_get_position(pointer, nullptr, &root_x, &root_y);
  // gtk_get_current_event_time() yields the timestamp of the mouse-down we are
  // called from, which window managers require to accept the drag request.
  gdk_window_begin_resize_drag_for_device(pimpl_->gdk_window_, gdk_edge, pointer,
                                          GDK_BUTTON_PRIMARY, root_x, root_y,
                                          gtk_get_current_event_time());
}

void* Window::GetNativeObjectInternal() const {
  // Return the GtkWidget* (GtkWindow) as the native handle on Linux
  return pimpl_ ? static_cast<void*>(pimpl_->widget_ ? pimpl_->widget_ : nullptr) : nullptr;
}

}  // namespace nativeapi

namespace nativeapi {
bool Window::SetTitleBarColors(const Color&, const Color&) { return false; }
bool Window::ResetTitleBarColors() { return false; }
}
