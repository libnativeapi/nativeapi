// Test-only exports in the actual ctypes DSO: the shared confirmation producer
// and handle table are real. Only platform launching/pumping is substituted;
// the fixture creates no windows, sends no input and never terminates a
// process.
#include "../src/event_loop.h"
#include "application.h"
#include "foundation/dispatcher.h"
#include "foundation/handle_table.h"

#include <functional>
#include <mutex>
#include <thread>
#include <vector>

#if defined(__APPLE__)
#import <Cocoa/Cocoa.h>
@interface PythonQuitTestHost : NSObject <NSApplicationDelegate>
@end
@implementation PythonQuitTestHost
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender {
  return NSTerminateCancel;
}
@end
static PythonQuitTestHost* host;
#endif

namespace {
std::mutex queue_mutex;
std::vector<std::function<void()>> queue;
}  // namespace

extern "C" NATIVEAPI_PY_EXPORT void nativeapi_py_test_init(void) {
#if defined(__APPLE__)
  [NSApplication sharedApplication];
  if (!host)
    host = [[PythonQuitTestHost alloc] init];
  NSApp.delegate = host;
#endif
  const auto ui = std::this_thread::get_id();
  nativeapi::SetMainThreadDispatcher(
      [](auto callback) {
        std::lock_guard<std::mutex> lock(queue_mutex);
        queue.push_back(std::move(callback));
        return true;
      },
      [ui] { return ui == std::this_thread::get_id(); });
  (void)nativeapi::Application::GetInstance();
}

extern "C" NATIVEAPI_PY_EXPORT int nativeapi_py_test_pump(void) {
  for (;;) {
    std::vector<std::function<void()>> work;
    {
      std::lock_guard<std::mutex> lock(queue_mutex);
      work.swap(queue);
    }
    if (work.empty())
      return -1;
    for (auto& callback : work)
      callback();
  }
}

extern "C" NATIVEAPI_PY_EXPORT void nativeapi_py_test_reset(void) {
  nativeapi_py_test_pump();
  nativeapi::SetMainThreadDispatcher(nullptr, nullptr);
}

extern "C" NATIVEAPI_PY_EXPORT uint64_t nativeapi_py_test_live_handles(void) {
  return nativeapi::HandleTable::GetInstance().LiveCount();
}
