#include "window_c.h"
#include <cstring>
#include <memory>
#include <mutex>
#include <unordered_map>
#include "../window.h"
#include "string_utils_c.h"

using namespace nativeapi;

// Internal registry to manage window lifetimes for C API
// This keeps shared_ptr alive while C code holds the handle
namespace {
std::mutex g_windows_mutex;
std::unordered_map<WindowId, std::shared_ptr<Window>> g_windows;
}  // namespace

// Window creation and destruction
FFI_PLUGIN_EXPORT
native_window_t native_window_create(void) {
  try {
    // Create window with default settings
    auto window = std::make_shared<Window>();

    // Store in internal registry to keep it alive
    {
      std::lock_guard<std::mutex> lock(g_windows_mutex);
      g_windows[window->GetId()] = window;
    }

    // Return raw pointer (internal registry holds the shared_ptr)
    return static_cast<void*>(window.get());
  } catch (...) {
    return nullptr;
  }
}

FFI_PLUGIN_EXPORT
native_window_t native_window_create_from_native(void* native_window) {
  if (!native_window)
    return nullptr;

  try {
    // Wrap existing native window
    auto window = std::make_shared<Window>(native_window);

    // Store in internal registry to keep it alive
    {
      std::lock_guard<std::mutex> lock(g_windows_mutex);
      g_windows[window->GetId()] = window;
    }

    // Return raw pointer (internal registry holds the shared_ptr)
    return static_cast<void*>(window.get());
  } catch (...) {
    return nullptr;
  }
}

FFI_PLUGIN_EXPORT
void native_window_destroy(native_window_t window) {
  if (!window)
    return;

  try {
    auto* win = static_cast<nativeapi::Window*>(window);
    WindowId window_id = win->GetId();

    // Remove from internal registry (this will destroy it if no other references exist)
    {
      std::lock_guard<std::mutex> lock(g_windows_mutex);
      g_windows.erase(window_id);
    }
  } catch (...) {
    // Silently fail
  }
}

// Window basic operations
FFI_PLUGIN_EXPORT
native_window_id_t native_window_get_id(native_window_t window) {
  if (!window)
    return -1;
  auto* win = static_cast<nativeapi::Window*>(window);
  return win->GetId();
}

FFI_PLUGIN_EXPORT
void native_window_focus(native_window_t window) {
  if (!window)
    return;
  auto* win = static_cast<nativeapi::Window*>(window);
  win->Focus();
}

FFI_PLUGIN_EXPORT
void native_window_blur(native_window_t window) {
  if (!window)
    return;
  auto* win = static_cast<nativeapi::Window*>(window);
  win->Blur();
}

FFI_PLUGIN_EXPORT
bool native_window_is_focused(native_window_t window) {
  if (!window)
    return false;
  auto* win = static_cast<nativeapi::Window*>(window);
  return win->IsFocused();
}

FFI_PLUGIN_EXPORT
void native_window_show(native_window_t window) {
  if (!window)
    return;
  auto* win = static_cast<nativeapi::Window*>(window);
  win->Show();
}

FFI_PLUGIN_EXPORT
void native_window_show_inactive(native_window_t window) {
  if (!window)
    return;
  auto* win = static_cast<nativeapi::Window*>(window);
  win->ShowInactive();
}

FFI_PLUGIN_EXPORT
void native_window_hide(native_window_t window) {
  if (!window)
    return;
  auto* win = static_cast<nativeapi::Window*>(window);
  win->Hide();
}

FFI_PLUGIN_EXPORT
bool native_window_is_visible(native_window_t window) {
  if (!window)
    return false;
  auto* win = static_cast<nativeapi::Window*>(window);
  return win->IsVisible();
}

// Window state operations
FFI_PLUGIN_EXPORT
void native_window_maximize(native_window_t window) {
  if (!window)
    return;
  auto* win = static_cast<nativeapi::Window*>(window);
  win->Maximize();
}

FFI_PLUGIN_EXPORT
void native_window_unmaximize(native_window_t window) {
  if (!window)
    return;
  auto* win = static_cast<nativeapi::Window*>(window);
  win->Unmaximize();
}

FFI_PLUGIN_EXPORT
bool native_window_is_maximized(native_window_t window) {
  if (!window)
    return false;
  auto* win = static_cast<nativeapi::Window*>(window);
  return win->IsMaximized();
}

FFI_PLUGIN_EXPORT
void native_window_minimize(native_window_t window) {
  if (!window)
    return;
  auto* win = static_cast<nativeapi::Window*>(window);
  win->Minimize();
}

FFI_PLUGIN_EXPORT
void native_window_restore(native_window_t window) {
  if (!window)
    return;
  auto* win = static_cast<nativeapi::Window*>(window);
  win->Restore();
}

FFI_PLUGIN_EXPORT
bool native_window_is_minimized(native_window_t window) {
  if (!window)
    return false;
  auto* win = static_cast<nativeapi::Window*>(window);
  return win->IsMinimized();
}

FFI_PLUGIN_EXPORT
void native_window_set_fullscreen(native_window_t window, bool is_fullscreen) {
  if (!window)
    return;
  auto* win = static_cast<nativeapi::Window*>(window);
  win->SetFullScreen(is_fullscreen);
}

FFI_PLUGIN_EXPORT
bool native_window_is_fullscreen(native_window_t window) {
  if (!window)
    return false;
  auto* win = static_cast<nativeapi::Window*>(window);
  return win->IsFullScreen();
}

// Window geometry operations
FFI_PLUGIN_EXPORT
void native_window_set_bounds(native_window_t window, native_rectangle_t bounds) {
  if (!window)
    return;
  auto* win = static_cast<nativeapi::Window*>(window);
  Rectangle rect = {bounds.x, bounds.y, bounds.width, bounds.height};
  win->SetBounds(rect);
}

FFI_PLUGIN_EXPORT
native_rectangle_t native_window_get_bounds(native_window_t window) {
  native_rectangle_t result = {0.0, 0.0, 0.0, 0.0};
  if (!window)
    return result;

  auto* win = static_cast<nativeapi::Window*>(window);
  Rectangle bounds = win->GetBounds();
  result.x = bounds.x;
  result.y = bounds.y;
  result.width = bounds.width;
  result.height = bounds.height;
  return result;
}

FFI_PLUGIN_EXPORT
void native_window_set_size(native_window_t window, double width, double height, bool animate) {
  if (!window)
    return;
  auto* win = static_cast<nativeapi::Window*>(window);
  nativeapi::Size size = {width, height};
  win->SetSize(size, animate);
}

FFI_PLUGIN_EXPORT
native_size_t native_window_get_size(native_window_t window) {
  native_size_t result = {0.0, 0.0};
  if (!window)
    return result;

  auto* win = static_cast<nativeapi::Window*>(window);
  nativeapi::Size size = win->GetSize();
  result.width = size.width;
  result.height = size.height;
  return result;
}

FFI_PLUGIN_EXPORT
void native_window_set_content_size(native_window_t window, double width, double height) {
  if (!window)
    return;
  nativeapi::Size size = {width, height};
  auto* win = static_cast<nativeapi::Window*>(window);
  win->SetContentSize(size);
}

FFI_PLUGIN_EXPORT
native_size_t native_window_get_content_size(native_window_t window) {
  native_size_t result = {0.0, 0.0};
  if (!window)
    return result;

  auto* win = static_cast<nativeapi::Window*>(window);
  nativeapi::Size size = win->GetContentSize();
  result.width = size.width;
  result.height = size.height;
  return result;
}

FFI_PLUGIN_EXPORT
void native_window_set_content_bounds(native_window_t window, native_rectangle_t bounds) {
  if (!window)
    return;
  auto* win = static_cast<nativeapi::Window*>(window);
  nativeapi::Rectangle rect = {bounds.x, bounds.y, bounds.width, bounds.height};
  win->SetContentBounds(rect);
}

FFI_PLUGIN_EXPORT
native_rectangle_t native_window_get_content_bounds(native_window_t window) {
  native_rectangle_t result = {0.0, 0.0, 0.0, 0.0};
  if (!window)
    return result;

  auto* win = static_cast<nativeapi::Window*>(window);
  nativeapi::Rectangle bounds = win->GetContentBounds();
  result.x = bounds.x;
  result.y = bounds.y;
  result.width = bounds.width;
  result.height = bounds.height;
  return result;
}

FFI_PLUGIN_EXPORT
void native_window_set_minimum_size(native_window_t window, double width, double height) {
  if (!window)
    return;
  nativeapi::Size size = {width, height};
  auto* win = static_cast<nativeapi::Window*>(window);
  win->SetMinimumSize(size);
}

FFI_PLUGIN_EXPORT
native_size_t native_window_get_minimum_size(native_window_t window) {
  native_size_t result = {0.0, 0.0};
  if (!window)
    return result;

  auto* win = static_cast<nativeapi::Window*>(window);
  nativeapi::Size size = win->GetMinimumSize();
  result.width = size.width;
  result.height = size.height;
  return result;
}

FFI_PLUGIN_EXPORT
void native_window_set_maximum_size(native_window_t window, double width, double height) {
  if (!window)
    return;
  nativeapi::Size size = {width, height};
  auto* win = static_cast<nativeapi::Window*>(window);
  win->SetMaximumSize(size);
}

FFI_PLUGIN_EXPORT
native_size_t native_window_get_maximum_size(native_window_t window) {
  native_size_t result = {0.0, 0.0};
  if (!window)
    return result;

  auto* win = static_cast<nativeapi::Window*>(window);
  nativeapi::Size size = win->GetMaximumSize();
  result.width = size.width;
  result.height = size.height;
  return result;
}

FFI_PLUGIN_EXPORT
void native_window_set_position(native_window_t window, double x, double y) {
  if (!window)
    return;
  nativeapi::Point point = {x, y};
  auto* win = static_cast<nativeapi::Window*>(window);
  win->SetPosition(point);
}

FFI_PLUGIN_EXPORT
native_point_t native_window_get_position(native_window_t window) {
  native_point_t result = {0.0, 0.0};
  if (!window)
    return result;

  auto* win = static_cast<nativeapi::Window*>(window);
  nativeapi::Point point = win->GetPosition();
  result.x = point.x;
  result.y = point.y;
  return result;
}

FFI_PLUGIN_EXPORT
void native_window_center(native_window_t window) {
  if (!window)
    return;
  auto* win = static_cast<nativeapi::Window*>(window);
  win->Center();
}

// Window properties
FFI_PLUGIN_EXPORT
void native_window_set_resizable(native_window_t window, bool resizable) {
  if (!window)
    return;
  auto* win = static_cast<nativeapi::Window*>(window);
  win->SetResizable(resizable);
}

FFI_PLUGIN_EXPORT
bool native_window_is_resizable(native_window_t window) {
  if (!window)
    return false;
  auto* win = static_cast<nativeapi::Window*>(window);
  return win->IsResizable();
}

FFI_PLUGIN_EXPORT
void native_window_set_movable(native_window_t window, bool movable) {
  if (!window)
    return;
  auto* win = static_cast<nativeapi::Window*>(window);
  win->SetMovable(movable);
}

FFI_PLUGIN_EXPORT
bool native_window_is_movable(native_window_t window) {
  if (!window)
    return false;
  auto* win = static_cast<nativeapi::Window*>(window);
  return win->IsMovable();
}

FFI_PLUGIN_EXPORT
void native_window_set_minimizable(native_window_t window, bool minimizable) {
  if (!window)
    return;
  auto* win = static_cast<nativeapi::Window*>(window);
  win->SetMinimizable(minimizable);
}

FFI_PLUGIN_EXPORT
bool native_window_is_minimizable(native_window_t window) {
  if (!window)
    return false;
  auto* win = static_cast<nativeapi::Window*>(window);
  return win->IsMinimizable();
}

FFI_PLUGIN_EXPORT
void native_window_set_maximizable(native_window_t window, bool maximizable) {
  if (!window)
    return;
  auto* win = static_cast<nativeapi::Window*>(window);
  win->SetMaximizable(maximizable);
}

FFI_PLUGIN_EXPORT
bool native_window_is_maximizable(native_window_t window) {
  if (!window)
    return false;
  auto* win = static_cast<nativeapi::Window*>(window);
  return win->IsMaximizable();
}

FFI_PLUGIN_EXPORT
void native_window_set_fullscreenable(native_window_t window, bool fullscreenable) {
  if (!window)
    return;
  auto* win = static_cast<nativeapi::Window*>(window);
  win->SetFullScreenable(fullscreenable);
}

FFI_PLUGIN_EXPORT
bool native_window_is_fullscreenable(native_window_t window) {
  if (!window)
    return false;
  auto* win = static_cast<nativeapi::Window*>(window);
  return win->IsFullScreenable();
}

FFI_PLUGIN_EXPORT
void native_window_set_closable(native_window_t window, bool closable) {
  if (!window)
    return;
  auto* win = static_cast<nativeapi::Window*>(window);
  win->SetClosable(closable);
}

FFI_PLUGIN_EXPORT
bool native_window_is_closable(native_window_t window) {
  if (!window)
    return false;
  auto* win = static_cast<nativeapi::Window*>(window);
  return win->IsClosable();
}

FFI_PLUGIN_EXPORT
void native_window_set_window_control_buttons_visible(native_window_t window, bool visible) {
  if (!window)
    return;
  auto* win = static_cast<nativeapi::Window*>(window);
  win->SetWindowControlButtonsVisible(visible);
}

FFI_PLUGIN_EXPORT
bool native_window_is_window_control_buttons_visible(native_window_t window) {
  if (!window)
    return true;
  auto* win = static_cast<nativeapi::Window*>(window);
  return win->IsWindowControlButtonsVisible();
}

FFI_PLUGIN_EXPORT
void native_window_set_always_on_top(native_window_t window, bool always_on_top) {
  if (!window)
    return;
  auto* win = static_cast<nativeapi::Window*>(window);
  win->SetAlwaysOnTop(always_on_top);
}

FFI_PLUGIN_EXPORT
bool native_window_is_always_on_top(native_window_t window) {
  if (!window)
    return false;
  auto* win = static_cast<nativeapi::Window*>(window);
  return win->IsAlwaysOnTop();
}

FFI_PLUGIN_EXPORT
bool native_window_set_title(native_window_t window, const char* title) {
  if (!window || !title)
    return false;

  try {
    auto* win = static_cast<nativeapi::Window*>(window);
    win->SetTitle(std::string(title));
    return true;
  } catch (...) {
    return false;
  }
}

FFI_PLUGIN_EXPORT
char* native_window_get_title(native_window_t window) {
  if (!window)
    return nullptr;

  try {
    auto* win = static_cast<nativeapi::Window*>(window);
    std::string title = win->GetTitle();
    return to_c_str(title);
  } catch (...) {
    return nullptr;
  }
}

FFI_PLUGIN_EXPORT
void native_window_set_title_bar_style(native_window_t window, native_title_bar_style_t style) {
  if (!window)
    return;

  try {
    auto* win = static_cast<nativeapi::Window*>(window);
    TitleBarStyle cpp_style =
        (style == NATIVE_TITLE_BAR_STYLE_HIDDEN) ? TitleBarStyle::Hidden : TitleBarStyle::Normal;
    win->SetTitleBarStyle(cpp_style);
  } catch (...) {
    // Silently fail
  }
}

FFI_PLUGIN_EXPORT
native_title_bar_style_t native_window_get_title_bar_style(native_window_t window) {
  if (!window)
    return NATIVE_TITLE_BAR_STYLE_NORMAL;

  try {
    auto* win = static_cast<nativeapi::Window*>(window);
    TitleBarStyle cpp_style = win->GetTitleBarStyle();
    return (cpp_style == TitleBarStyle::Hidden) ? NATIVE_TITLE_BAR_STYLE_HIDDEN
                                                : NATIVE_TITLE_BAR_STYLE_NORMAL;
  } catch (...) {
    return NATIVE_TITLE_BAR_STYLE_NORMAL;
  }
}

FFI_PLUGIN_EXPORT
void native_window_set_has_shadow(native_window_t window, bool has_shadow) {
  if (!window)
    return;
  auto* win = static_cast<nativeapi::Window*>(window);
  win->SetHasShadow(has_shadow);
}

FFI_PLUGIN_EXPORT
bool native_window_has_shadow(native_window_t window) {
  if (!window)
    return false;
  auto* win = static_cast<nativeapi::Window*>(window);
  return win->HasShadow();
}

FFI_PLUGIN_EXPORT
void native_window_set_opacity(native_window_t window, float opacity) {
  if (!window)
    return;
  auto* win = static_cast<nativeapi::Window*>(window);
  win->SetOpacity(opacity);
}

FFI_PLUGIN_EXPORT
float native_window_get_opacity(native_window_t window) {
  if (!window)
    return 1.0f;
  auto* win = static_cast<nativeapi::Window*>(window);
  return win->GetOpacity();
}

FFI_PLUGIN_EXPORT
void native_window_set_visual_effect(native_window_t window, native_visual_effect_t effect) {
  if (!window)
    return;
  auto* win = static_cast<nativeapi::Window*>(window);
  win->SetVisualEffect(static_cast<nativeapi::VisualEffect>(effect));
}

FFI_PLUGIN_EXPORT
native_visual_effect_t native_window_get_visual_effect(native_window_t window) {
  if (!window)
    return NATIVE_VISUAL_EFFECT_NONE;
  auto* win = static_cast<nativeapi::Window*>(window);
  return static_cast<native_visual_effect_t>(win->GetVisualEffect());
}

FFI_PLUGIN_EXPORT
void native_window_set_background_color(native_window_t window, native_color_t color) {
  if (!window)
    return;
  auto* win = static_cast<nativeapi::Window*>(window);
  Color cpp_color = Color::FromRGBA(color.r, color.g, color.b, color.a);
  win->SetBackgroundColor(cpp_color);
}

FFI_PLUGIN_EXPORT
native_color_t native_window_get_background_color(native_window_t window) {
  native_color_t result = {255, 255, 255, 255};  // Default to white
  if (!window)
    return result;
  auto* win = static_cast<nativeapi::Window*>(window);
  Color cpp_color = win->GetBackgroundColor();
  result.r = cpp_color.r;
  result.g = cpp_color.g;
  result.b = cpp_color.b;
  result.a = cpp_color.a;
  return result;
}

FFI_PLUGIN_EXPORT
void native_window_set_visible_on_all_workspaces(native_window_t window, bool visible) {
  if (!window)
    return;
  auto* win = static_cast<nativeapi::Window*>(window);
  win->SetVisibleOnAllWorkspaces(visible);
}

FFI_PLUGIN_EXPORT
bool native_window_is_visible_on_all_workspaces(native_window_t window) {
  if (!window)
    return false;
  auto* win = static_cast<nativeapi::Window*>(window);
  return win->IsVisibleOnAllWorkspaces();
}

FFI_PLUGIN_EXPORT
void native_window_set_ignore_mouse_events(native_window_t window, bool ignore) {
  if (!window)
    return;
  auto* win = static_cast<nativeapi::Window*>(window);
  win->SetIgnoreMouseEvents(ignore);
}

FFI_PLUGIN_EXPORT
bool native_window_is_ignore_mouse_events(native_window_t window) {
  if (!window)
    return false;
  auto* win = static_cast<nativeapi::Window*>(window);
  return win->IsIgnoreMouseEvents();
}

FFI_PLUGIN_EXPORT
void native_window_set_focusable(native_window_t window, bool focusable) {
  if (!window)
    return;
  auto* win = static_cast<nativeapi::Window*>(window);
  win->SetFocusable(focusable);
}

FFI_PLUGIN_EXPORT
bool native_window_is_focusable(native_window_t window) {
  if (!window)
    return false;
  auto* win = static_cast<nativeapi::Window*>(window);
  return win->IsFocusable();
}

// Window interactions
FFI_PLUGIN_EXPORT
void native_window_start_dragging(native_window_t window) {
  if (!window)
    return;
  auto* win = static_cast<nativeapi::Window*>(window);
  win->StartDragging();
}

FFI_PLUGIN_EXPORT
void native_window_start_resizing(native_window_t window) {
  if (!window)
    return;
  auto* win = static_cast<nativeapi::Window*>(window);
  win->StartResizing();
}

// Platform-specific functions
FFI_PLUGIN_EXPORT
void* native_window_get_native_object(native_window_t window) {
  if (!window)
    return nullptr;
  auto* win = static_cast<nativeapi::Window*>(window);
  return win->GetNativeObject();
}

// Memory management
FFI_PLUGIN_EXPORT
void native_window_free_string(char* str) {
  free_c_str(str);
}

FFI_PLUGIN_EXPORT
void native_window_list_free(native_window_list_t* list) {
  if (!list)
    return;

  if (list->windows) {
    // Note: We don't delete the individual window handles here
    // because they are managed by the window manager
    delete[] list->windows;
  }
  list->windows = nullptr;
  list->count = 0;
}
