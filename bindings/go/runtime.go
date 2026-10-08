package nativeapi

/*
#cgo darwin LDFLAGS: -L${SRCDIR}/build -lnativeapi_go -Wl,-rpath,${SRCDIR}/build
#cgo linux LDFLAGS: -L${SRCDIR}/build -lnativeapi_go -Wl,-rpath,${SRCDIR}/build
#cgo windows LDFLAGS: -L${SRCDIR}/build -lnativeapi_go
#include "bridge.gen.h"
#include "runtime.h"
extern void goDispatchCallback(uintptr_t handle);
static inline void go_dispatch_callback(void* data) { goDispatchCallback(*(uintptr_t*)data); }
static inline nativeapi_go_callback_t go_get_dispatch_callback(void) { return go_dispatch_callback; }
*/
import "C"

import (
	"errors"
	"fmt"
	"runtime/cgo"
	"time"
	"unsafe"
)

// ErrOperationFailed means the native operation reported failure. The C ABI
// does not provide a more specific cause for boolean or invalid-handle failures.
var ErrOperationFailed = errors.New("native operation failed")

// ErrInvalidArgument means an argument cannot be used by the binding.
var ErrInvalidArgument = errors.New("invalid argument")

func nativeError(op string) error {
	return fmt.Errorf("nativeapi: %s: %w", op, ErrOperationFailed)
}

func invalidArgument(op string) error {
	return fmt.Errorf("nativeapi: %s: %w", op, ErrInvalidArgument)
}

// ListenerID identifies a registered listener. Zero indicates failure.
type ListenerID uint64

// nativeHandle is shared by a wrapper and its embedded base wrappers.
// All access must occur on the native UI thread.
type nativeHandle struct {
	value    uint64
	borrowed bool
}

func (h *nativeHandle) nativeHandleValue() uint64 {
	if h == nil {
		return 0
	}
	return h.value
}

func newCallback(callback any) unsafe.Pointer {
	// A retained C allocation holds the integer, never a Go pointer.
	data := C.malloc(C.size_t(unsafe.Sizeof(C.uintptr_t(0))))
	if data == nil {
		panic("nativeapi: unable to allocate callback context")
	}
	*(*C.uintptr_t)(data) = C.uintptr_t(cgo.NewHandle(callback))
	return data
}

//export goReleaseCallback
func goReleaseCallback(handle C.uintptr_t) {
	cgo.Handle(handle).Delete()
}

func optionalStringFromC(value *C.char) *string {
	if value == nil {
		return nil
	}
	result := C.GoString(value)
	return &result
}

func stringsFromC(raw C.native_string_list_t) []string {
	result := make([]string, int(raw.count))
	for i, value := range unsafe.Slice(raw.items, int(raw.count)) {
		result[i] = C.GoString(value)
	}
	return result
}

func stringsToC(values []string) (C.native_string_list_t, func()) {
	var raw C.native_string_list_t
	if len(values) == 0 {
		return raw, func() {}
	}
	raw.count = C.long(len(values))
	raw.items = (**C.char)(C.calloc(C.size_t(len(values)), C.size_t(unsafe.Sizeof(uintptr(0)))))
	if raw.items == nil {
		panic("nativeapi: unable to allocate string list")
	}
	for i, value := range values {
		unsafe.Slice(raw.items, len(values))[i] = C.CString(value)
	}
	// Input memory comes from libc; output memory uses the core's own frees.
	// Keeping the allocators paired matters on Windows.
	return raw, func() {
		for _, value := range unsafe.Slice(raw.items, len(values)) {
			C.free(unsafe.Pointer(value))
		}
		C.free(unsafe.Pointer(raw.items))
	}
}

func mapFromC(raw C.native_string_map_t) map[string]string {
	result := make(map[string]string, int(raw.count))
	keys := unsafe.Slice(raw.keys, int(raw.count))
	values := unsafe.Slice(raw.values, int(raw.count))
	for i, key := range keys {
		result[C.GoString(key)] = C.GoString(values[i])
	}
	return result
}

// Init declares the calling OS thread to be the native UI thread. Call once
// from the main goroutine after runtime.LockOSThread, before other native APIs.
// On macOS the OS determines the main thread; Init cannot change it.
func Init() { C.nativeapi_go_init() }

// PollEvents services native events for up to timeout. A zero timeout drains
// pending work. Call only on the UI thread when ApplicationRun is not running.
// Console tools can use it to deliver listeners and reclaim callback contexts.
// Negative timeouts and durations above the native limit wrap ErrInvalidArgument.
func PollEvents(timeout time.Duration) error {
	if timeout < 0 {
		return invalidArgument("PollEvents")
	}
	millis := timeout / time.Millisecond
	if timeout%time.Millisecond != 0 {
		millis++
	}
	if millis > 1<<31-1 {
		return invalidArgument("PollEvents")
	}
	if !bool(C.nativeapi_go_poll_events(C.int(millis))) {
		return nativeError("PollEvents")
	}
	return nil
}

// Dispatch queues callback on the UI thread. It is safe from any goroutine.
// A running ApplicationRun or calls to PollEvents must service the native queue.
// Failure wraps ErrOperationFailed. A nil callback is ignored.
func Dispatch(callback func()) error {
	if callback == nil {
		return nil
	}
	data := newCallback(callback)
	if !bool(C.nativeapi_go_dispatch(C.nativeapi_go_callback_t(C.go_get_dispatch_callback()), data,
		C.native_release_user_data_t(C.go_get_release_callback()))) {
		C.go_release_callback(data)
		return nativeError("Dispatch")
	}
	return nil
}

//export goDispatchCallback
func goDispatchCallback(handle C.uintptr_t) {
	cgo.Handle(handle).Value().(func())()
}
