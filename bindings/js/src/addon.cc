// Module entry point: the generated C ABI glue plus the event loop pump.
#include <node_api.h>
#include <memory>

#include "application_quit_dispatch.h"
#include "event_loop.h"
#include "napi_support.h"

namespace nativeapi_js {

void RegisterGenerated(napi_env env, napi_value exports);
#ifdef NATIVEAPI_JS_EMBEDDED_FIXTURE
napi_value InitEventDeliveryFixture(napi_env env, napi_value exports);
#endif

namespace {

napi_value JsStartEventLoop(napi_env env, napi_callback_info info) {
  Args args(env, info);
  uint64_t window = 0;
  if (!args.ok() || !GetHandle(env, args[0], &window)) {
    return nullptr;
  }
  StartEventLoop(window);
  return Undefined(env);
}

napi_value JsPumpEventLoop(napi_env env, napi_callback_info) {
  return Value::Number(PumpEventLoop()).ToJs(env);
}

napi_value JsIsMainThread(napi_env env, napi_callback_info) {
  return Value::Bool(IsPlatformMainThread()).ToJs(env);
}

napi_value JsIsEventDeliveryActive(napi_env env, napi_callback_info info) {
  Args args(env, info);
  uint64_t delivery = 0;
  if (!args.ok() || !GetHandle(env, args[0], &delivery))
    return nullptr;
  return Value::Bool(native_event_delivery_is_active(delivery)).ToJs(env);
}

napi_value JsCompleteEventDelivery(napi_env env, napi_callback_info info) {
  Args args(env, info);
  uint64_t delivery = 0;
  bool accept = false;
  if (!args.ok() || !GetHandle(env, args[0], &delivery) || !GetBool(env, args[1], &accept))
    return nullptr;
  return Value::Bool(Callback::CompleteEvent(delivery, accept)).ToJs(env);
}

napi_value JsRequestEventLoopQuit(napi_env env, napi_callback_info info) try {
  Args args(env, info);
  int32_t exit_code = 0;
  Callback* callback = nullptr;
  if (!args.ok() || !GetNumber(env, args[0], &exit_code) ||
      !GetCallback(env, args[1], false, &callback))
    return nullptr;
  auto owner = std::shared_ptr<Callback>(callback, &Callback::ReleaseUserData);
  auto loop_owner = EventLoopOwner(env);
  if (!loop_owner)
    return Undefined(env);
  OnMainThread([&] {
    nativeapi::detail::ApplicationQuitDispatch::RequestForLoop(
        exit_code, [owner](int code) { Callback::Dispatch(owner.get(), {Value::Number(code)}); },
        loop_owner);
  });
  return Undefined(env);
} catch (...) {
  napi_throw_error(env, nullptr, "nativeapi: could not request event loop quit");
  return nullptr;
}

napi_value JsBeginEventLoopSession(napi_env env, napi_callback_info) try {
  return Value::Bool(BeginEventLoopSession(env)).ToJs(env);
} catch (...) {
  napi_throw_error(env, nullptr, "nativeapi: could not create event loop owner");
  return nullptr;
}
napi_value JsEndEventLoopSession(napi_env env, napi_callback_info) {
  EndEventLoopSession(env);
  return Undefined(env);
}

napi_value Init(napi_env env, napi_value exports) {
#ifdef NATIVEAPI_JS_EMBEDDED_FIXTURE
  // Register the fixture observer before the runtime cleanup hook; it checks
  // native state after production teardown has run.
  napi_value fixture;
  napi_create_object(env, &fixture);
  InitEventDeliveryFixture(env, fixture);
  napi_set_named_property(env, exports, "__eventDeliveryFixture", fixture);
#endif
  InitRuntime(env);
  RegisterGenerated(env, exports);
  Export(env, exports, "startEventLoop", JsStartEventLoop);
  Export(env, exports, "pumpEventLoop", JsPumpEventLoop);
  Export(env, exports, "isMainThread", JsIsMainThread);
  Export(env, exports, "isEventDeliveryActive", JsIsEventDeliveryActive);
  Export(env, exports, "completeEventDelivery", JsCompleteEventDelivery);
  Export(env, exports, "requestEventLoopQuit", JsRequestEventLoopQuit);
  Export(env, exports, "beginEventLoopSession", JsBeginEventLoopSession);
  Export(env, exports, "endEventLoopSession", JsEndEventLoopSession);
  return exports;
}

}  // namespace
}  // namespace nativeapi_js

NAPI_MODULE(nativeapi, nativeapi_js::Init)
