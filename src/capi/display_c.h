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
 * Display orientation enumeration
 */
typedef enum {
  NATIVE_DISPLAY_ORIENTATION_PORTRAIT = 0,
  NATIVE_DISPLAY_ORIENTATION_LANDSCAPE = 90,
  NATIVE_DISPLAY_ORIENTATION_PORTRAIT_FLIPPED = 180,
  NATIVE_DISPLAY_ORIENTATION_LANDSCAPE_FLIPPED = 270
} native_display_orientation_t;

/**
 * Opaque display handle
 * Align with window handle design: use a raw pointer to underlying C++ type
 */
typedef void* native_display_t;

/**
 * Display list structure
 */
typedef struct {
  native_display_t* displays;
  long count;
} native_display_list_t;

// Basic identification getters
FFI_PLUGIN_EXPORT
char* native_display_get_id(native_display_t display);

FFI_PLUGIN_EXPORT
char* native_display_get_name(native_display_t display);

// Physical properties getters
FFI_PLUGIN_EXPORT
native_point_t native_display_get_position(native_display_t display);

FFI_PLUGIN_EXPORT
native_size_t native_display_get_size(native_display_t display);

FFI_PLUGIN_EXPORT
native_rectangle_t native_display_get_work_area(native_display_t display);

FFI_PLUGIN_EXPORT
double native_display_get_scale_factor(native_display_t display);

// Additional properties getters
FFI_PLUGIN_EXPORT
bool native_display_is_primary(native_display_t display);

FFI_PLUGIN_EXPORT
native_display_orientation_t native_display_get_orientation(native_display_t display);

FFI_PLUGIN_EXPORT
int native_display_get_refresh_rate(native_display_t display);

FFI_PLUGIN_EXPORT
int native_display_get_bit_depth(native_display_t display);

// Platform-specific functions
FFI_PLUGIN_EXPORT
void* native_display_get_native_object(native_display_t display);

// Memory management
FFI_PLUGIN_EXPORT
void native_display_free(native_display_t display);

FFI_PLUGIN_EXPORT
void native_display_list_free(native_display_list_t* list);

#ifdef __cplusplus
}
#endif
