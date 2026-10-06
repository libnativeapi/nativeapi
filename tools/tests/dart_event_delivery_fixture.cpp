// Linked against the SAME cnativeapi DSO used by Dart, never a second core copy.
// Synthetic events and a private scheduler; no windows or OS input.
#include <functional>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

#include "display_manager.h"
#include "foundation/dispatcher.h"
#include "foundation/handle_table.h"
#include "shortcut_manager.h"
#include "application_quit_dispatch.h"

#if defined(__APPLE__)
#import <Cocoa/Cocoa.h>
@interface DartQuitTestHost : NSObject <NSApplicationDelegate>
@end
@implementation DartQuitTestHost
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender { return NSTerminateCancel; }
@end
static DartQuitTestHost* quit_host;
#endif

namespace {
std::mutex queue_mutex;
std::vector<std::function<void()>> queue;
std::atomic<int> quit_result{-1};
}
extern "C" void nativeapi_test_delivery_scheduler() {
  nativeapi::SetMainThreadDispatcher([](std::function<void()> fn) {
    std::lock_guard<std::mutex> lock(queue_mutex);
    queue.push_back(std::move(fn));
    return true;
  }, [] { return true; });
}
extern "C" void nativeapi_test_delivery_drain() {
  std::vector<std::function<void()>> work;
  { std::lock_guard<std::mutex> lock(queue_mutex); work.swap(queue); }
  for (auto& fn : work) fn();
}
extern "C" void nativeapi_test_delivery_restore() {
  nativeapi::SetMainThreadDispatcher(nullptr, nullptr);
}
extern "C" void nativeapi_test_emit_shortcut() {
  std::thread producer([] {
    nativeapi::ShortcutManager::GetInstance().Emit(nativeapi::ShortcutRegistrationFailedEvent(
        991, "Ctrl+ForeignThread", "owned UTF-8 payload: 异步确认"));
  });
  producer.join();
}
extern "C" void nativeapi_test_emit_display() {
  std::thread producer([] {
    nativeapi::DisplayManager::GetInstance().Emit(
        nativeapi::DisplayAddedEvent(std::make_shared<nativeapi::Display>(nullptr)));
  });
  producer.join();
}
extern "C" uint64_t nativeapi_test_delivery_live_handles() {
  return nativeapi::HandleTable::GetInstance().LiveCount();
}
extern "C" void nativeapi_test_request_quit(int exit_code) {
#if defined(__APPLE__)
  // Initialize headless only on the actual UI thread. If the CLI isolate is a
  // worker, Application's initializer refuses AppKit and these tests exercise
  // only the common request producer under their private scheduler.
  if ([NSThread isMainThread] && !quit_host) {
    [NSApplication sharedApplication];
    quit_host = [[DartQuitTestHost alloc] init];
    NSApp.delegate = quit_host;
  }
#endif
  quit_result = -1;
  nativeapi::detail::ApplicationQuitDispatch::RequestForLoop(exit_code, [](int code) { quit_result = code; });
}
extern "C" int nativeapi_test_quit_result() { return quit_result; }
