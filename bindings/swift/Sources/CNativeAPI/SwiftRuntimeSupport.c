#include "include/swift_runtime_support.h"

#include <stdlib.h>

#if defined(__APPLE__)

#include <CoreFoundation/CoreFoundation.h>

typedef struct NativeSwiftTimerBox {
  void (*callback)(void*);
  void* user_data;
  void (*release_user_data)(void*);
} NativeSwiftTimerBox;

static void NativeSwiftTimerRelease(const void* info) {
  NativeSwiftTimerBox* box = (NativeSwiftTimerBox*)info;
  if (!box) return;
  if (box->release_user_data) {
    box->release_user_data(box->user_data);
  }
  free(box);
}

static void NativeSwiftTimerCallout(CFRunLoopTimerRef timer, void* info) {
  (void)timer;
  NativeSwiftTimerBox* box = (NativeSwiftTimerBox*)info;
  if (box && box->callback) {
    box->callback(box->user_data);
  }
}

void native_swift_install_main_queue(void) {
  // AppKit's main CFRunLoop already services DispatchQueue.main.
}

uintptr_t native_swift_timer_start(uint32_t milliseconds,
                                   void (*callback)(void* user_data),
                                   void* user_data,
                                   void (*release_user_data)(void* user_data)) {
  if (!callback) {
    if (release_user_data) release_user_data(user_data);
    return 0;
  }
  NativeSwiftTimerBox* box = (NativeSwiftTimerBox*)malloc(sizeof(NativeSwiftTimerBox));
  if (!box) {
    if (release_user_data) release_user_data(user_data);
    return 0;
  }
  box->callback = callback;
  box->user_data = user_data;
  box->release_user_data = release_user_data;

  CFRunLoopTimerContext context = {0, box, NULL, NativeSwiftTimerRelease, NULL};
  const CFTimeInterval interval = (milliseconds > 0 ? (double)milliseconds : 1.0) / 1000.0;
  CFRunLoopTimerRef timer = CFRunLoopTimerCreate(
      kCFAllocatorDefault, CFAbsoluteTimeGetCurrent() + interval, interval, 0, 0,
      NativeSwiftTimerCallout, &context);
  if (!timer) {
    NativeSwiftTimerRelease(box);
    return 0;
  }

  CFRunLoopRef main_loop = CFRunLoopGetMain();
  CFRunLoopAddTimer(main_loop, timer, kCFRunLoopCommonModes);
  CFRunLoopAddTimer(main_loop, timer, CFSTR("NSEventTrackingRunLoopMode"));
  return (uintptr_t)timer;
}

void native_swift_timer_stop(uintptr_t timer_id) {
  if (timer_id == 0) return;
  CFRunLoopTimerRef timer = (CFRunLoopTimerRef)timer_id;
  CFRunLoopTimerInvalidate(timer);
  CFRelease(timer);
}

#elif defined(_WIN32)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <process.h>
#include <windows.h>

enum {
  kMsgDrainMainQueue = WM_APP + 1,
  kMsgStopTimer = WM_APP + 2,
  kFallbackPollTimerId = 1,
};

typedef void (*DispatchMainDrainFn)(void*);
typedef HANDLE (*DispatchMainHandleFn)(void);

typedef struct WinTimerEntry {
  UINT_PTR id;
  void (*callback)(void*);
  void* user_data;
  void (*release_user_data)(void*);
  int in_callback;
  int stopped;
  struct WinTimerEntry* next;
} WinTimerEntry;

static HWND g_pump_hwnd = NULL;
static DWORD g_pump_thread_id = 0;
static LONG g_main_queue_installed = 0;
static DispatchMainDrainFn g_drain_main_queue = NULL;
static HANDLE g_main_queue_handle = NULL;
static HANDLE g_drain_ack_event = NULL;
static UINT_PTR g_next_timer_id = 100;
static WinTimerEntry* g_timers = NULL;

static void FreeWinTimerEntry(WinTimerEntry* entry) {
  if (!entry) return;
  if (entry->release_user_data) {
    entry->release_user_data(entry->user_data);
  }
  free(entry);
}

static void StopWinTimerOnUiThread(UINT_PTR id) {
  WinTimerEntry** prev = &g_timers;
  while (*prev) {
    WinTimerEntry* current = *prev;
    if (current->id == id) {
      if (g_pump_hwnd) {
        KillTimer(g_pump_hwnd, id);
      }
      if (current->in_callback) {
        current->stopped = 1;
      } else {
        *prev = current->next;
        FreeWinTimerEntry(current);
      }
      return;
    }
    prev = &current->next;
  }
}

static LRESULT CALLBACK SwiftPumpWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
    case kMsgDrainMainQueue:
      if (g_drain_main_queue) {
        g_drain_main_queue(NULL);
      }
      if (wparam == 1 && g_drain_ack_event) {
        SetEvent(g_drain_ack_event);
      }
      return 0;

    case kMsgStopTimer:
      StopWinTimerOnUiThread((UINT_PTR)wparam);
      return 0;

    case WM_TIMER: {
      if (wparam == kFallbackPollTimerId) {
        if (g_drain_main_queue) {
          g_drain_main_queue(NULL);
        }
        return 0;
      }
      for (WinTimerEntry* entry = g_timers; entry != NULL; entry = entry->next) {
        if (entry->id == (UINT_PTR)wparam) {
          if (!entry->stopped && entry->callback) {
            entry->in_callback = 1;
            entry->callback(entry->user_data);
            entry->in_callback = 0;
          }
          if (entry->stopped) {
            StopWinTimerOnUiThread(entry->id);
          }
          return 0;
        }
      }
      return 0;
    }
  }
  return DefWindowProcW(hwnd, msg, wparam, lparam);
}

static HWND EnsurePumpWindowOnUiThread(void) {
  if (g_pump_hwnd && IsWindow(g_pump_hwnd)) {
    return g_pump_hwnd;
  }
  static ATOM class_atom = 0;
  HINSTANCE instance = GetModuleHandleW(NULL);
  if (!class_atom) {
    WNDCLASSEXW wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = SwiftPumpWndProc;
    wc.hInstance = instance;
    wc.lpszClassName = L"NativeAPISwiftRuntimePump";
    class_atom = RegisterClassExW(&wc);
  }
  g_pump_thread_id = GetCurrentThreadId();
  g_pump_hwnd = CreateWindowExW(0, L"NativeAPISwiftRuntimePump", L"", 0, 0, 0, 0, 0,
                                HWND_MESSAGE, NULL, instance, NULL);
  return g_pump_hwnd;
}

static unsigned __stdcall MainQueueWaitThread(void* unused) {
  (void)unused;
  for (;;) {
    if (WaitForSingleObject(g_main_queue_handle, INFINITE) != WAIT_OBJECT_0) {
      break;
    }
    HWND hwnd = g_pump_hwnd;
    if (!hwnd || !PostMessageW(hwnd, kMsgDrainMainQueue, 1, 0)) {
      break;
    }
    if (WaitForSingleObject(g_drain_ack_event, INFINITE) != WAIT_OBJECT_0) {
      break;
    }
  }
  return 0;
}

void native_swift_install_main_queue(void) {
  HWND hwnd = EnsurePumpWindowOnUiThread();
  if (!hwnd) return;
  if (InterlockedCompareExchange(&g_main_queue_installed, 1, 0) != 0) {
    PostMessageW(hwnd, kMsgDrainMainQueue, 0, 0);
    return;
  }

  HMODULE dispatch_mod = GetModuleHandleW(L"dispatch.dll");
  if (!dispatch_mod) {
    dispatch_mod = LoadLibraryW(L"dispatch.dll");
  }
  if (!dispatch_mod) return;

  g_drain_main_queue =
      (DispatchMainDrainFn)GetProcAddress(dispatch_mod, "_dispatch_main_queue_callback_4CF");
  if (!g_drain_main_queue) return;

  // Drain any blocks queued before the UI loop started as soon as pumping begins.
  PostMessageW(hwnd, kMsgDrainMainQueue, 0, 0);

  DispatchMainHandleFn get_handle =
      (DispatchMainHandleFn)GetProcAddress(dispatch_mod, "_dispatch_get_main_queue_handle_4CF");
  HANDLE handle = get_handle ? get_handle() : NULL;
  if (handle && handle != INVALID_HANDLE_VALUE) {
    g_main_queue_handle = handle;
    g_drain_ack_event = CreateEventW(NULL, FALSE, FALSE, NULL);
    if (g_drain_ack_event) {
      uintptr_t thread = _beginthreadex(NULL, 0, MainQueueWaitThread, NULL, 0, NULL);
      if (thread != 0) {
        CloseHandle((HANDLE)thread);
        return;
      }
      CloseHandle(g_drain_ack_event);
      g_drain_ack_event = NULL;
    }
  }

  // Fallback only if libdispatch did not expose a waitable main-queue handle.
  SetTimer(hwnd, kFallbackPollTimerId, 16, NULL);
}

uintptr_t native_swift_timer_start(uint32_t milliseconds,
                                   void (*callback)(void* user_data),
                                   void* user_data,
                                   void (*release_user_data)(void* user_data)) {
  if (!callback) {
    if (release_user_data) release_user_data(user_data);
    return 0;
  }
  HWND hwnd = EnsurePumpWindowOnUiThread();
  if (!hwnd) {
    if (release_user_data) release_user_data(user_data);
    return 0;
  }
  WinTimerEntry* entry = (WinTimerEntry*)calloc(1, sizeof(WinTimerEntry));
  if (!entry) {
    if (release_user_data) release_user_data(user_data);
    return 0;
  }
  UINT_PTR id = ++g_next_timer_id;
  if (id <= kFallbackPollTimerId) {
    id = ++g_next_timer_id;
  }
  const UINT elapse = milliseconds > 0 ? (UINT)milliseconds : 1u;
  if (SetTimer(hwnd, id, elapse, NULL) == 0) {
    free(entry);
    if (release_user_data) release_user_data(user_data);
    return 0;
  }
  entry->id = id;
  entry->callback = callback;
  entry->user_data = user_data;
  entry->release_user_data = release_user_data;
  entry->next = g_timers;
  g_timers = entry;
  return (uintptr_t)id;
}

void native_swift_timer_stop(uintptr_t timer_id) {
  if (timer_id == 0 || !g_pump_hwnd) return;
  if (GetCurrentThreadId() == g_pump_thread_id) {
    StopWinTimerOnUiThread((UINT_PTR)timer_id);
  } else {
    PostMessageW(g_pump_hwnd, kMsgStopTimer, (WPARAM)timer_id, 0);
  }
}

#elif defined(__linux__) && !defined(__OHOS__)

#include <dlfcn.h>
#include <errno.h>
#include <glib.h>
#include <pthread.h>
#include <unistd.h>

typedef int (*DispatchMainPortFn)(void);
typedef void (*DispatchMainDrainFn)(void*);

typedef struct GlibTimerBox {
  void (*callback)(void*);
  void* user_data;
  void (*release_user_data)(void*);
} GlibTimerBox;

static pthread_once_t g_linux_main_queue_once = PTHREAD_ONCE_INIT;
static DispatchMainDrainFn g_linux_drain_main_queue = NULL;

static gboolean OnLinuxMainQueueReady(GIOChannel* channel, GIOCondition condition, gpointer data) {
  (void)condition;
  (void)data;
  int fd = g_io_channel_unix_get_fd(channel);
  uint64_t wakeup = 0;
  ssize_t count = 0;
  do {
    count = read(fd, &wakeup, sizeof(wakeup));
  } while (count < 0 && errno == EINTR);
  if (count == (ssize_t)sizeof(wakeup) && g_linux_drain_main_queue) {
    g_linux_drain_main_queue(NULL);
  }
  return G_SOURCE_CONTINUE;
}

static void InstallLinuxMainQueueOnce(void) {
  DispatchMainPortFn get_port =
      (DispatchMainPortFn)dlsym(RTLD_DEFAULT, "_dispatch_get_main_queue_port_4CF");
  g_linux_drain_main_queue =
      (DispatchMainDrainFn)dlsym(RTLD_DEFAULT, "_dispatch_main_queue_callback_4CF");
  if (!get_port || !g_linux_drain_main_queue) return;

  int fd = get_port();
  if (fd < 0) return;

  GIOChannel* channel = g_io_channel_unix_new(fd);
  if (!channel) return;
  g_io_add_watch(channel, G_IO_IN, OnLinuxMainQueueReady, NULL);
  g_io_channel_unref(channel);
}

void native_swift_install_main_queue(void) {
  pthread_once(&g_linux_main_queue_once, InstallLinuxMainQueueOnce);
}

static gboolean OnGlibTimerTick(gpointer data) {
  GlibTimerBox* box = (GlibTimerBox*)data;
  if (box && box->callback) {
    box->callback(box->user_data);
  }
  return G_SOURCE_CONTINUE;
}

static void FreeGlibTimerBox(gpointer data) {
  GlibTimerBox* box = (GlibTimerBox*)data;
  if (!box) return;
  if (box->release_user_data) {
    box->release_user_data(box->user_data);
  }
  g_free(box);
}

uintptr_t native_swift_timer_start(uint32_t milliseconds,
                                   void (*callback)(void* user_data),
                                   void* user_data,
                                   void (*release_user_data)(void* user_data)) {
  if (!callback) {
    if (release_user_data) release_user_data(user_data);
    return 0;
  }
  GlibTimerBox* box = g_new0(GlibTimerBox, 1);
  box->callback = callback;
  box->user_data = user_data;
  box->release_user_data = release_user_data;
  const guint interval = milliseconds > 0 ? (guint)milliseconds : 1u;
  guint id = g_timeout_add_full(G_PRIORITY_DEFAULT, interval, OnGlibTimerTick, box, FreeGlibTimerBox);
  return (uintptr_t)id;
}

void native_swift_timer_stop(uintptr_t timer_id) {
  if (timer_id == 0) return;
  g_source_remove((guint)timer_id);
}

#else

void native_swift_install_main_queue(void) {}

uintptr_t native_swift_timer_start(uint32_t milliseconds,
                                   void (*callback)(void* user_data),
                                   void* user_data,
                                   void (*release_user_data)(void* user_data)) {
  (void)milliseconds;
  (void)callback;
  if (release_user_data) {
    release_user_data(user_data);
  }
  return 0;
}

void native_swift_timer_stop(uintptr_t timer_id) {
  (void)timer_id;
}

#endif
