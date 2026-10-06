# nativeapi (JavaScript / TypeScript)

Native desktop APIs — windows, tray icons, menus, displays, global shortcuts,
dialogs, preferences and secure storage — for **Node.js**, **Deno** and **Bun**,
on macOS, Windows and Linux.

A Node-API addon over the [libnativeapi](https://github.com/libnativeapi) C ABI,
with a typed TypeScript layer on top. Both are generated from the C++ headers of
core by `./codegen`; only the runtime (`lib/runtime.ts`, `src/napi_support.*`,
`src/event_loop_*`) is hand-written.

```ts
import { Application, Window, WindowManager } from "nativeapi";

const window = Window.create()!;
window.setTitle("Hello");
window.setSize({ width: 800, height: 600 }, false);
window.center();

WindowManager.addListener((event) => {
  if (event.type === "closed") Application.quit();
});

await Application.run(window); // timers, promises and I/O keep running
```

## Model

- **Objects** (`Window`, `Menu`, `TrayIcon`, …) wrap a `bigint` handle. The
  reference is released by `dispose()` / `using`, or when the wrapper is garbage
  collected. Calls on a released handle fail safely.
- **Values** (`Point`, `Size`, `Rectangle`, `Color`, …) are plain objects.
- **Enums** are `as const` objects: `TitleBarStyle.Hidden`.
- **Events** are discriminated unions on `type`: `{ type: "moved", windowId, newPosition }`.
  Listeners run on the JS thread and may return `Promise<void>`. Event payloads
  and borrowed objects stay valid until the Promise settles. Removal skips queued
  callbacks that have not started; active callbacks may finish. Callback errors
  release the event before reaching the runtime's error handler. Environment
  shutdown cancels that environment's outstanding deliveries and owned deferred
  decisions, releases decision handles and removes native event subscriptions. Closing
  a Node Worker leaves other environments' requests and callbacks active.
- **Singletons** (`WindowManager`, `DisplayManager`, `Application`, …) are
  classes with static members.

## The event loop

`Application.run()` doesn't block. The native event loop is pumped from the JS
event loop instead, so `await Application.run()` resolves with the exit code once
`Application.quit(code)` is confirmed. Listen for `type: "quitRequested"` and
cancel its `request`, return a Promise for confirmation, or retain a decision
from `request.defer()`. The loop keeps pumping during confirmation. Repeated
quit calls share one pending confirmation and use the last exit code; cancellation
allows a later retry. Stopping a run invalidates its pending confirmation; a late
reply cannot stop a subsequent run. `Window.close()` and native desktop close
requests share window confirmation, while macOS native application quit queries
share application confirmation. Required native termination cannot be delayed.
The host's platform loop must be serviced for native events to arrive.

## Runtimes

| Runtime | Entry | Notes |
| --- | --- | --- |
| Node.js ≥ 22.18 | `dist/` | |
| Deno 2 | `dist/` | needs `--allow-ffi --allow-read --allow-env` |
| Bun | `dist/` | |

Inside this repository, `--conditions=source` resolves the package to `lib/`
instead, so the examples run without building `dist/`.

Under hosts that own the UI thread and run JavaScript on another one —
`deno desktop` does — every native call hops to the UI thread and events arrive
asynchronously; `Application.run()` then only waits for `quit()`, since the host
already runs the loop. `isHostedEventLoop()` tells which mode is active. See
`examples/deno_detachable_window_example`.

The TypeScript in `lib/` only uses erasable syntax, so every runtime can run it
without a build step.

## Building

```bash
npm install          # at the repository root: installs the workspace and compiles the addon
npm run build        # (in bindings/js) recompile the addon with cmake-js
npm test
npm start -w js_window_example
```

To run the event ownership regressions against one shared native addon/handle table:

```bash
# From bindings/js, after cmake-js has configured the build:
cmake -S . -B build -DNATIVEAPI_JS_BUILD_TESTS=ON
cmake --build build --config Release
python ../../tools/tests/js_event_delivery.py --fixture build/Release/nativeapi.node
```

These tests launch no windows or input. They cover foreign-thread callbacks,
Promise and explicit votes, environment teardown, Node Worker isolation and
stopping/restarting the JS loop. Linux runs them under a private Xvfb display.

## Contributing

Development happens in [nativeapi](https://github.com/libnativeapi/nativeapi), which holds every binding and the code generator and checks out the core library as a submodule:

```bash
git clone --recursive https://github.com/libnativeapi/nativeapi.git
```

Files marked `AUTO-GENERATED. DO NOT EDIT.` are generated from the C++ headers in [nativeapi](https://github.com/libnativeapi/nativeapi-core). To change the API, send a pull request there; maintainers regenerate the bindings.

- API requests and native behavior bugs → [nativeapi-core issues](https://github.com/libnativeapi/nativeapi-core/issues)
- Bugs specific to one binding → [nativeapi issues](https://github.com/libnativeapi/nativeapi/issues)
- Not sure → [nativeapi-core issues](https://github.com/libnativeapi/nativeapi-core/issues)
