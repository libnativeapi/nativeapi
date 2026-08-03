// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

#include "dialog_c.h"

#include <cstdio>
#include <memory>
#include <new>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "string_utils_c.h"
#include "../foundation/handle_table.h"
#include "../dialog.h"

namespace {

native_dialog_modality_t ToCDialogModality(nativeapi::DialogModality value) {
  switch (value) {
    case nativeapi::DialogModality::None:
      return NATIVE_DIALOG_MODALITY_NONE;
    case nativeapi::DialogModality::Application:
      return NATIVE_DIALOG_MODALITY_APPLICATION;
    case nativeapi::DialogModality::Window:
      return NATIVE_DIALOG_MODALITY_WINDOW;
    default:
      return NATIVE_DIALOG_MODALITY_NONE;
  }
}

nativeapi::DialogModality ToCppDialogModality(native_dialog_modality_t value) {
  switch (value) {
    case NATIVE_DIALOG_MODALITY_NONE:
      return nativeapi::DialogModality::None;
    case NATIVE_DIALOG_MODALITY_APPLICATION:
      return nativeapi::DialogModality::Application;
    case NATIVE_DIALOG_MODALITY_WINDOW:
      return nativeapi::DialogModality::Window;
    default:
      return nativeapi::DialogModality::None;
  }
}

}  // namespace

