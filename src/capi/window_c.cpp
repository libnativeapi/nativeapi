#include "window_c.h"
#include "../window.h"
#include <cstring>
#include <memory>

using namespace nativeapi;

// Internal structure to hold the actual Window pointer
struct native_window_handle {
    std::shared_ptr<Window> window;
    explicit native_window_handle(std::shared_ptr<Window> w) : window(std::move(w)) {}
};

// Window options creation and destruction
FFI_PLUGIN_EXPORT
native_window_options_t* native_window_options_create(void) {
    auto* options = new (std::nothrow) native_window_options_t;
    if (!options) return nullptr;
    
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
    if (!options) return;
    
    if (options->title) {
        delete[] options->title;
    }
    delete options;
}

FFI_PLUGIN_EXPORT
bool native_window_options_set_title(native_window_options_t* options, const char* title) {
    if (!options) return false;
    
    // Free existing title
    if (options->title) {
        delete[] options->title;
        options->title = nullptr;
    }
    
    if (title) {
        size_t len = strlen(title) + 1;
        options->title = new (std::nothrow) char[len];
        if (!options->title) return false;
        strcpy(options->title, title);
    }
    
    return true;
}

FFI_PLUGIN_EXPORT
void native_window_options_set_size(native_window_options_t* options, double width, double height) {
    if (!options) return;
    options->size.width = width;
    options->size.height = height;
}

FFI_PLUGIN_EXPORT
void native_window_options_set_minimum_size(native_window_options_t* options, double width, double height) {
    if (!options) return;
    options->minimum_size.width = width;
    options->minimum_size.height = height;
}

FFI_PLUGIN_EXPORT
void native_window_options_set_maximum_size(native_window_options_t* options, double width, double height) {
    if (!options) return;
    options->maximum_size.width = width;
    options->maximum_size.height = height;
}

FFI_PLUGIN_EXPORT
void native_window_options_set_centered(native_window_options_t* options, bool centered) {
    if (!options) return;
    options->centered = centered;
}

// Window basic operations
FFI_PLUGIN_EXPORT
native_window_id_t native_window_get_id(native_window_t window) {
    if (!window || !window->window) return -1;
    return window->window->id;
}

FFI_PLUGIN_EXPORT
void native_window_focus(native_window_t window) {
    if (!window || !window->window) return;
    window->window->Focus();
}

FFI_PLUGIN_EXPORT
void native_window_blur(native_window_t window) {
    if (!window || !window->window) return;
    window->window->Blur();
}

FFI_PLUGIN_EXPORT
bool native_window_is_focused(native_window_t window) {
    if (!window || !window->window) return false;
    return window->window->IsFocused();
}

FFI_PLUGIN_EXPORT
void native_window_show(native_window_t window) {
    if (!window || !window->window) return;
    window->window->Show();
}

FFI_PLUGIN_EXPORT
void native_window_show_inactive(native_window_t window) {
    if (!window || !window->window) return;
    window->window->ShowInactive();
}

FFI_PLUGIN_EXPORT
void native_window_hide(native_window_t window) {
    if (!window || !window->window) return;
    window->window->Hide();
}

FFI_PLUGIN_EXPORT
bool native_window_is_visible(native_window_t window) {
    if (!window || !window->window) return false;
    return window->window->IsVisible();
}

// Window state operations
FFI_PLUGIN_EXPORT
void native_window_maximize(native_window_t window) {
    if (!window || !window->window) return;
    window->window->Maximize();
}

FFI_PLUGIN_EXPORT
void native_window_unmaximize(native_window_t window) {
    if (!window || !window->window) return;
    window->window->Unmaximize();
}

FFI_PLUGIN_EXPORT
bool native_window_is_maximized(native_window_t window) {
    if (!window || !window->window) return false;
    return window->window->IsMaximized();
}

FFI_PLUGIN_EXPORT
void native_window_minimize(native_window_t window) {
    if (!window || !window->window) return;
    window->window->Minimize();
}

FFI_PLUGIN_EXPORT
void native_window_restore(native_window_t window) {
    if (!window || !window->window) return;
    window->window->Restore();
}

FFI_PLUGIN_EXPORT
bool native_window_is_minimized(native_window_t window) {
    if (!window || !window->window) return false;
    return window->window->IsMinimized();
}

FFI_PLUGIN_EXPORT
void native_window_set_fullscreen(native_window_t window, bool is_fullscreen) {
    if (!window || !window->window) return;
    window->window->SetFullScreen(is_fullscreen);
}

FFI_PLUGIN_EXPORT
bool native_window_is_fullscreen(native_window_t window) {
    if (!window || !window->window) return false;
    return window->window->IsFullScreen();
}

// Window geometry operations
FFI_PLUGIN_EXPORT
void native_window_set_bounds(native_window_t window, native_rectangle_t bounds) {
    if (!window || !window->window) return;
    Rectangle rect = {bounds.x, bounds.y, bounds.height, bounds.width};
    window->window->SetBounds(rect);
}

FFI_PLUGIN_EXPORT
native_rectangle_t native_window_get_bounds(native_window_t window) {
    native_rectangle_t result = {0.0, 0.0, 0.0, 0.0};
    if (!window || !window->window) return result;
    
    Rectangle bounds = window->window->GetBounds();
    result.x = bounds.x;
    result.y = bounds.y;
    result.width = bounds.width;
    result.height = bounds.height;
    return result;
}

FFI_PLUGIN_EXPORT
void native_window_set_size(native_window_t window, double width, double height, bool animate) {
    if (!window || !window->window) return;
    Size size = {width, height};
    window->window->SetSize(size, animate);
}

FFI_PLUGIN_EXPORT
native_size_t native_window_get_size(native_window_t window) {
    native_size_t result = {0.0, 0.0};
    if (!window || !window->window) return result;
    
    Size size = window->window->GetSize();
    result.width = size.width;
    result.height = size.height;
    return result;
}

FFI_PLUGIN_EXPORT
void native_window_set_content_size(native_window_t window, double width, double height) {
    if (!window || !window->window) return;
    Size size = {width, height};
    window->window->SetContentSize(size);
}

FFI_PLUGIN_EXPORT
native_size_t native_window_get_content_size(native_window_t window) {
    native_size_t result = {0.0, 0.0};
    if (!window || !window->window) return result;
    
    Size size = window->window->GetContentSize();
    result.width = size.width;
    result.height = size.height;
    return result;
}

FFI_PLUGIN_EXPORT
void native_window_set_minimum_size(native_window_t window, double width, double height) {
    if (!window || !window->window) return;
    Size size = {width, height};
    window->window->SetMinimumSize(size);
}

FFI_PLUGIN_EXPORT
native_size_t native_window_get_minimum_size(native_window_t window) {
    native_size_t result = {0.0, 0.0};
    if (!window || !window->window) return result;
    
    Size size = window->window->GetMinimumSize();
    result.width = size.width;
    result.height = size.height;
    return result;
}

FFI_PLUGIN_EXPORT
void native_window_set_maximum_size(native_window_t window, double width, double height) {
    if (!window || !window->window) return;
    Size size = {width, height};
    window->window->SetMaximumSize(size);
}

FFI_PLUGIN_EXPORT
native_size_t native_window_get_maximum_size(native_window_t window) {
    native_size_t result = {0.0, 0.0};
    if (!window || !window->window) return result;
    
    Size size = window->window->GetMaximumSize();
    result.width = size.width;
    result.height = size.height;
    return result;
}

FFI_PLUGIN_EXPORT
void native_window_set_position(native_window_t window, double x, double y) {
    if (!window || !window->window) return;
    Point point = {x, y};
    window->window->SetPosition(point);
}

FFI_PLUGIN_EXPORT
native_point_t native_window_get_position(native_window_t window) {
    native_point_t result = {0.0, 0.0};
    if (!window || !window->window) return result;
    
    Point point = window->window->GetPosition();
    result.x = point.x;
    result.y = point.y;
    return result;
}

// Window properties
FFI_PLUGIN_EXPORT
void native_window_set_resizable(native_window_t window, bool resizable) {
    if (!window || !window->window) return;
    window->window->SetResizable(resizable);
}

FFI_PLUGIN_EXPORT
bool native_window_is_resizable(native_window_t window) {
    if (!window || !window->window) return false;
    return window->window->IsResizable();
}

FFI_PLUGIN_EXPORT
void native_window_set_movable(native_window_t window, bool movable) {
    if (!window || !window->window) return;
    window->window->SetMovable(movable);
}

FFI_PLUGIN_EXPORT
bool native_window_is_movable(native_window_t window) {
    if (!window || !window->window) return false;
    return window->window->IsMovable();
}

FFI_PLUGIN_EXPORT
void native_window_set_minimizable(native_window_t window, bool minimizable) {
    if (!window || !window->window) return;
    window->window->SetMinimizable(minimizable);
}

FFI_PLUGIN_EXPORT
bool native_window_is_minimizable(native_window_t window) {
    if (!window || !window->window) return false;
    return window->window->IsMinimizable();
}

FFI_PLUGIN_EXPORT
void native_window_set_maximizable(native_window_t window, bool maximizable) {
    if (!window || !window->window) return;
    window->window->SetMaximizable(maximizable);
}

FFI_PLUGIN_EXPORT
bool native_window_is_maximizable(native_window_t window) {
    if (!window || !window->window) return false;
    return window->window->IsMaximizable();
}

FFI_PLUGIN_EXPORT
void native_window_set_fullscreenable(native_window_t window, bool fullscreenable) {
    if (!window || !window->window) return;
    window->window->SetFullScreenable(fullscreenable);
}

FFI_PLUGIN_EXPORT
bool native_window_is_fullscreenable(native_window_t window) {
    if (!window || !window->window) return false;
    return window->window->IsFullScreenable();
}

FFI_PLUGIN_EXPORT
void native_window_set_closable(native_window_t window, bool closable) {
    if (!window || !window->window) return;
    window->window->SetClosable(closable);
}

FFI_PLUGIN_EXPORT
bool native_window_is_closable(native_window_t window) {
    if (!window || !window->window) return false;
    return window->window->IsClosable();
}

FFI_PLUGIN_EXPORT
void native_window_set_always_on_top(native_window_t window, bool always_on_top) {
    if (!window || !window->window) return;
    window->window->SetAlwaysOnTop(always_on_top);
}

FFI_PLUGIN_EXPORT
bool native_window_is_always_on_top(native_window_t window) {
    if (!window || !window->window) return false;
    return window->window->IsAlwaysOnTop();
}

FFI_PLUGIN_EXPORT
bool native_window_set_title(native_window_t window, const char* title) {
    if (!window || !window->window || !title) return false;
    
    try {
        window->window->SetTitle(std::string(title));
        return true;
    } catch (...) {
        return false;
    }
}

FFI_PLUGIN_EXPORT
char* native_window_get_title(native_window_t window) {
    if (!window || !window->window) return nullptr;
    
    try {
        std::string title = window->window->GetTitle();
        char* result = new (std::nothrow) char[title.length() + 1];
        if (result) {
            strcpy(result, title.c_str());
        }
        return result;
    } catch (...) {
        return nullptr;
    }
}

FFI_PLUGIN_EXPORT
void native_window_set_has_shadow(native_window_t window, bool has_shadow) {
    if (!window || !window->window) return;
    window->window->SetHasShadow(has_shadow);
}

FFI_PLUGIN_EXPORT
bool native_window_has_shadow(native_window_t window) {
    if (!window || !window->window) return false;
    return window->window->HasShadow();
}

FFI_PLUGIN_EXPORT
void native_window_set_opacity(native_window_t window, float opacity) {
    if (!window || !window->window) return;
    window->window->SetOpacity(opacity);
}

FFI_PLUGIN_EXPORT
float native_window_get_opacity(native_window_t window) {
    if (!window || !window->window) return 1.0f;
    return window->window->GetOpacity();
}

FFI_PLUGIN_EXPORT
void native_window_set_visible_on_all_workspaces(native_window_t window, bool visible) {
    if (!window || !window->window) return;
    window->window->SetVisibleOnAllWorkspaces(visible);
}

FFI_PLUGIN_EXPORT
bool native_window_is_visible_on_all_workspaces(native_window_t window) {
    if (!window || !window->window) return false;
    return window->window->IsVisibleOnAllWorkspaces();
}

FFI_PLUGIN_EXPORT
void native_window_set_ignore_mouse_events(native_window_t window, bool ignore) {
    if (!window || !window->window) return;
    window->window->SetIgnoreMouseEvents(ignore);
}

FFI_PLUGIN_EXPORT
bool native_window_is_ignore_mouse_events(native_window_t window) {
    if (!window || !window->window) return false;
    return window->window->IsIgnoreMouseEvents();
}

FFI_PLUGIN_EXPORT
void native_window_set_focusable(native_window_t window, bool focusable) {
    if (!window || !window->window) return;
    window->window->SetFocusable(focusable);
}

FFI_PLUGIN_EXPORT
bool native_window_is_focusable(native_window_t window) {
    if (!window || !window->window) return false;
    return window->window->IsFocusable();
}

// Window interactions
FFI_PLUGIN_EXPORT
void native_window_start_dragging(native_window_t window) {
    if (!window || !window->window) return;
    window->window->StartDragging();
}

FFI_PLUGIN_EXPORT
void native_window_start_resizing(native_window_t window) {
    if (!window || !window->window) return;
    window->window->StartResizing();
}

// Platform-specific functions
FFI_PLUGIN_EXPORT
void* native_window_get_native_object(native_window_t window) {
    if (!window || !window->window) return nullptr;
    return window->window->GetNativeObject();
}

// Memory management
FFI_PLUGIN_EXPORT
void native_window_free_string(char* str) {
    if (str) {
        delete[] str;
    }
}

FFI_PLUGIN_EXPORT
void native_window_list_free(native_window_list_t* list) {
    if (!list) return;
    
    if (list->windows) {
        // Note: We don't delete the individual window handles here
        // because they are managed by the window manager
        delete[] list->windows;
    }
    list->windows = nullptr;
    list->count = 0;
}