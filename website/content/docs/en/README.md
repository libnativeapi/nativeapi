# nativeapi

nativeapi gives Dart/Flutter, Rust, C#, JavaScript/TypeScript, Python and Go one API for the native parts of a desktop app — windows, tray icons, menus, displays, keyboard, dialogs, storage and more — implemented once in C++ for each platform.

> [!NOTE]
> nativeapi is under active development and the API may still change. The Dart and Rust packages are published; the other bindings are built from source for now.

## What is covered

| Area | Examples |
| --- | --- |
| Windows | create, move and resize, title bar styles, drag-to-move and drag-to-resize areas, visual effects, shaped windows, multiple windows |
| Tray icons & menus | menu bar and notification area icons, native menus with submenus, check items and accelerators |
| Displays & input | display bounds, work areas and scale factors, cursor position, global shortcuts, keyboard monitoring |
| System services | message and file dialogs, preferences, secure storage, clipboard, launch at login, opening URLs, accessibility |
| Application | lifecycle events, quit confirmation, the native event loop |

## Bindings

| Binding | Packages | Status |
| --- | --- | --- |
| [Dart / Flutter](../../../../bindings/dart/README.md) | `nativeapi`, `cnativeapi`, `nativeapi_flutter` on pub.dev | published |
| [Rust](../../../../bindings/rust/README.md) | `nativeapi`, `cnativeapi` on crates.io | published |
| [C#](../../../../bindings/csharp/README.md) | `NativeAPI` | build from source |
| [JavaScript / TypeScript](../../../../bindings/js/README.md) | `nativeapi` (Node.js, Deno, Bun) | build from source |
| [Python](../../../../bindings/python/README.md) | `nativeapi` (`ctypes`, Python 3.10+) | prototype |
| [Go](../../../../bindings/go/README.md) | `github.com/libnativeapi/nativeapi/bindings/go` | build from source |

## Next steps

- [Getting started](getting-started.md) — install a binding and open your first window.
- [Architecture](architecture.md) — how the core, the C ABI and the generated bindings fit together.
