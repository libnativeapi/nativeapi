#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Connects libdispatch's main queue (DispatchQueue.main / @MainActor) to the
// calling thread's native UI event loop on platforms where CoreFoundation does
// not already drive the loop (Windows and Linux). Safe to call repeatedly.
void native_swift_install_main_queue(void);

// Starts a repeating timer on the UI thread's native message loop so it keeps
// firing during modal tracking loops (such as native popup menus). Returns a
// non-zero identifier on success, or 0 on failure.
uintptr_t native_swift_timer_start(uint32_t milliseconds,
                                   void (*callback)(void* user_data),
                                   void* user_data,
                                   void (*release_user_data)(void* user_data));

// Stops a timer returned by native_swift_timer_start and schedules user_data
// release. Safe to call from inside the timer callback or from any thread.
void native_swift_timer_stop(uintptr_t timer_id);

#ifdef __cplusplus
}
#endif
