#include "event_loop.h"

#include <algorithm>
#include <atomic>
#include <limits>
#include <memory>
#include <mutex>
#include <vector>

#include "application_quit_dispatch.h"
#include "capi/application_c.h"
#include "capi/window_c.h"
#include "foundation/dispatcher.h"

namespace nativeapi_py {

void ShowPrimaryWindow(uint64_t window) {
  if (window == 0) {
    return;
  }
  native_application_set_primary_window(window);
  native_window_show(window);
  native_window_focus(window);
}

}  // namespace nativeapi_py

bool nativeapi_py_is_main_thread(void) {
  return nativeapi::IsMainThread();
}

namespace {
struct QuitCallback {
  nativeapi_py_quit_callback_t callback;
  std::shared_ptr<void> data;
  std::atomic<bool> released{false};

  QuitCallback(nativeapi_py_quit_callback_t fn, std::shared_ptr<void> value)
      : callback(fn), data(std::move(value)) {}
  ~QuitCallback() { Release(); }
  void Release() {
    if (!released.exchange(true))
      data.reset();
  }
  void Complete(int code) {
    // End/complete are serialized on the loop's UI thread. Queued requests
    // may retain this context past interpreter exit; its cleared callback
    // data must then never be read or released a second time.
    if (!released.load())
      callback(code, data.get());
  }
};
std::mutex session_mutex;
uint64_t session_id = 0;
std::shared_ptr<void> session_owner;
bool session_requested_quit = false;
std::vector<std::weak_ptr<QuitCallback>> session_callbacks;
}  // namespace

uint64_t nativeapi_py_begin_event_loop(void) try {
  std::lock_guard<std::mutex> lock(session_mutex);
  if (session_owner || session_id == std::numeric_limits<uint64_t>::max())
    return 0;
  session_owner = std::make_shared<int>(0);
  session_requested_quit = false;
  session_callbacks.clear();
  return ++session_id;
} catch (...) {
  return 0;
}

void nativeapi_py_end_event_loop(uint64_t session) try {
  std::weak_ptr<void> owner;
  bool requested;
  std::vector<std::weak_ptr<QuitCallback>> callbacks;
  {
    std::lock_guard<std::mutex> lock(session_mutex);
    if (!session_owner || session != session_id)
      return;
    owner = session_owner;
    requested = session_requested_quit;
    callbacks.swap(session_callbacks);
    session_owner.reset();
  }
  // Release ctypes trampolines before interpreter teardown, including work
  // still parked in an OS queue which may never be pumped again.
  for (auto& weak : callbacks) {
    if (auto callback = weak.lock())
      callback->Release();
  }
  if (requested)
    nativeapi::detail::ApplicationQuitDispatch::CancelForLoop(std::move(owner));
} catch (...) {
  // No C++ exception crosses ctypes. The expired owner also fences late work.
}

bool nativeapi_py_request_event_loop_quit(uint64_t session,
                                          int exit_code,
                                          nativeapi_py_quit_callback_t callback,
                                          void* user_data,
                                          nativeapi_py_release_callback_t release_user_data) {
  try {
    // This callback is an internal ctypes completion, whose release reacquires
    // the GIL. Release inline: once asyncio stops pumping, posting its final
    // release to a platform queue would retain the callback until another run.
    // shared_ptr also invokes the deleter if control-block allocation fails.
    auto data = std::shared_ptr<void>(user_data, [release_user_data](void* value) {
      if (release_user_data)
        release_user_data(value);
    });
    auto context = std::make_shared<QuitCallback>(callback, std::move(data));
    std::weak_ptr<void> owner;
    {
      std::lock_guard<std::mutex> lock(session_mutex);
      if (!session_owner || session != session_id || !callback)
        return false;
      owner = session_owner;
      session_requested_quit = true;
      session_callbacks.erase(std::remove_if(session_callbacks.begin(), session_callbacks.end(),
                                             [](const auto& weak) { return weak.expired(); }),
                              session_callbacks.end());
      session_callbacks.push_back(context);
    }
    auto request = [owner, context, exit_code] {
      if (auto live = owner.lock()) {
        nativeapi::detail::ApplicationQuitDispatch::RequestForLoop(
            exit_code, [context](int code) { context->Complete(code); }, std::move(live));
      }
    };
    if (nativeapi::IsMainThread()) {
      request();
      return true;
    }
    return nativeapi::RunOnMainThread(std::move(request));
  } catch (...) {
    return false;
  }
}
