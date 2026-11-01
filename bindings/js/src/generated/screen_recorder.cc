// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

#include "types.h"

namespace nativeapi_js {
namespace {

napi_value Js_native_screen_recorder_create(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  Arena arena;
  (void)arena;
  auto result = OnMainThread([&] { return native_screen_recorder_create(); });
  return Value::BigInt(result).ToJs(env);
}

napi_value Js_native_screen_recorder_free(napi_env env, napi_callback_info info) {
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
  OnMainThread([&] { return native_screen_recorder_free(self); });
  return Undefined(env);
}

napi_value Js_native_screen_recorder_is_supported(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  Arena arena;
  (void)arena;
  auto result = OnMainThread([&] { return native_screen_recorder_is_supported(); });
  return Value::Bool(result).ToJs(env);
}

napi_value Js_native_screen_recorder_start(napi_env env, napi_callback_info info) {
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
  native_capture_source_t p0 = {};
  if (!GetHandle(env, args[1], &p0)) {
    return nullptr;
  }
  const char* p1 = {};
  if (!GetString(env, args[2], arena, &p1)) {
    return nullptr;
  }
  native_screen_recording_options_t p2 = {};
  if (!FromJs(env, args[3], &p2, arena)) {
    return nullptr;
  }
  auto result = OnMainThread([&] { return native_screen_recorder_start(self, p0, p1, p2); });
  return Value::Bool(result).ToJs(env);
}

napi_value Js_native_screen_recorder_stop(napi_env env, napi_callback_info info) {
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
  auto result = OnMainThread([&] { return native_screen_recorder_stop(self); });
  return Value::Bool(result).ToJs(env);
}

napi_value Js_native_screen_recorder_cancel(napi_env env, napi_callback_info info) {
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
  auto result = OnMainThread([&] { return native_screen_recorder_cancel(self); });
  return Value::Bool(result).ToJs(env);
}

napi_value Js_native_screen_recorder_get_state(napi_env env, napi_callback_info info) {
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
  auto result = OnMainThread([&] { return native_screen_recorder_get_state(self); });
  return Value::Number(static_cast<double>(result)).ToJs(env);
}

napi_value Js_native_screen_recorder_is_active(napi_env env, napi_callback_info info) {
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
  auto result = OnMainThread([&] { return native_screen_recorder_is_active(self); });
  return Value::Bool(result).ToJs(env);
}

napi_value Js_native_screen_recorder_get_duration(napi_env env, napi_callback_info info) {
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
  auto result = OnMainThread([&] { return native_screen_recorder_get_duration(self); });
  return Value::Number(static_cast<double>(result)).ToJs(env);
}

napi_value Js_native_screen_recorder_get_file_path(napi_env env, napi_callback_info info) {
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
  auto result = OnMainThread([&] { return native_screen_recorder_get_file_path(self); });
  Value value = Value::String(result ? result : "");
  free_c_str(result);
  return value.ToJs(env);
}

napi_value Js_native_screen_recorder_add_listener(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  uint64_t self = 0;
  if (!GetHandle(env, args[0], &self)) {
    return nullptr;
  }
  Callback* callback = nullptr;
  if (!GetCallback(env, args[1], /*optional=*/false, &callback)) {
    return nullptr;
  }
  native_listener_id_t id = OnMainThread([&] { return native_screen_recorder_add_listener_async(self, +[](const native_screen_recording_event_t* event, native_event_delivery_t delivery, void* user_data) {
    if (event != nullptr) {
      Callback::DispatchEvent(user_data, {ToValue(*event)}, delivery);
    } else { native_event_delivery_complete(delivery, false); }
  }, callback, &Callback::ReleaseUserData); });
  if (id) Callback::AttachRegistration(callback, [self, id] { (void)native_screen_recorder_remove_listener(self, id); });
  return Value::Number(static_cast<double>(id)).ToJs(env);
}

napi_value Js_native_screen_recorder_remove_listener(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  uint64_t self = 0;
  if (!GetHandle(env, args[0], &self)) {
    return nullptr;
  }
  native_listener_id_t id = 0;
  if (!GetNumber(env, args[1], &id)) {
    return nullptr;
  }
  bool removed = OnMainThread([&] { return native_screen_recorder_remove_listener(self, id); });
  return Value::Bool(removed).ToJs(env);
}

}  // namespace

void RegisterScreenRecorder(napi_env env, napi_value exports) {
  Export(env, exports, "native_screen_recorder_create", Js_native_screen_recorder_create);
  Export(env, exports, "native_screen_recorder_free", Js_native_screen_recorder_free);
  Export(env, exports, "native_screen_recorder_is_supported", Js_native_screen_recorder_is_supported);
  Export(env, exports, "native_screen_recorder_start", Js_native_screen_recorder_start);
  Export(env, exports, "native_screen_recorder_stop", Js_native_screen_recorder_stop);
  Export(env, exports, "native_screen_recorder_cancel", Js_native_screen_recorder_cancel);
  Export(env, exports, "native_screen_recorder_get_state", Js_native_screen_recorder_get_state);
  Export(env, exports, "native_screen_recorder_is_active", Js_native_screen_recorder_is_active);
  Export(env, exports, "native_screen_recorder_get_duration", Js_native_screen_recorder_get_duration);
  Export(env, exports, "native_screen_recorder_get_file_path", Js_native_screen_recorder_get_file_path);
  Export(env, exports, "native_screen_recorder_add_listener", Js_native_screen_recorder_add_listener);
  Export(env, exports, "native_screen_recorder_remove_listener", Js_native_screen_recorder_remove_listener);
}

}  // namespace nativeapi_js
