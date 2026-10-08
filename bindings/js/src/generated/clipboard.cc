// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

#include "types.h"

namespace nativeapi_js {
namespace {

napi_value Js_native_clipboard_is_supported(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  Arena arena;
  (void)arena;
  auto result = OnMainThread([&] { return native_clipboard_is_supported(); });
  return Value::Bool(result).ToJs(env);
}

napi_value Js_native_clipboard_is_change_monitoring_supported(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  Arena arena;
  (void)arena;
  auto result = OnMainThread([&] { return native_clipboard_is_change_monitoring_supported(); });
  return Value::Bool(result).ToJs(env);
}

napi_value Js_native_clipboard_read(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  Arena arena;
  (void)arena;
  Callback* p0 = nullptr;
  if (!GetCallback(env, args[0], /*optional=*/false, &p0)) {
    return nullptr;
  }
  OnMainThread([&] { return native_clipboard_read(+[](bool arg0, const native_clipboard_data_t* arg1, native_event_delivery_t delivery, void* user_data) { Callback::DispatchEvent(user_data, {Value::Bool(arg0), ToValue((*arg1))}, delivery); }, p0, &Callback::ReleaseUserData); });
  return Undefined(env);
}

napi_value Js_native_clipboard_read_text(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  Arena arena;
  (void)arena;
  Callback* p0 = nullptr;
  if (!GetCallback(env, args[0], /*optional=*/false, &p0)) {
    return nullptr;
  }
  OnMainThread([&] { return native_clipboard_read_text(+[](bool arg0, const char* arg1, native_event_delivery_t delivery, void* user_data) { Callback::DispatchEvent(user_data, {Value::Bool(arg0), Value::String(arg1)}, delivery); }, p0, &Callback::ReleaseUserData); });
  return Undefined(env);
}

napi_value Js_native_clipboard_read_html(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  Arena arena;
  (void)arena;
  Callback* p0 = nullptr;
  if (!GetCallback(env, args[0], /*optional=*/false, &p0)) {
    return nullptr;
  }
  OnMainThread([&] { return native_clipboard_read_html(+[](bool arg0, const char* arg1, native_event_delivery_t delivery, void* user_data) { Callback::DispatchEvent(user_data, {Value::Bool(arg0), Value::String(arg1)}, delivery); }, p0, &Callback::ReleaseUserData); });
  return Undefined(env);
}

napi_value Js_native_clipboard_read_image(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  Arena arena;
  (void)arena;
  Callback* p0 = nullptr;
  if (!GetCallback(env, args[0], /*optional=*/false, &p0)) {
    return nullptr;
  }
  OnMainThread([&] { return native_clipboard_read_image(+[](bool arg0, native_image_t arg1, native_event_delivery_t delivery, void* user_data) { Callback::DispatchEvent(user_data, {Value::Bool(arg0), Value::BigInt(arg1)}, delivery); }, p0, &Callback::ReleaseUserData); });
  return Undefined(env);
}

napi_value Js_native_clipboard_read_file_paths(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  Arena arena;
  (void)arena;
  Callback* p0 = nullptr;
  if (!GetCallback(env, args[0], /*optional=*/false, &p0)) {
    return nullptr;
  }
  OnMainThread([&] { return native_clipboard_read_file_paths(+[](bool arg0, const native_string_list_t* arg1, native_event_delivery_t delivery, void* user_data) { Callback::DispatchEvent(user_data, {Value::Bool(arg0), CopyStringList((*arg1))}, delivery); }, p0, &Callback::ReleaseUserData); });
  return Undefined(env);
}

napi_value Js_native_clipboard_write(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  Arena arena;
  (void)arena;
  native_clipboard_data_t p0 = {};
  if (!FromJs(env, args[0], &p0, arena)) {
    return nullptr;
  }
  auto result = OnMainThread([&] { return native_clipboard_write(p0); });
  return Value::Bool(result).ToJs(env);
}

napi_value Js_native_clipboard_write_text(napi_env env, napi_callback_info info) {
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
  auto result = OnMainThread([&] { return native_clipboard_write_text(p0); });
  return Value::Bool(result).ToJs(env);
}

napi_value Js_native_clipboard_write_html(napi_env env, napi_callback_info info) {
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
  auto result = OnMainThread([&] { return native_clipboard_write_html(p0); });
  return Value::Bool(result).ToJs(env);
}

napi_value Js_native_clipboard_write_image(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  Arena arena;
  (void)arena;
  native_image_t p0 = {};
  if (!GetHandle(env, args[0], &p0)) {
    return nullptr;
  }
  auto result = OnMainThread([&] { return native_clipboard_write_image(p0); });
  return Value::Bool(result).ToJs(env);
}

napi_value Js_native_clipboard_write_file_paths(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  Arena arena;
  (void)arena;
  native_string_list_t p0 = {};
  if (!GetStringList(env, args[0], arena, &p0)) {
    return nullptr;
  }
  auto result = OnMainThread([&] { return native_clipboard_write_file_paths(p0); });
  return Value::Bool(result).ToJs(env);
}

napi_value Js_native_clipboard_clear(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  Arena arena;
  (void)arena;
  auto result = OnMainThread([&] { return native_clipboard_clear(); });
  return Value::Bool(result).ToJs(env);
}

napi_value Js_native_clipboard_is_monitoring(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  Arena arena;
  (void)arena;
  auto result = OnMainThread([&] { return native_clipboard_is_monitoring(); });
  return Value::Bool(result).ToJs(env);
}

napi_value Js_native_clipboard_add_listener(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  uint64_t self = 0;
  Callback* callback = nullptr;
  if (!GetCallback(env, args[0], /*optional=*/false, &callback)) {
    return nullptr;
  }
  native_listener_id_t id = OnMainThread([&] { return native_clipboard_add_listener_async(+[](const native_clipboard_event_t* event, native_event_delivery_t delivery, void* user_data) {
    if (event != nullptr) {
      Callback::DispatchEvent(user_data, {ToValue(*event)}, delivery);
    } else { native_event_delivery_complete(delivery, false); }
  }, callback, &Callback::ReleaseUserData); });
  if (id) Callback::AttachRegistration(callback, [self, id] { (void)native_clipboard_remove_listener(id); });
  return Value::Number(static_cast<double>(id)).ToJs(env);
}

napi_value Js_native_clipboard_remove_listener(napi_env env, napi_callback_info info) {
  Args args(env, info);
  if (!args.ok()) {
    return nullptr;
  }
  uint64_t self = 0;
  native_listener_id_t id = 0;
  if (!GetNumber(env, args[0], &id)) {
    return nullptr;
  }
  bool removed = OnMainThread([&] { return native_clipboard_remove_listener(id); });
  return Value::Bool(removed).ToJs(env);
}

}  // namespace

void RegisterClipboard(napi_env env, napi_value exports) {
  Export(env, exports, "native_clipboard_is_supported", Js_native_clipboard_is_supported);
  Export(env, exports, "native_clipboard_is_change_monitoring_supported", Js_native_clipboard_is_change_monitoring_supported);
  Export(env, exports, "native_clipboard_read", Js_native_clipboard_read);
  Export(env, exports, "native_clipboard_read_text", Js_native_clipboard_read_text);
  Export(env, exports, "native_clipboard_read_html", Js_native_clipboard_read_html);
  Export(env, exports, "native_clipboard_read_image", Js_native_clipboard_read_image);
  Export(env, exports, "native_clipboard_read_file_paths", Js_native_clipboard_read_file_paths);
  Export(env, exports, "native_clipboard_write", Js_native_clipboard_write);
  Export(env, exports, "native_clipboard_write_text", Js_native_clipboard_write_text);
  Export(env, exports, "native_clipboard_write_html", Js_native_clipboard_write_html);
  Export(env, exports, "native_clipboard_write_image", Js_native_clipboard_write_image);
  Export(env, exports, "native_clipboard_write_file_paths", Js_native_clipboard_write_file_paths);
  Export(env, exports, "native_clipboard_clear", Js_native_clipboard_clear);
  Export(env, exports, "native_clipboard_is_monitoring", Js_native_clipboard_is_monitoring);
  Export(env, exports, "native_clipboard_add_listener", Js_native_clipboard_add_listener);
  Export(env, exports, "native_clipboard_remove_listener", Js_native_clipboard_remove_listener);
}

}  // namespace nativeapi_js
