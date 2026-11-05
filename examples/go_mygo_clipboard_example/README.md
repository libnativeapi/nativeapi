# MyGo clipboard example

A [MyGo](https://mygo.egoist.dev/) native UI using nativeapi's clipboard:

- Edit text and copy it as plain text or as text plus HTML in one write.
- Read asynchronously and inspect text, HTML, image presence and file count.
- Observe clipboard change events when supported by the platform.

The example reads only when Read clipboard is clicked. Change events increment
a counter without reading the clipboard. The asynchronous read releases its
returned image handle; the change listener is removed at shutdown.

## Run

Requires Go 1.27.3+, cgo, CMake 3.24+ and a C/C++ compiler. MyGo is pinned to
v0.7.4. From the workspace root:

```bash
git submodule update --init core
cmake -S bindings/go -B bindings/go/build -DCMAKE_BUILD_TYPE=Release
cmake --build bindings/go/build --config Release --parallel
cd examples/go_mygo_clipboard_example
go run .
```

See the [Go binding setup](../../bindings/go/README.md#build) for Linux packages,
Windows cgo / DLL lookup, and distributing the native shared library. A desktop
session is required. Available clipboard formats and monitoring support depend
on the operating system and desktop session.

## Integration

MyGo creates and draws the window using `ui.View`. The explicit copy/read
buttons use nativeapi's `Clipboard.WriteText`, `Clipboard.Write` and
`Clipboard.Read`; change notifications use `Clipboard.AddListener`.

MyGo owns the AppKit / Win32 / GTK 3 event loop, which also services nativeapi's
asynchronous callbacks. `Init()` runs in `App.WhenReady`, and native calls and
callbacks run on that main thread. UI updates request `Window.Invalidate`.
Do not block waiting for clipboard callbacks or start a second nativeapi loop.

## Smoke check

```bash
go run . --smoke
```

Opens the real window, checks clipboard support and an asynchronous read,
then quits automatically. The check does not write to the clipboard or log
its contents. A missing callback fails the check after eight seconds.
