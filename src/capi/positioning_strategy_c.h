#pragma once

#include <stdbool.h>

#include "geometry_c.h"
#include "window_c.h"

#if _WIN32
#define FFI_PLUGIN_EXPORT __declspec(dllexport)
#else
#define FFI_PLUGIN_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Type of positioning strategy
 */
typedef enum {
  NATIVE_POSITIONING_ABSOLUTE,
  NATIVE_POSITIONING_CURSOR_POSITION,
  NATIVE_POSITIONING_RELATIVE
} native_positioning_strategy_type_t;

/**
 * Opaque handle for positioning strategy
 */
typedef void* native_positioning_strategy_t;

/**
 * Create a positioning strategy for absolute positioning at fixed coordinates
 * @param point Point in screen coordinates
 * @return Positioning strategy handle
 */
FFI_PLUGIN_EXPORT
native_positioning_strategy_t native_positioning_strategy_absolute(const native_point_t* point);

/**
 * Create a positioning strategy for positioning at current mouse location
 * @return Positioning strategy handle
 */
FFI_PLUGIN_EXPORT
native_positioning_strategy_t native_positioning_strategy_cursor_position();

/**
 * Create a positioning strategy for positioning relative to a rectangle
 * @param rect Rectangle in screen coordinates to position relative to
 * @param offset Offset point to apply to the position, or NULL for no offset
 * @return Positioning strategy handle
 *
 * @example
 * ```c
 * native_rectangle_t buttonRect = {100, 100, 50, 30};
 * native_point_t offset = {0, 10};
 * native_positioning_strategy_t strategy = native_positioning_strategy_relative(&buttonRect,
 * &offset); native_menu_open(menu, strategy); native_positioning_strategy_free(strategy);
 * ```
 */
FFI_PLUGIN_EXPORT
native_positioning_strategy_t native_positioning_strategy_relative(const native_rectangle_t* rect,
                                                                   const native_point_t* offset);

/**
 * Create a positioning strategy for positioning relative to a window
 * @param window Window to position relative to
 * @param offset Offset point to apply to the position, or NULL for no offset
 * @return Positioning strategy handle, or NULL if window is invalid
 *
 * This function obtains the window's bounds using native_window_get_bounds()
 * and creates a Relative positioning strategy based on those bounds.
 *
 * @example
 * ```c
 * native_window_t window = native_window_manager_create(&options);
 * native_point_t offset = {0, 10};
 * native_positioning_strategy_t strategy = native_positioning_strategy_relative_to_window(window,
 * &offset); native_menu_open(menu, strategy); native_positioning_strategy_free(strategy);
 * ```
 */
FFI_PLUGIN_EXPORT
native_positioning_strategy_t native_positioning_strategy_relative_to_window(
    native_window_t window,
    const native_point_t* offset);

/**
 * Free a positioning strategy handle
 * @param strategy The positioning strategy to free
 */
FFI_PLUGIN_EXPORT
void native_positioning_strategy_free(native_positioning_strategy_t strategy);

#ifdef __cplusplus
}
#endif
