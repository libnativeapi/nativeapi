#include "swift_ohos_runtime.h"
#include "swift_runtime_support.h"

#include <cerrno>
#include <cstdint>
#include <spawn.h>
#include <unistd.h>
#include <uv.h>

extern "C" {
int _dispatch_get_main_queue_port_4CF(void);
void _dispatch_main_queue_callback_4CF(void*);

// Swift's Static Linux Foundation archive references these musl/glibc symbols,
// which are not exported by the OpenHarmony NDK libc. Keep them weak and hidden
// so host overrides or future NDK additions take precedence cleanly.
static char g_swift_ohos_process_name[] = "nativeapi.swift.ohos";
__attribute__((weak, visibility("hidden"))) char* program_invocation_short_name =
    g_swift_ohos_process_name;

__attribute__((weak, visibility("hidden"))) int posix_spawn_file_actions_addchdir_np(
    posix_spawn_file_actions_t*, const char*) {
  return ENOTSUP;
}
}

namespace {

struct MainQueueWatcher {
  uv_poll_t poll{};
  int descriptor = -1;
  uint64_t generation = 0;
  uint64_t cleanup_token = 0;
  bool closing = false;
};

struct OhosTimerEntry {
  uv_timer_t timer{};
  void (*callback)(void*) = nullptr;
  void* user_data = nullptr;
  void (*release_user_data)(void*) = nullptr;
  uint64_t cleanup_token = 0;
  bool closing = false;
};

MainQueueWatcher* g_watcher = nullptr;

void CloseWatcherOnUiThread(MainQueueWatcher* watcher) {
  if (!watcher || watcher->closing) return;
  watcher->closing = true;
  if (g_watcher == watcher) {
    g_watcher = nullptr;
  }
  uv_poll_stop(&watcher->poll);
  uv_close(reinterpret_cast<uv_handle_t*>(&watcher->poll), [](uv_handle_t* handle) {
    delete static_cast<MainQueueWatcher*>(handle->data);
  });
}

void CloseTimerOnUiThread(OhosTimerEntry* entry) {
  if (!entry || entry->closing) return;
  entry->closing = true;
  if (entry->cleanup_token != 0) {
    nativeapi::ohos::RemoveEnvironmentCleanup(entry->cleanup_token);
    entry->cleanup_token = 0;
  }
  uv_timer_stop(&entry->timer);
  uv_close(reinterpret_cast<uv_handle_t*>(&entry->timer), [](uv_handle_t* handle) {
    auto* item = static_cast<OhosTimerEntry*>(handle->data);
    if (item->release_user_data) {
      item->release_user_data(item->user_data);
    }
    delete item;
  });
}

}  // namespace

namespace nativeapi::swift_ohos {

bool InstallMainQueue(napi_env env) {
  if (!env) return false;
  const uint64_t generation = nativeapi::ohos::EnvironmentGeneration();
  if (g_watcher && !g_watcher->closing && g_watcher->generation == generation && generation != 0) {
    return true;
  }

  uv_loop_t* loop = nullptr;
  const int descriptor = _dispatch_get_main_queue_port_4CF();
  if (descriptor < 0 || napi_get_uv_event_loop(env, &loop) != napi_ok || !loop) {
    return false;
  }

  auto* watcher = new MainQueueWatcher{};
  watcher->descriptor = descriptor;
  watcher->generation = generation;
  if (uv_poll_init(loop, &watcher->poll, descriptor) != 0) {
    delete watcher;
    return false;
  }
  watcher->poll.data = watcher;
  if (uv_poll_start(&watcher->poll, UV_READABLE, [](uv_poll_t* handle, int status, int events) {
        if (status != 0 || (events & UV_READABLE) == 0) return;
        const auto* current = static_cast<MainQueueWatcher*>(handle->data);
        std::uint64_t wakeup = 0;
        ssize_t count = 0;
        do {
          count = read(current->descriptor, &wakeup, sizeof(wakeup));
        } while (count < 0 && errno == EINTR);
        if (count == static_cast<ssize_t>(sizeof(wakeup))) {
          _dispatch_main_queue_callback_4CF(nullptr);
        }
      }) != 0) {
    CloseWatcherOnUiThread(watcher);
    return false;
  }

  g_watcher = watcher;
  if (generation != 0) {
    watcher->cleanup_token = nativeapi::ohos::AddEnvironmentCleanup(
        generation, [watcher] { CloseWatcherOnUiThread(watcher); });
  }
  return true;
}

bool Initialize(napi_env env) {
  if (!nativeapi::ohos::Initialize(env)) {
    return false;
  }
  return InstallMainQueue(env);
}

}  // namespace nativeapi::swift_ohos

extern "C" {

void native_swift_install_main_queue(void) {
  nativeapi::ohos::WithEnvironment(
      [](napi_env env) { nativeapi::swift_ohos::InstallMainQueue(env); });
}

uintptr_t native_swift_timer_start(uint32_t milliseconds,
                                   void (*callback)(void* user_data),
                                   void* user_data,
                                   void (*release_user_data)(void* user_data)) {
  if (!callback) {
    if (release_user_data) release_user_data(user_data);
    return 0;
  }

  uintptr_t token = 0;
  const bool started = nativeapi::ohos::WithEnvironment([&](napi_env env) {
    uv_loop_t* loop = nullptr;
    if (napi_get_uv_event_loop(env, &loop) != napi_ok || !loop) return;

    auto* entry = new OhosTimerEntry{};
    if (uv_timer_init(loop, &entry->timer) != 0) {
      delete entry;
      return;
    }
    entry->callback = callback;
    entry->user_data = user_data;
    entry->release_user_data = release_user_data;
    entry->timer.data = entry;

    const uint64_t interval = milliseconds > 0 ? milliseconds : 1;
    if (uv_timer_start(
            &entry->timer,
            [](uv_timer_t* handle) {
              auto* item = static_cast<OhosTimerEntry*>(handle->data);
              if (item && !item->closing && item->callback) {
                item->callback(item->user_data);
              }
            },
            interval, interval) != 0) {
      entry->release_user_data = nullptr;
      CloseTimerOnUiThread(entry);
      return;
    }

    const uint64_t generation = nativeapi::ohos::EnvironmentGeneration();
    if (generation != 0) {
      entry->cleanup_token = nativeapi::ohos::AddEnvironmentCleanup(
          generation, [entry] { CloseTimerOnUiThread(entry); });
    }
    token = reinterpret_cast<uintptr_t>(entry);
  });

  if (!started || token == 0) {
    if (release_user_data) release_user_data(user_data);
    return 0;
  }
  return token;
}

void native_swift_timer_stop(uintptr_t timer_id) {
  if (timer_id == 0) return;
  auto* entry = reinterpret_cast<OhosTimerEntry*>(timer_id);
  if (!nativeapi::ohos::Post([entry] { CloseTimerOnUiThread(entry); })) {
    CloseTimerOnUiThread(entry);
  }
}

}  // extern "C"
