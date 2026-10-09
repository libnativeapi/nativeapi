# Getting started

Pick the binding for your language, install it, and call the API. Every binding exposes the same objects — `Window`, `TrayIcon`, `Menu`, `DisplayManager` and so on — named the way that language names things.

## Install

### Dart and Flutter

A Flutter app:

```bash
flutter pub add nativeapi_flutter
```

A plain Dart program:

```bash
dart pub add nativeapi
```

### Rust

```bash
cargo add nativeapi
```

Building needs CMake and a C++17 compiler; `build.rs` compiles the core. On Linux, also install `libgtk-3-dev libx11-dev libxi-dev`.

### C#, JavaScript / TypeScript, Python and Go

These bindings are not published yet. Clone the repository with its `core` submodule and follow the binding's own page under **Bindings**:

```bash
git clone --recursive https://github.com/libnativeapi/nativeapi.git
```

## List the displays

The same program in three bindings:

```dart
import 'package:nativeapi/nativeapi.dart';

for (final display in DisplayManager.instance.getAll()) {
  print('${display.name ?? ''}: ${display.size.width}x${display.size.height}');
}
```

```rust
use nativeapi::DisplayManager;

fn main() {
    for display in DisplayManager::get_all() {
        let size = display.size();
        println!("{}: {}x{}", display.name().unwrap_or_default(), size.width, size.height);
    }
}
```

```csharp
using NativeAPI;

foreach (var display in DisplayManager.Shared.GetAll())
{
    using (display)
    {
        Console.WriteLine($"{display.Name}: {display.Size.Width}x{display.Size.Height}");
    }
}
```

## Open a window

Windows need the native event loop. In JavaScript / TypeScript the loop is pumped from the JS event loop, so timers, promises and I/O keep running:

```ts
import { Application, Window, WindowManager } from "nativeapi";

const window = Window.create()!;
window.setTitle("Hello");
window.setSize({ width: 800, height: 600 }, false);
window.center();

WindowManager.addListener((event) => {
  if (event.type === "closed") Application.quit();
});

await Application.run(window);
```

## Examples

The repository's [`examples/`](../../../../examples) directory has an example app per feature for every binding, prefixed by binding: `flutter_*`, `rust_*`, `csharp_*`, `js_*`, `deno_*`, `python_*`, `go_*` and `gpui_*`.
