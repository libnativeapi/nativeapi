#pragma once

#include "display_c.h"
#include "geometry_c.h"

#if _WIN32
#define FFI_PLUGIN_EXPORT __declspec(dllexport)
#else
#define FFI_PLUGIN_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

FFI_PLUGIN_EXPORT
native_display_list_t native_display_manager_get_all();

FFI_PLUGIN_EXPORT
native_display_t native_display_manager_get_primary();

FFI_PLUGIN_EXPORT
native_point_t native_display_manager_get_cursor_position();

#ifdef __cplusplus
}
#endif
