// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "common_c.h"

#if _WIN32
#define FFI_PLUGIN_EXPORT __declspec(dllexport)
#else
#define FFI_PLUGIN_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  unsigned char r;
  unsigned char g;
  unsigned char b;
  unsigned char a;
} native_color_t;

FFI_PLUGIN_EXPORT
extern const native_color_t NATIVE_COLOR_TRANSPARENT;

FFI_PLUGIN_EXPORT
extern const native_color_t NATIVE_COLOR_BLACK;

FFI_PLUGIN_EXPORT
extern const native_color_t NATIVE_COLOR_WHITE;

FFI_PLUGIN_EXPORT
extern const native_color_t NATIVE_COLOR_RED;

FFI_PLUGIN_EXPORT
extern const native_color_t NATIVE_COLOR_GREEN;

FFI_PLUGIN_EXPORT
extern const native_color_t NATIVE_COLOR_BLUE;

FFI_PLUGIN_EXPORT
extern const native_color_t NATIVE_COLOR_YELLOW;

FFI_PLUGIN_EXPORT
extern const native_color_t NATIVE_COLOR_CYAN;

FFI_PLUGIN_EXPORT
extern const native_color_t NATIVE_COLOR_MAGENTA;

FFI_PLUGIN_EXPORT
native_color_t native_color_from_rgba(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha);

FFI_PLUGIN_EXPORT
native_color_t native_color_from_hex(const char* hex);

FFI_PLUGIN_EXPORT
unsigned int native_color_to_rgba(native_color_t color);

FFI_PLUGIN_EXPORT
unsigned int native_color_to_argb(native_color_t color);

#ifdef __cplusplus
}
#endif
