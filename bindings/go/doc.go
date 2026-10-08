// Package nativeapi provides native desktop APIs through the libnativeapi C ABI.
//
// Build the native library with CMake before building a Go program. This source
// module currently requires a workspace checkout including the core submodule;
// see README.md for platform dependencies and build commands.
//
// Lock the main goroutine to its OS thread using runtime.LockOSThread in main
// (or init), then call Init before other native APIs. Create, use, and release
// native objects on that thread; Application.Run runs the native event loop there.
// Listeners and callbacks execute synchronously on the UI thread. Do not call UI APIs from
// other goroutines or block the event loop while waiting for one of them. Use
// Dispatch to queue UI work from other goroutines, or PollEvents in console tools.
//
// Constructors and factories return an owned reference and an error. Object
// getters return an owned reference or nil for absence. Fallible operations
// return errors wrapping ErrOperationFailed; predicates keep returning bool.
// Release each owned reference explicitly. Copies of a wrapper share release
// state. Derived views embed their base view, for example label.View can be
// passed to View.AddSubview. Handles in event payloads are borrowed and
// valid only during the callback. Strings and value payloads are copied to Go.
// AddListener returns a ListenerID and an error. Remove registered listeners when
// no longer needed. Callback resources are reclaimed when the native core calls
// its release hook, which may require pumping the event loop.
package nativeapi
