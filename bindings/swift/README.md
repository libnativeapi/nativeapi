# nativeapi for Swift

Native desktop APIs for Swift on macOS, backed by the libnativeapi C ABI.
The generated API covers windows, views, menus, tray icons, displays, keyboard,
dialogs, preferences, storage and the other APIs currently exposed by the C ABI.

This is a source-checkout binding: the Swift package compiles the workspace's
`core/` submodule directly through SwiftPM (no CMake step and no separate
native library at runtime). It is not published as a standalone Swift package
yet.

## Requirements

- macOS 11 or later (the binding is built and tested on macOS; the core also
  supports Windows and Linux, where SwiftPM would need each platform's setup)
- Swift 5.9 or later (Xcode 15+); the package builds in Swift 5 language mode
- The `core` submodule checked out:

```bash
git clone --recursive https://github.com/libnativeapi/nativeapi.git
# or, in an existing checkout:
git submodule update --init core
```

## Use from SwiftPM

Add the checkout as a path dependency. The package name is `NativeAPI`, which
also names the product:

```swift
dependencies: [
  .package(name: "NativeAPI", path: "../nativeapi/bindings/swift")
],
targets: [
  .executableTarget(
    name: "MyApp",
    dependencies: [.product(name: "NativeAPI", package: "NativeAPI")]
  )
]
```

```swift
import NativeAPI

let window = Window.create()
window?.setTitle("My Swift App")
window?.setSize(Size(width: 900, height: 600), animate: false)
window?.center()
window?.show()
```

Build and test the package itself from `bindings/swift`:

```bash
swift build
swift test
```

## API conventions

- **Constructors and factories** are static, and return `nil` when the native
  side fails: `Window.create()`, `Preferences.createWithScope("my-app")`,
  `DisplayManager.shared.getPrimary()`.
- **Singletons** are `.shared` instances: `Application.shared`,
  `DisplayManager.shared`, `Clipboard.shared`. They hold no owned handle.
- **Owned handles.** Every object returned by a constructor, getter, factory or
  list owns one reference. `dispose()` releases it immediately (call it on the
  UI thread). `deinit` otherwise hands the handle to
  `native_handle_finalize`, which returns to the UI thread before releasing,
  so dropping a reference from another thread is safe.
- **Derived views** subclass their base: `Label`, `Button`, `TextField` and
  `ImageView` are `View` subclasses, and `Label` can be passed to
  `View.addSubview`. Each subclass releases with its own C function.
- **Structs** are Swift value types with memberwise initializers. Optional
  fields default to `nil` and string lists to `[]`. `Color` also carries the
  named constants (`Color.black`) and helpers (`Color.fromHex`, `color.toRgba()`).
- **Enums** are `CInt`-backed Swift enums. Unknown values fall back to the
  first case rather than trapping. `ModifierKey` is an `OptionSet` because the
  core treats it as a bitmask: pass combinations like `[.meta, .shift]`.
- **Strings** are copied in both directions; the binding handles the native
  allocations.

```swift
guard let preferences = Preferences.createWithScope("my-app") else { exit(1) }
defer { preferences.dispose() }

if preferences.set("theme", value: "dark") {
  print(preferences.get("theme", defaultValue: "system") ?? "system")
}
```

## Events and callbacks

An event source exposes `addListener` and `removeListener`. The closure runs
synchronously on the UI thread and receives a typed enum whose payload is
already copied or retained, so it is safe to capture:

```swift
let listener = window.addListener { event in
  switch event {
  case .closeRequested(_, let request):
    // A close request can be vetoed or answered later.
    if request?.isCancelable == true {
      _ = request?.cancel()
    }
  case .moved(_, let position):
    print("moved to \(position.x), \(position.y)")
  default:
    break
  }
}

_ = window.removeListener(listener)
```

`addListener` returns a `ListenerId`; `0` means registration failed. The
closure context is released through the C ABI's `release_user_data` hook, so
removing the listener (or destroying its owner) reclaims it.

Callbacks that the core invokes once, such as `Clipboard.readText` or
`WindowManager.shared.setWillShowHook`, take ordinary escaping closures. Their
payload arguments are borrowed for the duration of the closure and copied into
Swift values by the binding. For payload-carrying callbacks the binding
completes the native delivery token after the closure returns.

## Threads

Create, use and release native objects on the UI thread. UI work belongs there;
use `DispatchQueue.main.async` to return to it from another queue. Console tools
can run the native loop with:

```swift
let exitCode = Application.shared.runWithWindow(window)
```

On macOS the OS owns the main thread; initialize the binding there.

## Examples

The examples are standalone SwiftPM packages that depend on
`../../bindings/swift`:

```bash
cd examples/swift_window_example
swift run SwiftWindowExample            # close the window to exit
swift run SwiftWindowExample --smoke    # self-closing check

cd examples/swift_display_example
swift run
```

| Example | nativeapi features |
| --- | --- |
| [Window](../../examples/swift_window_example) | Window creation, title, size, centering, event listeners, application loop. |
| [Display](../../examples/swift_display_example) | Display enumeration, geometry, work areas, scale and primary display. |

## Generation

From the workspace root:

```bash
./codegen bindings --lang swift
./codegen check --lang swift
```

The generated files live in `Sources/NativeAPI/` (mirroring `core/src/`, so
`foundation/geometry.h` becomes `Foundation/Geometry.swift`) and start with
`AUTO-GENERATED. DO NOT EDIT.`. `Sources/NativeAPI/Support.swift` is generated
as well. `Package.swift`, `Sources/CNativeAPI/include/nativeapi.h` (which
imports the core's umbrella header; the core itself is reached through the
`Sources/CNativeAPI/core` symlink), the tests and the examples are hand-written.

## Contributing

Development happens in [nativeapi](https://github.com/libnativeapi/nativeapi), which holds every binding and the code generator and checks out the core library as a submodule:

```bash
git clone --recursive https://github.com/libnativeapi/nativeapi.git
```

Files marked `AUTO-GENERATED. DO NOT EDIT.` are generated from the C++ headers in [nativeapi](https://github.com/libnativeapi/nativeapi-core). To change the API, send a pull request there; maintainers regenerate the bindings.

Report all bugs and feature requests in [nativeapi issues](https://github.com/libnativeapi/nativeapi/issues). Include the affected package or binding, its version, your operating system, and a minimal reproduction.
