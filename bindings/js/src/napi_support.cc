#include "napi_support.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <limits>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "application_quit_dispatch.h"
#include "capi/event_request_c.h"
#include "foundation/dispatcher.h"

namespace nativeapi_js {

struct RuntimeState {
  napi_env env;
  std::thread::id js_thread = std::this_thread::get_id();
  std::atomic<bool> alive{true};
  bool hop_to_main_thread = false;
  std::shared_ptr<void> loop_owner;
  explicit RuntimeState(napi_env value) : env(value) {}
};

namespace {
std::mutex g_runtime_mutex;
std::unordered_map<napi_env, std::shared_ptr<RuntimeState>> g_runtimes;
std::unordered_map<Callback*, std::shared_ptr<Callback>> g_callbacks;
std::unordered_map<uint64_t, std::shared_ptr<RuntimeState>> g_event_deliveries;
std::unordered_map<uint64_t, std::shared_ptr<RuntimeState>> g_event_decisions;
thread_local std::weak_ptr<RuntimeState> current_runtime;
std::atomic<uintptr_t> next_callback_token{1};
Callback* NewCallbackToken() {
  auto next = next_callback_token.load();
  while (next != std::numeric_limits<uintptr_t>::max()) {
    if (next_callback_token.compare_exchange_weak(next, next + 1))
      return reinterpret_cast<Callback*>(next);
  }
  return nullptr;
}

std::shared_ptr<RuntimeState> RuntimeFor(napi_env env) {
  std::lock_guard<std::mutex> lock(g_runtime_mutex);
  auto found = g_runtimes.find(env);
  return found == g_runtimes.end() ? nullptr : found->second;
}
bool IsJsThread(const std::shared_ptr<RuntimeState>& runtime) {
  return runtime && std::this_thread::get_id() == runtime->js_thread;
}
std::shared_ptr<Callback> CallbackFor(void* data) {
  std::lock_guard<std::mutex> lock(g_runtime_mutex);
  auto found = g_callbacks.find(static_cast<Callback*>(data));
  return found == g_callbacks.end() ? nullptr : found->second;
}

bool Throw(napi_env env, const std::string& message) {
  napi_throw_type_error(env, nullptr, message.c_str());
  return false;
}

bool TypeIs(napi_env env, napi_value value, napi_valuetype expected) {
  napi_valuetype type = napi_undefined;
  return napi_typeof(env, value, &type) == napi_ok && type == expected;
}

}  // namespace

// ---------------------------------------------------------------------------
// Value
// ---------------------------------------------------------------------------

Value Value::Bool(bool value) {
  Value result(Kind::kBool);
  result.bool_ = value;
  return result;
}

Value Value::Number(double value) {
  Value result(Kind::kNumber);
  result.number_ = value;
  return result;
}

Value Value::BigInt(uint64_t value) {
  Value result(Kind::kBigInt);
  result.bigint_ = value;
  return result;
}

Value Value::String(const char* value) {
  if (value == nullptr) {
    return Null();
  }
  return String(std::string(value));
}

Value Value::String(std::string value) {
  Value result(Kind::kString);
  result.string_ = std::move(value);
  return result;
}

Value& Value::Push(Value item) {
  items_.push_back(std::move(item));
  return *this;
}

Value& Value::Set(std::string key, Value item) {
  fields_.emplace_back(std::move(key), std::move(item));
  return *this;
}

napi_value Value::ToJs(napi_env env) const {
  napi_value result = nullptr;
  switch (kind_) {
    case Kind::kUndefined:
      napi_get_undefined(env, &result);
      break;
    case Kind::kNull:
      napi_get_null(env, &result);
      break;
    case Kind::kBool:
      napi_get_boolean(env, bool_, &result);
      break;
    case Kind::kNumber:
      napi_create_double(env, number_, &result);
      break;
    case Kind::kBigInt:
      napi_create_bigint_uint64(env, bigint_, &result);
      break;
    case Kind::kString:
      napi_create_string_utf8(env, string_.data(), string_.size(), &result);
      break;
    case Kind::kArray:
      napi_create_array_with_length(env, items_.size(), &result);
      for (size_t i = 0; i < items_.size(); ++i) {
        napi_set_element(env, result, static_cast<uint32_t>(i), items_[i].ToJs(env));
      }
      break;
    case Kind::kObject:
      napi_create_object(env, &result);
      for (const auto& [key, item] : fields_) {
        napi_set_named_property(env, result, key.c_str(), item.ToJs(env));
      }
      break;
  }
  return result;
}

Value CopyStringList(const native_string_list_t& list) {
  Value result = Value::Array();
  for (long i = 0; list.items != nullptr && i < list.count; ++i) {
    result.Push(Value::String(list.items[i] ? list.items[i] : ""));
  }
  return result;
}

Value CopyStringMap(const native_string_map_t& map) {
  Value result = Value::Object();
  for (long i = 0; map.keys != nullptr && map.values != nullptr && i < map.count; ++i) {
    if (map.keys[i] != nullptr) {
      result.Set(map.keys[i], Value::String(map.values[i] ? map.values[i] : ""));
    }
  }
  return result;
}

// ---------------------------------------------------------------------------
// Arguments
// ---------------------------------------------------------------------------

Args::Args(napi_env env, napi_callback_info info) : env_(env) {
  current_runtime = RuntimeFor(env);
  ok_ = napi_get_cb_info(env, info, &count_, argv_, nullptr, nullptr) == napi_ok;
  if (count_ > kMaxArgs) {
    count_ = kMaxArgs;
  }
}

napi_value Args::operator[](size_t index) const {
  if (index < count_ && argv_[index] != nullptr) {
    return argv_[index];
  }
  return Undefined(env_);
}

char* Arena::Keep(std::string value) {
  strings_.push_back(std::move(value));
  return strings_.back().data();
}

char** Arena::KeepArray(std::vector<char*> items) {
  arrays_.push_back(std::move(items));
  return arrays_.back().data();
}

napi_value Undefined(napi_env env) {
  napi_value result = nullptr;
  napi_get_undefined(env, &result);
  return result;
}

bool IsNullish(napi_env env, napi_value value) {
  napi_valuetype type = napi_undefined;
  napi_typeof(env, value, &type);
  return type == napi_undefined || type == napi_null;
}

bool ExpectObject(napi_env env, napi_value value, const char* type_name) {
  if (!TypeIs(env, value, napi_object)) {
    return Throw(env, std::string("expected a ") + type_name + " object");
  }
  return true;
}

bool GetField(napi_env env, napi_value object, const char* name, napi_value* out) {
  napi_value field = nullptr;
  if (napi_get_named_property(env, object, name, &field) != napi_ok) {
    return false;
  }
  *out = TypeIs(env, field, napi_undefined) ? nullptr : field;
  return true;
}

bool GetBool(napi_env env, napi_value value, bool* out) {
  if (napi_get_value_bool(env, value, out) != napi_ok) {
    return Throw(env, "expected a boolean");
  }
  return true;
}

bool GetDouble(napi_env env, napi_value value, double* out) {
  if (TypeIs(env, value, napi_bigint)) {
    int64_t number = 0;
    bool lossless = false;
    napi_get_value_bigint_int64(env, value, &number, &lossless);
    *out = static_cast<double>(number);
    return true;
  }
  if (napi_get_value_double(env, value, out) != napi_ok) {
    return Throw(env, "expected a number");
  }
  return true;
}

bool GetHandle(napi_env env, napi_value value, uint64_t* out) {
  if (IsNullish(env, value)) {
    *out = 0;
    return true;
  }
  if (TypeIs(env, value, napi_bigint)) {
    bool lossless = false;
    napi_get_value_bigint_uint64(env, value, out, &lossless);
    return true;
  }
  double number = 0;
  if (napi_get_value_double(env, value, &number) != napi_ok) {
    return Throw(env, "expected a handle (bigint)");
  }
  *out = static_cast<uint64_t>(number);
  return true;
}

bool GetPointer(napi_env env, napi_value value, void** out) {
  uint64_t address = 0;
  if (!GetHandle(env, value, &address)) {
    return false;
  }
  *out = reinterpret_cast<void*>(static_cast<uintptr_t>(address));
  return true;
}

bool GetString(napi_env env, napi_value value, Arena& arena, char** out) {
  size_t length = 0;
  if (napi_get_value_string_utf8(env, value, nullptr, 0, &length) != napi_ok) {
    return Throw(env, "expected a string");
  }
  std::string buffer(length, '\0');
  napi_get_value_string_utf8(env, value, buffer.data(), length + 1, &length);
  *out = arena.Keep(std::move(buffer));
  return true;
}

bool GetString(napi_env env, napi_value value, Arena& arena, const char** out) {
  char* text = nullptr;
  if (!GetString(env, value, arena, &text)) {
    return false;
  }
  *out = text;
  return true;
}

bool GetOptionalString(napi_env env, napi_value value, Arena& arena, char** out) {
  if (IsNullish(env, value)) {
    *out = nullptr;
    return true;
  }
  return GetString(env, value, arena, out);
}

bool GetOptionalString(napi_env env, napi_value value, Arena& arena, const char** out) {
  char* text = nullptr;
  if (!GetOptionalString(env, value, arena, &text)) {
    return false;
  }
  *out = text;
  return true;
}

bool GetStringList(napi_env env, napi_value value, Arena& arena, native_string_list_t* out) {
  bool is_array = false;
  if (napi_is_array(env, value, &is_array) != napi_ok || !is_array) {
    return Throw(env, "expected an array of strings");
  }
  uint32_t length = 0;
  napi_get_array_length(env, value, &length);
  std::vector<char*> items(length, nullptr);
  for (uint32_t i = 0; i < length; ++i) {
    napi_value item = nullptr;
    napi_get_element(env, value, i, &item);
    if (!GetString(env, item, arena, &items[i])) {
      return false;
    }
  }
  out->items = arena.KeepArray(std::move(items));
  out->count = static_cast<long>(length);
  return true;
}

bool GetStringMap(napi_env env, napi_value value, Arena& arena, native_string_map_t* out) {
  if (!ExpectObject(env, value, "string map")) {
    return false;
  }
  napi_value names = nullptr;
  napi_get_property_names(env, value, &names);
  uint32_t length = 0;
  napi_get_array_length(env, names, &length);
  std::vector<char*> keys(length, nullptr);
  std::vector<char*> values(length, nullptr);
  for (uint32_t i = 0; i < length; ++i) {
    napi_value key = nullptr;
    napi_value item = nullptr;
    napi_get_element(env, names, i, &key);
    napi_get_property(env, value, key, &item);
    if (!GetString(env, key, arena, &keys[i]) || !GetString(env, item, arena, &values[i])) {
      return false;
    }
  }
  out->keys = arena.KeepArray(std::move(keys));
  out->values = arena.KeepArray(std::move(values));
  out->count = static_cast<long>(length);
  return true;
}

// ---------------------------------------------------------------------------
// Callbacks
// ---------------------------------------------------------------------------

namespace {

// Queued in place of call arguments: release the Callback instead.
char g_release_marker;

struct QueuedCall {
  std::vector<Value> args;
  uint64_t delivery = 0;
  ~QueuedCall() {
    if (delivery)
      Callback::CompleteEvent(delivery, false);
  }
};

void ReportException(napi_env env) {
  bool pending = false;
  if (napi_is_exception_pending(env, &pending) != napi_ok || !pending) {
    return;
  }
  napi_value error = nullptr;
  napi_get_and_clear_last_exception(env, &error);
  // Same as an exception thrown from any other event handler: it surfaces as
  // an uncaught exception rather than vanishing inside native code.
  napi_fatal_exception(env, error);
}

}  // namespace

bool GetCallback(napi_env env, napi_value value, bool optional, Callback** out) {
  *out = nullptr;
  if (optional && IsNullish(env, value)) {
    return true;
  }
  if (!TypeIs(env, value, napi_function)) {
    return Throw(env, "expected a function");
  }
  auto owner = std::shared_ptr<Callback>(new Callback());
  auto* callback = owner.get();
  auto* token = NewCallbackToken();
  if (!token)
    return Throw(env, "callback tokens exhausted");
  callback->runtime_ = RuntimeFor(env);
  if (!callback->runtime_ || !callback->runtime_->alive.load())
    return Throw(env, "nativeapi environment is closed");
  callback->env_ = env;
  if (napi_create_reference(env, value, 1, &callback->function_) != napi_ok)
    return Throw(env, "could not retain callback");
  try {
    std::lock_guard<std::mutex> lock(g_runtime_mutex);
    g_callbacks.emplace(token, owner);
  } catch (...) {
    napi_delete_reference(env, callback->function_);
    return Throw(env, "could not retain callback owner");
  }
  auto queue_owner = std::make_unique<std::shared_ptr<Callback>>(owner);
  napi_value name = nullptr;
  napi_create_string_utf8(env, "nativeapi callback", NAPI_AUTO_LENGTH, &name);
  // The token registry and the TSFN own separate references. Closing an env
  // discards its registry entries; any later native use of the opaque token
  // finds no callback, while queued TSFN contexts stay alive through finalizing.
  auto finalize = [](napi_env env, void* data, void*) {
    auto keeper =
        std::unique_ptr<std::shared_ptr<Callback>>(static_cast<std::shared_ptr<Callback>*>(data));
    auto callback = *keeper;
    napi_ref function;
    {
      std::lock_guard<std::mutex> lock(g_runtime_mutex);
      callback->queue_ = nullptr;
      function = callback->function_;
      callback->function_ = nullptr;
    }
    if (env && callback->runtime_->alive.load() && function)
      napi_delete_reference(env, function);
  };
  if (napi_create_threadsafe_function(env, nullptr, nullptr, name, 0, 1, queue_owner.get(),
                                      finalize, callback, &Callback::CallFromQueue,
                                      &callback->queue_) == napi_ok) {
    queue_owner.release();
    napi_unref_threadsafe_function(env, callback->queue_);
  } else {
    napi_delete_reference(env, callback->function_);
    std::lock_guard<std::mutex> lock(g_runtime_mutex);
    g_callbacks.erase(token);
    return Throw(env, "could not create callback queue");
  }
  *out = token;
  return true;
}

void Callback::AttachRegistration(void* user_data, std::function<void()> remove) {
  auto callback = CallbackFor(user_data);
  if (!callback)
    return;
  std::lock_guard<std::mutex> lock(g_runtime_mutex);
  callback->remove_registration_ = std::move(remove);
}

void Callback::Dispatch(void* user_data, std::vector<Value> args) {
  DispatchCall(user_data, std::move(args), 0);
}

bool Callback::CompleteEvent(uint64_t delivery, bool accept) {
  {
    std::lock_guard<std::mutex> lock(g_runtime_mutex);
    if (g_event_deliveries.erase(delivery) == 0)
      return false;
  }
  return native_event_delivery_complete(delivery, accept);
}

void Callback::DispatchEvent(void* user_data, std::vector<Value> args, uint64_t delivery) {
  bool registered = false;
  try {
    auto callback = CallbackFor(user_data);
    {
      std::lock_guard<std::mutex> lock(g_runtime_mutex);
      if (callback && callback->runtime_->alive.load()) {
        g_event_deliveries.emplace(delivery, callback->runtime_);
        registered = true;
      }
    }
    if (!registered) {
      // Existing deliveries are vetoed during cleanup. A fresh event emitted
      // after the environment closes has no consumer; an inert registration
      // awaiting UI removal must not veto another environment's new request.
      native_event_delivery_complete(delivery, callback && !callback->runtime_->alive.load());
      return;
    }
    args.push_back(Value::BigInt(delivery));
    DispatchCall(user_data, std::move(args), delivery);
  } catch (...) {
    if (registered)
      CompleteEvent(delivery, false);
    else
      native_event_delivery_complete(delivery, false);
    throw;
  }
}

void Callback::DispatchCall(void* user_data, std::vector<Value> args, uint64_t delivery) {
  auto queued = std::make_unique<QueuedCall>();
  queued->args = std::move(args);
  queued->delivery = delivery;
  auto callback = CallbackFor(user_data);
  if (!callback) {
    return;
  }
  if (IsJsThread(callback->runtime_)) {
    if (!callback->runtime_->alive.load())
      return;
    auto env = callback->env_;
    // Native code only runs on this thread from inside a call we made (an API
    // call, or the event loop pump), so the engine is in a callable state.
    napi_handle_scope scope = nullptr;
    if (napi_open_handle_scope(env, &scope) != napi_ok)
      return;
    if (callback->Call(env, queued->args)) {
      queued->delivery = 0;
    } else {
      queued.reset();
      ReportException(env);
    }
    napi_close_handle_scope(env, scope);
    return;
  }
  napi_threadsafe_function queue = nullptr;
  {
    // Cleanup/finalization cannot free the callback while acquiring this
    // temporary producer reference. It keeps the TSFN alive until enqueue ends.
    std::lock_guard<std::mutex> lock(g_runtime_mutex);
    if (callback->runtime_->alive.load() && callback->queue_ != nullptr &&
        napi_acquire_threadsafe_function(callback->queue_) == napi_ok)
      queue = callback->queue_;
  }
  if (queue != nullptr) {
    if (napi_call_threadsafe_function(queue, queued.get(), napi_tsfn_nonblocking) == napi_ok)
      queued.release();
    napi_release_threadsafe_function(queue, napi_tsfn_release);
  }
}

void Callback::CallFromQueue(napi_env env, napi_value, void* context, void* data) {
  if (data == &g_release_marker) {
    if (env != nullptr && static_cast<Callback*>(context)->runtime_->alive.load()) {
      static_cast<Callback*>(context)->ReleaseOnJsThread();
    }
    return;
  }
  auto queued = std::unique_ptr<QueuedCall>(static_cast<QueuedCall*>(data));
  if (env != nullptr && static_cast<Callback*>(context)->runtime_->alive.load()) {
    if (static_cast<Callback*>(context)->Call(env, queued->args)) {
      queued->delivery = 0;
    } else {
      queued.reset();
      ReportException(env);
    }
  }
}

bool Callback::Call(napi_env env, const std::vector<Value>& args) {
  if (function_ == nullptr) {
    return false;
  }
  napi_value function = nullptr;
  if (napi_get_reference_value(env, function_, &function) != napi_ok || function == nullptr) {
    return false;
  }
  std::vector<napi_value> argv;
  argv.reserve(args.size());
  for (const auto& arg : args) {
    auto value = arg.ToJs(env);
    if (!value)
      return false;
    argv.push_back(value);
  }
  napi_value result = nullptr;
  if (napi_call_function(env, Undefined(env), function, argv.size(), argv.data(), &result) !=
      napi_ok) {
    return false;
  }
  return true;
}

void Callback::ReleaseUserData(void* user_data) {
  std::shared_ptr<Callback> callback;
  {
    std::lock_guard<std::mutex> lock(g_runtime_mutex);
    auto found = g_callbacks.find(static_cast<Callback*>(user_data));
    if (found == g_callbacks.end())
      return;
    callback = std::move(found->second);
    g_callbacks.erase(found);
  }
  if (!callback->runtime_->alive.load())
    return;
  if (IsJsThread(callback->runtime_)) {
    callback->ReleaseOnJsThread();
  } else {
    napi_threadsafe_function queue = nullptr;
    {
      std::lock_guard<std::mutex> lock(g_runtime_mutex);
      if (callback->runtime_->alive.load() && callback->queue_ != nullptr &&
          napi_acquire_threadsafe_function(callback->queue_) == napi_ok)
        queue = callback->queue_;
    }
    if (queue) {
      if (napi_call_threadsafe_function(queue, &g_release_marker, napi_tsfn_nonblocking) != napi_ok)
        napi_release_threadsafe_function(queue, napi_tsfn_release);  // Core's producer reference.
      napi_release_threadsafe_function(queue, napi_tsfn_release);    // Temporary enqueue reference.
    }
  }
}

void Callback::ReleaseOnJsThread() {
  if (function_ != nullptr) {
    napi_delete_reference(env_, function_);
    function_ = nullptr;
  }
  if (queue_ != nullptr) {
    napi_threadsafe_function queue;
    {
      std::lock_guard<std::mutex> lock(g_runtime_mutex);
      queue = queue_;
      queue_ = nullptr;
    }
    napi_release_threadsafe_function(queue,
                                     napi_tsfn_release);  // finalizer frees this
  }
}

// ---------------------------------------------------------------------------
// Module setup
// ---------------------------------------------------------------------------

bool NeedsMainThreadHop() {
  auto runtime = current_runtime.lock();
  return runtime && runtime->hop_to_main_thread && !nativeapi::IsMainThread();
}

void RunOnMainThreadSync(const std::function<void()>& work) {
  std::mutex mutex;
  std::condition_variable done_signal;
  bool done = false;
  bool posted = nativeapi::RunOnMainThread([&] {
    work();
    std::lock_guard<std::mutex> lock(mutex);
    done = true;
    done_signal.notify_one();
  });
  if (!posted) {
    // No UI thread to post to; calling here is the best that is left.
    work();
    return;
  }
  std::unique_lock<std::mutex> lock(mutex);
  done_signal.wait(lock, [&] { return done; });
}

namespace {

// Whether something on the UI thread drains its queue — a host running the
// platform loop, as under `deno desktop`. `deno test` also runs JS off the main
// thread, but its main thread only waits in Deno's own loop: hopping there
// would block forever, so calls stay on the JS thread instead.
bool MainThreadIsServiced() {
  struct Probe {
    std::mutex mutex;
    std::condition_variable signal;
    bool ran = false;
  };
  // Shared, because a late run can land after this returns.
  auto probe = std::make_shared<Probe>();
  bool posted = nativeapi::RunOnMainThread([probe] {
    std::lock_guard<std::mutex> lock(probe->mutex);
    probe->ran = true;
    probe->signal.notify_one();
  });
  if (!posted) {
    return false;
  }
  std::unique_lock<std::mutex> lock(probe->mutex);
  return probe->signal.wait_for(lock, std::chrono::seconds(1), [&] { return probe->ran; });
}

}  // namespace

void InitRuntime(napi_env env) {
  auto runtime = std::make_shared<RuntimeState>(env);
  runtime->hop_to_main_thread = !nativeapi::IsMainThread() &&
                                nativeapi::IsMainThreadDispatchSupported() &&
                                MainThreadIsServiced();
  {
    std::lock_guard<std::mutex> lock(g_runtime_mutex);
    g_runtimes[env] = runtime;
  }
  current_runtime = runtime;
  auto keeper = new std::shared_ptr<RuntimeState>(runtime);
  napi_add_env_cleanup_hook(
      env,
      [](void* data) {
        auto keeper = std::unique_ptr<std::shared_ptr<RuntimeState>>(
            static_cast<std::shared_ptr<RuntimeState>*>(data));
        auto runtime = *keeper;
        std::shared_ptr<void> loop_owner;
        {
          std::lock_guard<std::mutex> lock(g_runtime_mutex);
          runtime->alive.store(false);
          loop_owner = std::move(runtime->loop_owner);
          auto found = g_runtimes.find(runtime->env);
          if (found != g_runtimes.end() && found->second == runtime)
            g_runtimes.erase(found);
        }
        // Destroying a vote or delivery can call native continuations. Do it
        // outside the runtime lock and without calling back into the closing JS
        // environment.
        if (loop_owner) {
          std::weak_ptr<void> old_owner = loop_owner;
          loop_owner.reset();
          nativeapi::detail::ApplicationQuitDispatch::CancelForLoop(old_owner);
        }
        for (;;) {
          std::function<void()> remove;
          {
            std::lock_guard<std::mutex> lock(g_runtime_mutex);
            for (const auto& entry : g_callbacks) {
              if (entry.second->runtime_ == runtime && entry.second->remove_registration_) {
                remove.swap(entry.second->remove_registration_);
                break;
              }
            }
          }
          if (!remove)
            break;
          if (runtime->hop_to_main_thread && !nativeapi::IsMainThread()) {
            try {
              (void)nativeapi::RunOnMainThread(std::move(remove));
            } catch (...) {
            }
          } else {
            try {
              remove();
            } catch (...) {
            }
          }
        }
        auto take = [&](auto& handles) -> uint64_t {
          std::lock_guard<std::mutex> lock(g_runtime_mutex);
          for (auto it = handles.begin(); it != handles.end(); ++it) {
            if (it->second == runtime) {
              const auto handle = it->first;
              handles.erase(it);
              return handle;
            }
          }
          return 0;
        };
        // Removing one entry at a time requires no allocation during teardown.
        while (auto delivery = take(g_event_deliveries))
          native_event_delivery_complete(delivery, false);
        while (auto decision = take(g_event_decisions)) {
          native_event_decision_cancel(decision);
          native_event_decision_free(decision);
        }
        for (;;) {
          std::shared_ptr<Callback> callback;
          {
            std::lock_guard<std::mutex> lock(g_runtime_mutex);
            for (auto it = g_callbacks.begin(); it != g_callbacks.end(); ++it) {
              if (it->second->runtime_ == runtime) {
                callback = std::move(it->second);
                g_callbacks.erase(it);
                break;
              }
            }
          }
          if (!callback)
            break;
          // Core release tasks carry a monotonic opaque token, not this object's
          // address. A late release cannot match an allocation in a new env.
        }
      },
      keeper);
}

uint64_t TrackEventDecision(napi_env env, uint64_t handle) noexcept {
  if (!handle)
    return 0;
  try {
    auto runtime = RuntimeFor(env);
    std::lock_guard<std::mutex> lock(g_runtime_mutex);
    if (runtime && runtime->alive.load()) {
      g_event_decisions.emplace(handle, runtime);
      return handle;
    }
  } catch (...) {
  }
  native_event_decision_cancel(handle);
  native_event_decision_free(handle);
  return 0;
}
void ForgetEventDecision(uint64_t handle) {
  std::lock_guard<std::mutex> lock(g_runtime_mutex);
  g_event_decisions.erase(handle);
}
std::shared_ptr<void> EventLoopOwner(napi_env env) {
  auto runtime = RuntimeFor(env);
  std::lock_guard<std::mutex> lock(g_runtime_mutex);
  return runtime && runtime->alive.load() ? runtime->loop_owner : nullptr;
}
void EndEventLoopSession(napi_env env) {
  auto runtime = RuntimeFor(env);
  std::shared_ptr<void> owner;
  {
    std::lock_guard<std::mutex> lock(g_runtime_mutex);
    if (runtime)
      owner = std::move(runtime->loop_owner);
  }
  if (owner) {
    std::weak_ptr<void> old_owner = owner;
    owner.reset();
    nativeapi::detail::ApplicationQuitDispatch::CancelForLoop(old_owner);
  }
}
bool BeginEventLoopSession(napi_env env) {
  EndEventLoopSession(env);
  auto runtime = RuntimeFor(env);
  auto owner = std::make_shared<int>(0);
  std::lock_guard<std::mutex> lock(g_runtime_mutex);
  if (!runtime || !runtime->alive.load())
    return false;
  runtime->loop_owner = owner;
  return true;
}

void Export(napi_env env, napi_value exports, const char* name, napi_callback callback) {
  napi_value function = nullptr;
  napi_create_function(env, name, NAPI_AUTO_LENGTH, callback, nullptr, &function);
  napi_set_named_property(env, exports, name, function);
}

void ExportValue(napi_env env, napi_value exports, const char* name, const Value& value) {
  napi_set_named_property(env, exports, name, value.ToJs(env));
}

}  // namespace nativeapi_js
