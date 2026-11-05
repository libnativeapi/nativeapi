# MyGo display example

A [MyGo](https://mygo.egoist.dev/) native UI displaying nativeapi's connected
displays: identity, name, position, size, work area, scale and primary status.
Refresh reads the displays and cursor position again; native display events
refresh the list automatically where the platform implements them.

## Run

Requires Go 1.27.3+, cgo, CMake 3.24+ and a C/C++ compiler. MyGo is pinned to
v0.7.4. From the workspace root:

```bash
git submodule update --init core
cmake -S bindings/go -B bindings/go/build -DCMAKE_BUILD_TYPE=Release
cmake --build bindings/go/build --config Release --parallel
cd examples/go_mygo_display_example
go run .
```

See the [Go binding setup](../../bindings/go/README.md#build) for Linux packages,
Windows cgo / DLL lookup, and distributing the native shared library. A desktop
session is required. Linux's nativeapi display change notifications are not yet
implemented; use Refresh after changing the display configuration. Cursor
coordinates on Wayland depend on what GTK / the compositor exposes.

## Integration

MyGo draws the interface through `ui.View`. All display data comes from
nativeapi's `DisplayManager.All()` and `DisplayManager.CursorPosition()`.
Refresh copies each display's values into Go state and releases the returned
handle immediately. The display listener's event handle is borrowed and is not
retained by the interface. Its listener is removed at shutdown.

MyGo owns the AppKit / Win32 / GTK 3 event loop, which also services nativeapi.
`Init()` and native calls run on the main thread in `App.WhenReady`, view
callbacks or native listeners. Background work uses `mygo.RunOnMain`; there
is no second nativeapi application loop or `PollEvents` call.

## Smoke check

```bash
go run . --smoke
```

Opens the real window, enumerates displays, checks their size and scale, then
quits automatically. It does not change the display configuration.
