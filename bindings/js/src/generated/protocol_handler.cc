// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

#include "types.h"

namespace nativeapi_js {
namespace {

napi_value Js_native_protocol_handler_is_supported(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  Arena arena;
  (void)arena;
  auto result = OnMainThread([&] { return native_protocol_handler_is_supported(); });
  return Value::Bool(result).ToJs(env);
}

napi_value Js_native_protocol_handler_register(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  Arena arena;
  (void)arena;
  const char* p0 = {};
  if (!GetString(env, args[0], arena, &p0)) {
    return nullptr;
  }
  auto result = OnMainThread([&] { return native_protocol_handler_register(p0); });
  return Value::Bool(result).ToJs(env);
}

napi_value Js_native_protocol_handler_get_initial_url(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  Arena arena;
  (void)arena;
  auto result = OnMainThread([&] { return native_protocol_handler_get_initial_url(); });
  Value value = Value::String(result);
  free_c_str(result);
  return value.ToJs(env);
}

napi_value Js_native_protocol_handler_add_listener(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  uint64_t self = 0;
  Callback* callback = nullptr;
  if (!GetCallback(env, args[0], /*optional=*/false, &callback)) {
    return nullptr;
  }
  native_listener_id_t id = OnMainThread([&] { return native_protocol_handler_add_listener_async(+[](const native_protocol_handler_event_t* event, native_event_delivery_t delivery, void* user_data) {
    if (event != nullptr) {
      Callback::DispatchEvent(user_data, {ToValue(*event)}, delivery);
    } else { native_event_delivery_complete(delivery, false); }
  }, callback, &Callback::ReleaseUserData); });
  if (id) Callback::AttachRegistration(callback, [self, id] { (void)native_protocol_handler_remove_listener(id); });
  return Value::Number(static_cast<double>(id)).ToJs(env);
}

napi_value Js_native_protocol_handler_remove_listener(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  uint64_t self = 0;
  native_listener_id_t id = 0;
  if (!GetNumber(env, args[0], &id)) {
    return nullptr;
  }
  bool removed = OnMainThread([&] { return native_protocol_handler_remove_listener(id); });
  return Value::Bool(removed).ToJs(env);
}

}  // namespace

void RegisterProtocolHandler(napi_env env, napi_value exports) {
  Export(env, exports, "native_protocol_handler_is_supported", Js_native_protocol_handler_is_supported);
  Export(env, exports, "native_protocol_handler_register", Js_native_protocol_handler_register);
  Export(env, exports, "native_protocol_handler_get_initial_url", Js_native_protocol_handler_get_initial_url);
  Export(env, exports, "native_protocol_handler_add_listener", Js_native_protocol_handler_add_listener);
  Export(env, exports, "native_protocol_handler_remove_listener", Js_native_protocol_handler_remove_listener);
}

}  // namespace nativeapi_js
