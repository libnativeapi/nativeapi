// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

#pragma once

#include <stdbool.h>

#if _WIN32
#define FFI_PLUGIN_EXPORT __declspec(dllexport)
#else
#define FFI_PLUGIN_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  double x;
  double y;
} native_point_t;

typedef struct {
  double width;
  double height;
} native_size_t;

typedef struct {
  double x;
  double y;
  double width;
  double height;
} native_rectangle_t;

#ifdef __cplusplus
}
#endif
