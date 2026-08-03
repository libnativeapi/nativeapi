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
  NATIVE_DIALOG_MODALITY_NONE = 0,
  NATIVE_DIALOG_MODALITY_APPLICATION = 1,
  NATIVE_DIALOG_MODALITY_WINDOW = 2,
} native_dialog_modality_t;

#ifdef __cplusplus
}
#endif
