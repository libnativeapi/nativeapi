#pragma once

#include <stdbool.h>
#include <stdint.h>

#if _WIN32
#define FFI_PLUGIN_EXPORT __declspec(dllexport)
#else
#define FFI_PLUGIN_EXPORT
#endif

#include "geometry_c.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Window ID type
 */
typedef long native_window_id_t;

/**
 * Window options structure for creating windows
 */
typedef struct {
  char* title;                    // Window title
  native_size_t size;             // Initial window size
  native_size_t minimum_size;     // Minimum window size
  native_size_t maximum_size;     // Maximum window size
  bool centered;                  // Whether to center the window on screen
} native_window_options_t;

/**
 * Opaque window handle
 */
typedef struct native_window_handle* native_window_t;

/**
 * Window list structure
 */
typedef struct {
  native_window_t* windows;
  long count;
} native_window_list_t;

// Window creation and destruction
FFI_PLUGIN_EXPORT
native_window_options_t* native_window_options_create(void);

FFI_PLUGIN_EXPORT
void native_window_options_destroy(native_window_options_t* options);

FFI_PLUGIN_EXPORT
bool native_window_options_set_title(native_window_options_t* options, const char* title);

FFI_PLUGIN_EXPORT
void native_window_options_set_size(native_window_options_t* options, double width, double height);

FFI_PLUGIN_EXPORT
void native_window_options_set_minimum_size(native_window_options_t* options, double width, double height);

FFI_PLUGIN_EXPORT
void native_window_options_set_maximum_size(native_window_options_t* options, double width, double height);

FFI_PLUGIN_EXPORT
void native_window_options_set_centered(native_window_options_t* options, bool centered);

// Window basic operations
FFI_PLUGIN_EXPORT
native_window_id_t native_window_get_id(native_window_t window);

FFI_PLUGIN_EXPORT
void native_window_focus(native_window_t window);

FFI_PLUGIN_EXPORT
void native_window_blur(native_window_t window);

FFI_PLUGIN_EXPORT
bool native_window_is_focused(native_window_t window);

FFI_PLUGIN_EXPORT
void native_window_show(native_window_t window);

FFI_PLUGIN_EXPORT
void native_window_show_inactive(native_window_t window);

FFI_PLUGIN_EXPORT
void native_window_hide(native_window_t window);

FFI_PLUGIN_EXPORT
bool native_window_is_visible(native_window_t window);

// Window state operations
FFI_PLUGIN_EXPORT
void native_window_maximize(native_window_t window);

FFI_PLUGIN_EXPORT
void native_window_unmaximize(native_window_t window);

FFI_PLUGIN_EXPORT
bool native_window_is_maximized(native_window_t window);

FFI_PLUGIN_EXPORT
void native_window_minimize(native_window_t window);

FFI_PLUGIN_EXPORT
void native_window_restore(native_window_t window);

FFI_PLUGIN_EXPORT
bool native_window_is_minimized(native_window_t window);

FFI_PLUGIN_EXPORT
void native_window_set_fullscreen(native_window_t window, bool is_fullscreen);

FFI_PLUGIN_EXPORT
bool native_window_is_fullscreen(native_window_t window);

// Window geometry operations
FFI_PLUGIN_EXPORT
void native_window_set_bounds(native_window_t window, native_rectangle_t bounds);

FFI_PLUGIN_EXPORT
native_rectangle_t native_window_get_bounds(native_window_t window);

FFI_PLUGIN_EXPORT
void native_window_set_size(native_window_t window, double width, double height, bool animate);

FFI_PLUGIN_EXPORT
native_size_t native_window_get_size(native_window_t window);

FFI_PLUGIN_EXPORT
void native_window_set_content_size(native_window_t window, double width, double height);

FFI_PLUGIN_EXPORT
native_size_t native_window_get_content_size(native_window_t window);

FFI_PLUGIN_EXPORT
void native_window_set_minimum_size(native_window_t window, double width, double height);

FFI_PLUGIN_EXPORT
native_size_t native_window_get_minimum_size(native_window_t window);

FFI_PLUGIN_EXPORT
void native_window_set_maximum_size(native_window_t window, double width, double height);

FFI_PLUGIN_EXPORT
native_size_t native_window_get_maximum_size(native_window_t window);

FFI_PLUGIN_EXPORT
void native_window_set_position(native_window_t window, double x, double y);

FFI_PLUGIN_EXPORT
native_point_t native_window_get_position(native_window_t window);

// Window properties
FFI_PLUGIN_EXPORT
void native_window_set_resizable(native_window_t window, bool resizable);

FFI_PLUGIN_EXPORT
bool native_window_is_resizable(native_window_t window);

FFI_PLUGIN_EXPORT
void native_window_set_movable(native_window_t window, bool movable);

FFI_PLUGIN_EXPORT
bool native_window_is_movable(native_window_t window);

FFI_PLUGIN_EXPORT
void native_window_set_minimizable(native_window_t window, bool minimizable);

FFI_PLUGIN_EXPORT
bool native_window_is_minimizable(native_window_t window);

FFI_PLUGIN_EXPORT
void native_window_set_maximizable(native_window_t window, bool maximizable);

FFI_PLUGIN_EXPORT
bool native_window_is_maximizable(native_window_t window);

FFI_PLUGIN_EXPORT
void native_window_set_fullscreenable(native_window_t window, bool fullscreenable);

FFI_PLUGIN_EXPORT
bool native_window_is_fullscreenable(native_window_t window);

FFI_PLUGIN_EXPORT
void native_window_set_closable(native_window_t window, bool closable);

FFI_PLUGIN_EXPORT
bool native_window_is_closable(native_window_t window);

FFI_PLUGIN_EXPORT
void native_window_set_always_on_top(native_window_t window, bool always_on_top);

FFI_PLUGIN_EXPORT
bool native_window_is_always_on_top(native_window_t window);

FFI_PLUGIN_EXPORT
bool native_window_set_title(native_window_t window, const char* title);

FFI_PLUGIN_EXPORT
char* native_window_get_title(native_window_t window);

FFI_PLUGIN_EXPORT
void native_window_set_has_shadow(native_window_t window, bool has_shadow);

FFI_PLUGIN_EXPORT
bool native_window_has_shadow(native_window_t window);

FFI_PLUGIN_EXPORT
void native_window_set_opacity(native_window_t window, float opacity);

FFI_PLUGIN_EXPORT
float native_window_get_opacity(native_window_t window);

FFI_PLUGIN_EXPORT
void native_window_set_visible_on_all_workspaces(native_window_t window, bool visible);

FFI_PLUGIN_EXPORT
bool native_window_is_visible_on_all_workspaces(native_window_t window);

FFI_PLUGIN_EXPORT
void native_window_set_ignore_mouse_events(native_window_t window, bool ignore);

FFI_PLUGIN_EXPORT
bool native_window_is_ignore_mouse_events(native_window_t window);

FFI_PLUGIN_EXPORT
void native_window_set_focusable(native_window_t window, bool focusable);

FFI_PLUGIN_EXPORT
bool native_window_is_focusable(native_window_t window);

// Window interactions
FFI_PLUGIN_EXPORT
void native_window_start_dragging(native_window_t window);

FFI_PLUGIN_EXPORT
void native_window_start_resizing(native_window_t window);

// Platform-specific functions
FFI_PLUGIN_EXPORT
void* native_window_get_ns_window(native_window_t window);

// Memory management
FFI_PLUGIN_EXPORT
void native_window_free_string(char* str);

FFI_PLUGIN_EXPORT
void native_window_list_free(native_window_list_t* list);

#ifdef __cplusplus
}
#endif