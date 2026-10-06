// Links the actual JS addon, sharing its core, handle table and callback runtime.
// All events are synthetic; no native windows or input.
#include <node_api.h>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include "application.h"
#include "capi/event_delivery.h"
#include "display_manager.h"
#include "foundation/event_request_dispatch.h"
#include "foundation/handle_table.h"
#include "napi_support.h"
#include "shortcut_manager.h"

#if defined(__APPLE__)
#import <Cocoa/Cocoa.h>
@interface JsQuitTestHost : NSObject <NSApplicationDelegate>
@end
@implementation JsQuitTestHost
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender {
  return NSTerminateCancel;
}
@end
static JsQuitTestHost* quit_host;
#endif

namespace {
std::mutex queue_mutex;
std::vector<std::function<void()>> queue;
struct FixtureState {
  std::shared_ptr<nativeapi::capi::EventDeliveryRegistration> registration;
  std::shared_ptr<nativeapi::EventRequest> request;
  std::atomic<int> outcome{-1};
  bool check_cleanup = false, runtime_initialized = false;
  int expected_outcome = 0;
  uint64_t baseline = 0;
  napi_env env;
};
std::mutex states_mutex;
std::unordered_map<napi_env, std::shared_ptr<FixtureState>> states;
std::once_flag dispatcher_initialized;
std::shared_ptr<FixtureState> StateFor(napi_env env) {
  std::lock_guard<std::mutex> lock(states_mutex);
  return states.at(env);
}

napi_value Undefined(napi_env env) {
  napi_value value;
  napi_get_undefined(env, &value);
  return value;
}
napi_value Schedule(napi_env env, napi_callback_info) {
  std::call_once(dispatcher_initialized, [] {
    nativeapi::SetMainThreadDispatcher(
        [](std::function<void()> fn) {
          std::lock_guard<std::mutex> lock(queue_mutex);
          queue.push_back(std::move(fn));
          return true;
        },
        [] { return true; });
  });
  StateFor(env)->baseline = nativeapi::HandleTable::GetInstance().LiveCount();
  return Undefined(env);
}
napi_value Drain(napi_env env, napi_callback_info) {
  std::vector<std::function<void()>> work;
  {
    std::lock_guard<std::mutex> lock(queue_mutex);
    work.swap(queue);
  }
  for (auto& fn : work)
    fn();
  return Undefined(env);
}
napi_value EmitShortcut(napi_env env, napi_callback_info) {
  std::thread producer([] {
    nativeapi::ShortcutManager::GetInstance().Emit(nativeapi::ShortcutRegistrationFailedEvent(
        991, "Ctrl+ForeignThread", "owned UTF-8 payload: 异步确认"));
  });
  producer.join();
  return Undefined(env);
}
napi_value EmitDisplay(napi_env env, napi_callback_info) {
  std::thread producer([] {
    nativeapi::DisplayManager::GetInstance().Emit(
        nativeapi::DisplayAddedEvent(std::make_shared<nativeapi::Display>(nullptr)));
  });
  producer.join();
  return Undefined(env);
}
napi_value Live(napi_env env, napi_callback_info) {
  napi_value value;
  napi_create_bigint_uint64(env, nativeapi::HandleTable::GetInstance().LiveCount(), &value);
  return value;
}
napi_value ApplicationListeners(napi_env env, napi_callback_info) {
  napi_value value;
  napi_create_uint32(env, nativeapi::Application::GetInstance().GetTotalListenerCount(), &value);
  return value;
}
napi_value Outcome(napi_env env, napi_callback_info) {
  napi_value value;
  napi_create_int32(env, StateFor(env)->outcome.load(), &value);
  return value;
}
napi_value Request(napi_env env, napi_callback_info info) {
  auto state = StateFor(env);
  if (!state->runtime_initialized) {
    // Each addon has its own napi_env even within a Node worker. Register the
    // fixture's callback context after the production addon, so both use the
    // actual runtime and have their own teardown hooks.
    nativeapi_js::InitRuntime(env);
    state->runtime_initialized = true;
  }
  nativeapi_js::Args args(env, info);
  nativeapi_js::Callback* callback = nullptr;
  bool cancelable = true;
  if (!nativeapi_js::GetCallback(env, args[0], false, &callback) ||
      !nativeapi_js::GetBool(env, args[1], &cancelable))
    return nullptr;
  state->registration = std::make_shared<nativeapi::capi::EventDeliveryRegistration>(
      nativeapi::capi::UserData::Make(callback, &nativeapi_js::Callback::ReleaseUserData));
  state->outcome = -1;
  std::weak_ptr<FixtureState> weak = state;
  state->request = nativeapi::detail::EventRequestDispatch::Create(cancelable, [weak](bool accept) {
    if (auto state = weak.lock())
      state->outcome = accept ? 1 : 0;
  });
  auto request = state->request;
  auto handle = nativeapi::HandleTable::GetInstance().Insert(request);
  auto payload = std::shared_ptr<void>(new uint64_t(handle), [](void* value) {
    nativeapi::HandleTable::GetInstance().Release(*static_cast<uint64_t*>(value));
    delete static_cast<uint64_t*>(value);
  });
  auto vote = cancelable ? request->Defer() : nullptr;
  auto delivery =
      nativeapi::HandleTable::GetInstance().Insert(std::make_shared<nativeapi::capi::EventDelivery>(
          state->registration->context, payload, vote));
  std::thread producer([callback, handle, delivery] {
    nativeapi_js::Callback::DispatchEvent(
        callback,
        {nativeapi_js::Value::Object().Set("request", nativeapi_js::Value::BigInt(handle))},
        delivery);
  });
  producer.join();
  nativeapi::detail::EventRequestDispatch::Finish(request);
  return Undefined(env);
}
napi_value RemoveRequest(napi_env env, napi_callback_info) {
  StateFor(env)->registration.reset();
  return Undefined(env);
}
napi_value CheckCleanup(napi_env env, napi_callback_info info) {
  auto state = StateFor(env);
  nativeapi_js::Args args(env, info);
  if (!nativeapi_js::IsNullish(env, args[0]) &&
      !nativeapi_js::GetNumber(env, args[0], &state->expected_outcome))
    return nullptr;
  state->check_cleanup = true;
  return Undefined(env);
}
void Cleanup(void* data) {
  auto keeper = std::unique_ptr<std::shared_ptr<FixtureState>>(
      static_cast<std::shared_ptr<FixtureState>*>(data));
  auto state = *keeper;
  // Both fixture and production runtime hooks run before this observer.
  if (state->check_cleanup) {
    const auto live = nativeapi::HandleTable::GetInstance().LiveCount();
    std::printf("cleanup-outcome=%d live=%llu baseline=%llu\n", state->outcome.load(),
                static_cast<unsigned long long>(live),
                static_cast<unsigned long long>(state->baseline));
    std::fflush(stdout);
    if (state->outcome != state->expected_outcome || live != state->baseline)
      std::_Exit(88);
  }
  state->registration.reset();
  state->request.reset();
  bool last;
  {
    std::lock_guard<std::mutex> lock(states_mutex);
    states.erase(state->env);
    last = states.empty();
  }
  if (last)
    nativeapi::SetMainThreadDispatcher(nullptr, nullptr);
}

napi_value Init(napi_env env, napi_value exports) {
#if defined(__APPLE__)
  // Keep Application initialization headless: an existing host prevents the
  // core from installing standalone activation defaults. Never run AppKit.
  if ([NSThread isMainThread] && !quit_host) {
    [NSApplication sharedApplication];
    quit_host = [[JsQuitTestHost alloc] init];
    NSApp.delegate = quit_host;
  }
#endif
  auto state = std::make_shared<FixtureState>();
#ifdef NATIVEAPI_JS_EMBEDDED_FIXTURE
  state->runtime_initialized = true;
#endif
  state->env = env;
  {
    std::lock_guard<std::mutex> lock(states_mutex);
    states.emplace(env, state);
  }
  napi_property_descriptor methods[] = {
      {"schedule", nullptr, Schedule, nullptr, nullptr, nullptr, napi_default, nullptr},
      {"drain", nullptr, Drain, nullptr, nullptr, nullptr, napi_default, nullptr},
      {"emitShortcut", nullptr, EmitShortcut, nullptr, nullptr, nullptr, napi_default, nullptr},
      {"emitDisplay", nullptr, EmitDisplay, nullptr, nullptr, nullptr, napi_default, nullptr},
      {"live", nullptr, Live, nullptr, nullptr, nullptr, napi_default, nullptr},
      {"request", nullptr, Request, nullptr, nullptr, nullptr, napi_default, nullptr},
      {"outcome", nullptr, Outcome, nullptr, nullptr, nullptr, napi_default, nullptr},
      {"applicationListeners", nullptr, ApplicationListeners, nullptr, nullptr, nullptr,
       napi_default, nullptr},
      {"removeRequest", nullptr, RemoveRequest, nullptr, nullptr, nullptr, napi_default, nullptr},
      {"checkCleanup", nullptr, CheckCleanup, nullptr, nullptr, nullptr, napi_default, nullptr},
  };
  napi_define_properties(env, exports, sizeof(methods) / sizeof(methods[0]), methods);
  napi_add_env_cleanup_hook(env, Cleanup, new std::shared_ptr<FixtureState>(state));
  return exports;
}
}  // namespace
#ifdef NATIVEAPI_JS_EMBEDDED_FIXTURE
namespace nativeapi_js {
napi_value InitEventDeliveryFixture(napi_env env, napi_value exports) {
  return Init(env, exports);
}
}  // namespace nativeapi_js
#else
NAPI_MODULE(event_delivery_fixture, Init)
#endif
