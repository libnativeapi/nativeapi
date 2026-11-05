# MyGo tray icon example

A [MyGo](https://mygo.egoist.dev/) native UI window with a nativeapi tray icon
and native context menu. The tray and the window share one counter: increment
it from either interface, pause it with a checkbox menu item, hide the window,
then restore it by clicking the tray icon. Closing the window or choosing Quit
exits the application and releases the tray, menu, items and listeners.

## Run

Requires Go 1.27.3+, cgo, CMake 3.24+ and a C/C++ compiler. MyGo is pinned to
v0.7.4. Build nativeapi from the workspace root first:

```bash
git submodule update --init core
cmake -S bindings/go -B bindings/go/build -DCMAKE_BUILD_TYPE=Release
cmake --build bindings/go/build --config Release --parallel
cd examples/go_mygo_tray_icon_example
go run .
```

See the [Go binding setup](../../bindings/go/README.md#build) for Linux packages
and Windows cgo / DLL lookup. A desktop session is required. On Linux the desktop
also needs a StatusNotifierItem host, such as KDE's tray or GNOME's AppIndicator
extension; an Xvfb display alone does not provide a tray.

The nativeapi brand icon is embedded in the binary, so the working directory
does not affect icon loading. `icon.svg` uses the same paths as the
[website's brand mark](../../website/src/components/brand-icon.tsx); `icon.png`
is its 72 × 72 rasterization. macOS uses it as a template image to follow the
menu bar appearance; other platforms keep its blue gradient. Regenerate it with
`rsvg-convert -w 72 -h 72 icon.svg -o icon.png` after editing the SVG.
macOS can show the counter beside the icon; tray titles vary by platform and
desktop.

## Integration

MyGo creates and draws the window using `ui.View`. nativeapi creates the icon
(`NewTrayIconWithIdentifier`, `ImageFromBase64`) and menu (`NewMenu`,
`NewMenuItem`), and handles tray/menu events. Left-click restores the MyGo
window; right-click opens the context menu. Linux tray hosts decide which mouse
actions they expose and how they display the exported menu.

MyGo owns the event loop. `Init()` runs in `App.WhenReady` on the main thread;
nativeapi uses the same AppKit, Win32 or GTK 3 loop. Native callbacks update
the state and call `Window.Invalidate`. Goroutines use `mygo.RunOnMain` to
return to this thread. Do not start nativeapi's `Application.Run` or poll events
inside MyGo's loop.

Unlike a MyGo-only app, this integration needs cgo and the nativeapi shared
library at runtime. The [Go binding](../../bindings/go/README.md#build) explains
the loader paths to configure when distributing an app.

## Smoke check

```bash
go run . --smoke
```

Opens the real window and tray, checks visibility and the stable tray identifier,
changes the tray title/tooltip and native menu state, hides/restores the window
and icon, and quits automatically. It does not simulate mouse input or test the
desktop's tray-menu interaction.
