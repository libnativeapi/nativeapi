#include "window_c.h"
#include <cstring>
#include "../window.h"
#include "string_utils_c.h"

using namespace nativeapi;

// Window options creation and destruction
FFI_PLUGIN_EXPORT
native_window_options_t* native_window_options_create(void) {
  auto* options = new (std::nothrow) native_window_options_t;
  if (!options)
    return nullptr;

  // Initialize with default values
  options->title = nullptr;
  options->size = {800.0, 600.0};
  options->minimum_size = {0.0, 0.0};
  options->maximum_size = {0.0, 0.0};
  options->centered = true;

  return options;
}

FFI_PLUGIN_EXPORT
void native_window_options_destroy(native_window_options_t* options) {
  if (!options)
    return;

  if (options->title) {
    free_c_str(options->title);
  }
  delete options;
}

FFI_PLUGIN_EXPORT
bool native_window_options_set_title(native_window_options_t* options, const char* title) {
  if (!options)
    return false;

  // Free existing title
  if (options->title) {
    free_c_str(options->title);
    options->title = nullptr;
  }

  if (title) {
    std::string title_str(title);
    options->title = to_c_str(title_str);
    // to_c_str returns nullptr for empty strings, which is acceptable for title
    // For non-empty strings, nullptr indicates allocation failure
    if (!options->title && !title_str.empty()) {
      return false;
    }
  }

  return true;
}

FFI_PLUGIN_EXPORT
void native_window_options_set_size(native_window_options_t* options, double width, double height) {
  if (!options)
    return;
  options->size.width = width;
  options->size.height = height;
}

FFI_PLUGIN_EXPORT
void native_window_options_set_minimum_size(native_window_options_t* options,
                                            double width,
                                            double height) {
  if (!options)
    return;
  options->minimum_size.width = width;
  options->minimum_size.height = height;
}

FFI_PLUGIN_EXPORT
void native_window_options_set_maximum_size(native_window_options_t* options,
                                            double width,
                                            double height) {
  if (!options)
    return;
  options->maximum_size.width = width;
  options->maximum_size.height = height;
}

FFI_PLUGIN_EXPORT
void native_window_options_set_centered(native_window_options_t* options, bool centered) {
  if (!options)
    return;
  options->centered = centered;
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
  Rectangle rect = {bounds.x, bounds.y, bounds.height, bounds.width};
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
