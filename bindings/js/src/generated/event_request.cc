// AUTO-GENERATED. DO NOT EDIT.
// Any manual changes WILL BE LOST when this file is regenerated.

#include "types.h"

namespace nativeapi_js {
namespace {

napi_value Js_native_event_decision_free(napi_env env, napi_callback_info info) {
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
  ForgetEventDecision(self);
  native_event_decision_free(self);
  return Undefined(env);
}

napi_value Js_native_event_decision_accept(napi_env env, napi_callback_info info) {
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
  auto result = native_event_decision_accept(self);
  return Value::Bool(result).ToJs(env);
}

napi_value Js_native_event_decision_cancel(napi_env env, napi_callback_info info) {
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
  auto result = native_event_decision_cancel(self);
  return Value::Bool(result).ToJs(env);
}

napi_value Js_native_event_decision_is_pending(napi_env env, napi_callback_info info) {
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
  auto result = native_event_decision_is_pending(self);
  return Value::Bool(result).ToJs(env);
}

napi_value Js_native_event_request_free(napi_env env, napi_callback_info info) {
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
  native_event_request_free(self);
  return Undefined(env);
}

napi_value Js_native_event_request_is_cancelable(napi_env env, napi_callback_info info) {
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
  auto result = native_event_request_is_cancelable(self);
  return Value::Bool(result).ToJs(env);
}

napi_value Js_native_event_request_is_cancelled(napi_env env, napi_callback_info info) {
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
  auto result = native_event_request_is_cancelled(self);
  return Value::Bool(result).ToJs(env);
}

napi_value Js_native_event_request_is_pending(napi_env env, napi_callback_info info) {
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
  auto result = native_event_request_is_pending(self);
  return Value::Bool(result).ToJs(env);
}

napi_value Js_native_event_request_cancel(napi_env env, napi_callback_info info) {
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
  auto result = native_event_request_cancel(self);
  return Value::Bool(result).ToJs(env);
}

napi_value Js_native_event_request_defer(napi_env env, napi_callback_info info) {
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
  auto result = native_event_request_defer(self);
  result = TrackEventDecision(env, result);
  return Value::BigInt(result).ToJs(env);
}

}  // namespace

void RegisterEventRequest(napi_env env, napi_value exports) {
  Export(env, exports, "native_event_decision_free", Js_native_event_decision_free);
  Export(env, exports, "native_event_decision_accept", Js_native_event_decision_accept);
  Export(env, exports, "native_event_decision_cancel", Js_native_event_decision_cancel);
  Export(env, exports, "native_event_decision_is_pending", Js_native_event_decision_is_pending);
  Export(env, exports, "native_event_request_free", Js_native_event_request_free);
  Export(env, exports, "native_event_request_is_cancelable", Js_native_event_request_is_cancelable);
  Export(env, exports, "native_event_request_is_cancelled", Js_native_event_request_is_cancelled);
  Export(env, exports, "native_event_request_is_pending", Js_native_event_request_is_pending);
  Export(env, exports, "native_event_request_cancel", Js_native_event_request_cancel);
  Export(env, exports, "native_event_request_defer", Js_native_event_request_defer);
}

}  // namespace nativeapi_js
