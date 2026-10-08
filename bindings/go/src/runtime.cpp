#include "../runtime.h"
#include "../../../core/src/foundation/dispatcher.h"

void nativeapi_go_init() {
  nativeapi::SetMainThread();
}

bool nativeapi_go_poll_events(int timeout_ms) {
  return nativeapi::RunMainThreadLoopFor(timeout_ms);
}

bool nativeapi_go_dispatch(void (*callback)(void*), void* data, void (*release)(void*)) {
  try {
    return nativeapi::RunOnMainThread([callback, data, release] {
      callback(data);
      release(data);
    });
  } catch (...) {
    return false;
  }
}
