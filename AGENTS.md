# nativeapi

A modern cross-platform C++17 library providing unified access to native system APIs. Platform
details are hidden behind a per-module seam; an optional C API layer (`src/capi/`) exposes
everything for FFI from Dart, Rust, C# and others.

## Layout

```
include/nativeapi.h    # single public include
src/
├── foundation/        # events, dispatch, handle table, ID allocation, geometry
├── capi/              # C ABI bindings — 27 of 28 headers are GENERATED
├── platform/          # windows, macos, linux, android, ios, ohos — one is built
└── *.h, *.cpp         # cross-platform interface definitions (25 public headers)
examples/              # 23 examples; these double as integration tests
```

## Design specs

The design rules for this library live in the **libnativeapi workspace repo**, under
`specs/` — one directory up when this repo is checked out as the workspace's `core/`
submodule. Read them before adding or reshaping public API:

| Spec | Covers |
| --- | --- |
| `../specs/architecture.md` | Layering, the six-platform matrix, naming, build |
| `../specs/object-model.md` | Identity objects vs value objects |
| `../specs/api-style.md` | Method vocabulary, parameter/return types, failure reporting, doc comments |
| `../specs/platform-seam.md` | PIMPL, narrow seams, `NativeObjectProvider` |
| `../specs/event-system.md` | `Event` / `EventEmitter`, threading, lazy listening |
| `../specs/managers.md` | Singletons, registries, the handle table |
| `../specs/c-abi.md` | What codegen produces and how types cross the boundary |
| `../specs/handle-ownership.md` | C ABI handle ownership and invalidation |

Each spec also records the open questions and known legacy gaps of its own area inline;
there is no separate issue list.

## Non-negotiables

1. **No platform types in public headers** — no `HWND`, `NSWindow*`, `GtkWidget*`, no
   `<windows.h>`, no `#ifdef` platform branches in `src/*.h`.
2. **A new cross-platform module is six files** — every directory under `src/platform/`
   needs an implementation, or that platform fails to link.
3. **PIMPL discipline** — forward-declare `class Impl`, hold `std::unique_ptr<Impl> pimpl_`,
   define the destructor in the `.cpp`, delegate between constructors.
4. **Emit events through `EventEmitter<T>`**; `EmitAsync` lands on the *main* thread, and any
   class using it must call `ShutdownEmitter()` first thing in its destructor.
5. **Expose native handles only via `NativeObjectProvider`**, and never transfer ownership.
6. **Never hand-edit `src/capi/`** — change the C++ header and run `./codegen` from the
   workspace root. Files carrying `// AUTO-GENERATED. DO NOT EDIT.` are overwritten.
7. **New handle types need an `IdTypeTag<T>`** entry in
   [src/foundation/id_allocator.h](src/foundation/id_allocator.h) — append only, never
   renumber.

## Public API style

The full rules and the review checklist are in `../specs/api-style.md`. Everything public in
`src/*.h` is exported verbatim to C, Dart, Rust and C#, so the short version is:

- **Properties** are `SetX(v)` + `GetX() const`, booleans `SetX(bool is_x)` + `IsX() const`,
  declared as an adjacent pair. Getters are always `const` — codegen only turns `const`
  no-arg `Get`/`Is`/`Has` methods into binding properties.
- **Actions** are bare imperative verbs with a fixed opposite (`Show`/`Hide`, `Open`/`Close`,
  `Maximize`/`Unmaximize`, `Enable`/`Disable`) and an `IsXxx() const` state query.
- **No new overloads** — they surface as `native_x_verb_with_<params>`. Different meaning,
  different name (`RemoveItemById`, `RemoveItemAt`).
- **Types**: strings in as `const std::string&`, out by value; geometry and enums by value;
  identity objects only as `std::shared_ptr<T>` (`nullptr` = none), never `const T&` / `T*`;
  `std::optional` wraps strings only; floating point is `double`; IDs use the `XxxId` alias.
- **Enums**: `enum class`, PascalCase values, no `k` prefix, sequential from 0, first value
  is the neutral default, append only.
- **Failure**: never throw across the public API. `void` when it cannot fail, `bool` when a
  platform may not support it (document what `false` means), `nullptr` for lookups and
  factories. Do not invent another error channel.
- **Platform differences live in the doc comment**, not the signature: every API exists on
  all six platforms, and one that behaves differently carries the six-line
  `@note Platform availability:` block (✅ / ⚠️ / ❌).
- **"Intercept before X" is a cancellable event on the object**, not another
  `SetWillXxxHook`; internal entry points stay out of the `public:` section.
- After editing a header, run `./codegen` from the workspace root and make sure no
  `skipped` warning names the new API.

Where existing headers disagree, the spec says which side is the rule — do not copy the
nearest neighbour.

## Naming

- C++ — classes and methods `PascalCase`, members `snake_case_`, files `snake_case.h`,
  platform files `<module>_<platform>.<ext>`.
- C — handles `native_*_t` (`uint64_t`), functions `native_<module>_<verb>`, enums
  `NATIVE_*`, files `*_c.h`. All generated; do not write them by hand.

## Build

CMake, C++17, propagated via `target_compile_features(nativeapi PUBLIC cxx_std_17)`.
`src/CMakeLists.txt` selects exactly one platform directory per build. Verify changes
against the relevant program in `examples/`.

When this repo is the `core/` submodule of the libnativeapi workspace, a change to any
public header ripples into the generated bindings — run `./codegen sync` from the
workspace root rather than hand-editing generated files.

## Commits

- Do not add Co-Authored-By trailers to commits.
