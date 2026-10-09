# Architecture

nativeapi is one C++ library with generated bindings around it. A change to the native API is made once, in the core headers, and reaches every language by regenerating the bindings.

## The core

`core/` ([nativeapi-core](https://github.com/libnativeapi/nativeapi-core)) is the source of truth for the API surface. Each public header in `core/src/` declares a module — `window.h`, `tray_icon.h`, `menu.h`, `display_manager.h` and so on — and each module has an implementation per platform: AppKit on macOS, Win32 on Windows, GTK on Linux, plus Android and iOS where the Dart binding needs them.

The design rules for the public API — the object model, naming, the event system, managers and the C ABI — live in [`specs/`](../../../../specs/README.md).

## The C ABI

Every binding talks to the core through a flat C ABI: handles for objects, plain structs for values, and functions such as `native_window_set_title`. It is generated from the C++ headers, together with an umbrella header.

## The generator

`tools/codegen` parses the headers with libclang into an intermediate representation, emits the C ABI from it, and then generates every binding from the same representation. Each generator follows its language's conventions — the same setter is `window.title = …` in Dart and Python, `set_title` in Rust, `SetTitle` in C# and Go, and `setTitle` in TypeScript.

```bash
./codegen            # C ABI, then all bindings
./codegen check      # verify nothing is stale (CI mode)
```

Generated files start with `AUTO-GENERATED. DO NOT EDIT.`; hand-written runtime code (event loops, memory management, widgets) lives next to them.

## The bindings

| Binding | How it reaches the C ABI |
| --- | --- |
| Dart / Flutter | `dart:ffi`; the `cnativeapi` build hook compiles the core |
| Rust | `cnativeapi` raw FFI under the safe `nativeapi` crate; `build.rs` compiles the core |
| C# | P/Invoke against a native library built with CMake |
| JavaScript / TypeScript | a Node-API addon with a typed TypeScript layer |
| Python | `ctypes` over a shared library — no compiled extension module |
| Go | cgo against a shared library built with CMake |
