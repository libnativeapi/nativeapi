// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

#include "types.h"

namespace nativeapi_js {
namespace {

napi_value Js_native_capture_source_free(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  Arena arena;
  (void)arena;
  uint64_t self = 0;
  if (!GetHandle(env, args[0], &self)) {
    return nullptr;
  }
  OnMainThread([&] { return native_capture_source_free(self); });
  return Undefined(env);
}

napi_value Js_native_capture_source_from_display(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  Arena arena;
  (void)arena;
  native_display_t p0 = {};
  if (!GetHandle(env, args[0], &p0)) {
    return nullptr;
  }
  auto result = OnMainThread([&] { return native_capture_source_from_display(p0); });
  return Value::BigInt(result).ToJs(env);
}

napi_value Js_native_capture_source_from_window(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  Arena arena;
  (void)arena;
  native_window_t p0 = {};
  if (!GetHandle(env, args[0], &p0)) {
    return nullptr;
  }
  auto result = OnMainThread([&] { return native_capture_source_from_window(p0); });
  return Value::BigInt(result).ToJs(env);
}

napi_value Js_native_capture_source_get_type(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  Arena arena;
  (void)arena;
  uint64_t self = 0;
  if (!GetHandle(env, args[0], &self)) {
    return nullptr;
  }
  auto result = OnMainThread([&] { return native_capture_source_get_type(self); });
  return Value::Number(static_cast<double>(result)).ToJs(env);
}

napi_value Js_native_capture_source_get_name(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  Arena arena;
  (void)arena;
  uint64_t self = 0;
  if (!GetHandle(env, args[0], &self)) {
    return nullptr;
  }
  auto result = OnMainThread([&] { return native_capture_source_get_name(self); });
  Value value = Value::String(result ? result : "");
  free_c_str(result);
  return value.ToJs(env);
}

}  // namespace

void RegisterCaptureSource(napi_env env, napi_value exports) {
  Export(env, exports, "native_capture_source_free", Js_native_capture_source_free);
  Export(env, exports, "native_capture_source_from_display", Js_native_capture_source_from_display);
  Export(env, exports, "native_capture_source_from_window", Js_native_capture_source_from_window);
  Export(env, exports, "native_capture_source_get_type", Js_native_capture_source_get_type);
  Export(env, exports, "native_capture_source_get_name", Js_native_capture_source_get_name);
}

}  // namespace nativeapi_js
