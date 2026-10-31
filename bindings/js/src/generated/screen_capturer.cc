// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

#include "types.h"

namespace nativeapi_js {
namespace {

napi_value Js_native_screen_capturer_is_supported(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  Arena arena;
  (void)arena;
  auto result = OnMainThread([&] { return native_screen_capturer_is_supported(); });
  return Value::Bool(result).ToJs(env);
}

napi_value Js_native_screen_capturer_is_source_selection_supported(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  Arena arena;
  (void)arena;
  auto result = OnMainThread([&] { return native_screen_capturer_is_source_selection_supported(); });
  return Value::Bool(result).ToJs(env);
}

napi_value Js_native_screen_capturer_is_direct_capture_supported(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  Arena arena;
  (void)arena;
  native_capture_source_type_t p0 = {};
  if (!GetNumber(env, args[0], &p0)) {
    return nullptr;
  }
  auto result = OnMainThread([&] { return native_screen_capturer_is_direct_capture_supported(p0); });
  return Value::Bool(result).ToJs(env);
}

napi_value Js_native_screen_capturer_select_source(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  Arena arena;
  (void)arena;
  native_capture_source_selection_options_t p0 = {};
  if (!FromJs(env, args[0], &p0, arena)) {
    return nullptr;
  }
  Callback* p1 = nullptr;
  if (!GetCallback(env, args[1], /*optional=*/false, &p1)) {
    return nullptr;
  }
  OnMainThread([&] { return native_screen_capturer_select_source(p0, +[](bool arg0, native_capture_source_t arg1, native_event_delivery_t delivery, void* user_data) { Callback::DispatchEvent(user_data, {Value::Bool(arg0), Value::BigInt(arg1)}, delivery); }, p1, &Callback::ReleaseUserData); });
  return Undefined(env);
}

napi_value Js_native_screen_capturer_capture(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  Arena arena;
  (void)arena;
  native_capture_source_t p0 = {};
  if (!GetHandle(env, args[0], &p0)) {
    return nullptr;
  }
  native_screen_capture_options_t p1 = {};
  if (!FromJs(env, args[1], &p1, arena)) {
    return nullptr;
  }
  Callback* p2 = nullptr;
  if (!GetCallback(env, args[2], /*optional=*/false, &p2)) {
    return nullptr;
  }
  OnMainThread([&] { return native_screen_capturer_capture(p0, p1, +[](bool arg0, native_image_t arg1, native_event_delivery_t delivery, void* user_data) { Callback::DispatchEvent(user_data, {Value::Bool(arg0), Value::BigInt(arg1)}, delivery); }, p2, &Callback::ReleaseUserData); });
  return Undefined(env);
}

}  // namespace

void RegisterScreenCapturer(napi_env env, napi_value exports) {
  Export(env, exports, "native_screen_capturer_is_supported", Js_native_screen_capturer_is_supported);
  Export(env, exports, "native_screen_capturer_is_source_selection_supported", Js_native_screen_capturer_is_source_selection_supported);
  Export(env, exports, "native_screen_capturer_is_direct_capture_supported", Js_native_screen_capturer_is_direct_capture_supported);
  Export(env, exports, "native_screen_capturer_select_source", Js_native_screen_capturer_select_source);
  Export(env, exports, "native_screen_capturer_capture", Js_native_screen_capturer_capture);
}

}  // namespace nativeapi_js
