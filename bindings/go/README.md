# nativeapi for Go

Native desktop APIs for Go, backed by the libnativeapi C ABI through cgo.
The generated API covers windows, views, menus, tray icons, displays, keyboard,
dialogs, preferences and the other APIs currently exposed by the C ABI.

This is a source-checkout binding for macOS, Windows and Linux. It is not yet
published as a standalone Go module: the headers and CMake build require the
workspace's `core/` submodule. No third-party Go dependencies are required.

## Build

Requires Go 1.22+, cgo enabled, CMake 3.24+, and a C/C++ compiler. macOS needs
Xcode command-line tools. Linux also needs GTK 3, X11 and XI development packages:

```bash
sudo apt-get install cmake pkg-config build-essential libgtk-3-dev libx11-dev libxi-dev
```

From the workspace root:

```bash
git submodule update --init core
cmake -S bindings/go -B bindings/go/build -DCMAKE_BUILD_TYPE=Release
cmake --build bindings/go/build --config Release --parallel
cd bindings/go
go test ./...
```

On Windows, build the native library with Visual Studio's CMake generator and
use a MinGW-w64 C compiler for cgo (`CC=gcc`). Add `bindings\go\build` to `PATH`
when running Go tests or examples so Windows can find `nativeapi_go.dll`.
The Go-side C allocations are freed by the same C runtime that allocated them;
native outputs are freed by the core. macOS and Linux embed the checkout's
`bindings/go/build` location as an rpath. To distribute an application, ship the
native shared library and configure its loader path for your installation.

Linux tests and window examples require a desktop session; in headless CI use
`xvfb-run -a go test ./...` (install `xvfb`).

## Usage

A local consumer uses this module path with a replacement pointing to the checkout:

```go
require github.com/libnativeapi/nativeapi/bindings/go v0.0.0
replace github.com/libnativeapi/nativeapi/bindings/go => /path/to/nativeapi/bindings/go
```

```go
package main

import (
	"fmt"
	. "github.com/libnativeapi/nativeapi/bindings/go"
	"runtime"
)

func main() {
	runtime.LockOSThread()
	Init()
	prefs, err := NewPreferencesWithScope("my-app")
	if err != nil {
		panic(err)
	}
	defer prefs.Release()
	if err := prefs.Set("theme", "dark"); err != nil {
		panic(err)
	}
	fmt.Println(prefs.Get("theme", "system"))
	if err := PollEvents(0); err != nil {
		panic(err)
	}
}
```

The examples use a dot import (`. "github.com/libnativeapi/nativeapi/bindings/go"`)
to call exported APIs without a package prefix.

The generated API follows Go's [naming conventions](https://go.dev/doc/effective_go#names):

- Getters omit `Get`: `window.Title()`, `prefs.Keys()`, `DisplayManager.All()`.
  Lookup operations named `Get` keep that name, for example `prefs.Get(key, fallback)`.
- `KeyboardAccelerator.String()` implements `fmt.Stringer`. Initialisms use
  `ID`, `URL`, `OS` and `UI`, and parameters use lowerCamelCase.
- Constructors return `(*T, error)`. Overloads use concise names: `NewShortcut(id, options)`, `NewShortcutWithCallback(id, accelerator, callback)`,
  `NewPreferencesWithScope(scope)` and `NewWindowFromNative(pointer)`.
- Native singletons are exported values with methods, such as `DisplayManager.All()`
  and `Application.Run()`. Their implementation types are private, and the values
  hold no owned handles. Static methods on regular types remain package functions.
  Overloaded lookups name the key, such as `ShortcutManager.GetByID(id)` and
  `ShortcutManager.GetByAccelerator(accelerator)`.
- Fallible boolean operations return `error`: `prefs.Set(key, value)`,
  `window.Close()` and `menu.RemoveItem(item)`. Predicates including `Contains`,
  `IsVisible`, `HasShadow` and `CanOpen` keep returning `bool`.
- Object factories and registration return `(*T, error)`. Object getters keep
  returning `*T`, with `nil` for absence. The C ABI cannot distinguish absence
  from some getter failures; the binding does not invent an error cause.

Errors from native failures include the operation name and wrap
`ErrOperationFailed`; `errors.Is(err, ErrOperationFailed)` recognizes them.
`ErrInvalidArgument` covers binding-level validation, such as a nil listener
callback or a negative event-poll timeout. Native boolean/handle failures do not
carry a detailed cause in the C ABI, so the binding reports only the failure.
Structured results, such as `URLOpenResult`, retain the native error code and
message when the API provides them.

Enums have typed constants such as `TitleBarStyleNormal`. Default parameters
remain explicit Go arguments. Optional strings use `*string` (`nil` means absent).
Inherited views embed `*View`; pass `label.View` to `root.AddSubview`.
`Release()` drops an owned reference, while `Window.Close()` closes the window.

## Threads, events and ownership

Call `runtime.LockOSThread` from `main` or `init`, then `Init` on the
main goroutine before other native APIs. Create, use and release native objects
on that thread. `Application.Run()` / `Application.RunWithWindow(window)` runs the native
loop. Use `Dispatch(func() { ... })` to schedule UI work from other goroutines
and check its returned error. Console tools can call `PollEvents(timeout)` on the main
thread instead, where timeout is a `time.Duration` (for example `10*time.Millisecond`).
Do not call `PollEvents` inside an already running native event loop.

`AddListener` receives a Go function and returns `(ListenerID, error)`.
`RemoveListener` returns an error for an unknown listener ID.
Callbacks run synchronously on the UI thread and should return quickly without
panicking. Events have a `Type` discriminator, common fields and the fields for
each variant; only the active variant's fields are populated. Remove listeners
when done. Callback contexts are kept alive with `runtime/cgo.Handle` stored in
C memory (see [runtime/cgo.Handle](https://pkg.go.dev/runtime/cgo#Handle)) and
released through the core's release hook, including failure and replacement paths. Reclamation may be deferred until the event loop is serviced.

Call `Release` for every successful reference returned by a constructor, getter,
factory or object list. Getters return `nil` for invalid handles. List arrays are
freed by the binding while ownership of their handles transfers to Go. Copies of a wrapper
share release state; releasing one invalidates those copies. `Release` is safe to
repeat and accepts nil receivers. There are no automatic object finalizers.
Strings, lists, maps and value structs are copied into Go and native outputs are
freed automatically. Event handle fields are borrowed until the callback returns;
use `EventRequest.Defer` inside the callback to obtain an owned `EventDecision`
when an asynchronous decision is needed.

## Examples and generation

After building the shared library, run either example from its own directory:

```bash
cd examples/go_display_example
go run .
# Or: cd examples/go_window_example && go run .
# The window example also accepts --smoke for a self-closing runtime check.
```

From the workspace root:

```bash
./codegen bindings --lang go
./codegen check --lang go
```

The Go SDK (`gofmt`) is required when generating or checking Go output.
`nativeapi.gen.go` and `bridge.gen.h` are generated from the same IR as every
other binding; edit the generator or C++ headers, then regenerate.
`runtime.go`, `src/runtime.cpp` and the build configuration are hand-written.

## Contributing

Development happens in [nativeapi](https://github.com/libnativeapi/nativeapi), which holds every binding and the code generator and checks out the core library as a submodule:

```bash
git clone --recursive https://github.com/libnativeapi/nativeapi.git
```

Files marked `AUTO-GENERATED. DO NOT EDIT.` are generated from the C++ headers in [nativeapi](https://github.com/libnativeapi/nativeapi-core). To change the API, send a pull request there; maintainers regenerate the bindings.

- API requests and native behavior bugs → [nativeapi-core issues](https://github.com/libnativeapi/nativeapi-core/issues)
- Bugs specific to one binding → [nativeapi issues](https://github.com/libnativeapi/nativeapi/issues)
- Not sure → [nativeapi-core issues](https://github.com/libnativeapi/nativeapi-core/issues)
