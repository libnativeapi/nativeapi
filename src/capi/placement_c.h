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

typedef enum {
  NATIVE_PLACEMENT_TOP = 0,
  NATIVE_PLACEMENT_TOP_START = 1,
  NATIVE_PLACEMENT_TOP_END = 2,
  NATIVE_PLACEMENT_RIGHT = 3,
  NATIVE_PLACEMENT_RIGHT_START = 4,
  NATIVE_PLACEMENT_RIGHT_END = 5,
  NATIVE_PLACEMENT_BOTTOM = 6,
  NATIVE_PLACEMENT_BOTTOM_START = 7,
  NATIVE_PLACEMENT_BOTTOM_END = 8,
  NATIVE_PLACEMENT_LEFT = 9,
  NATIVE_PLACEMENT_LEFT_START = 10,
  NATIVE_PLACEMENT_LEFT_END = 11,
} native_placement_t;

#ifdef __cplusplus
}
#endif
