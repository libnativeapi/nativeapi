# 快速开始

选择你所用语言的绑定，安装后即可调用 API。每个绑定都提供相同的对象——`Window`、`TrayIcon`、`Menu`、`DisplayManager` 等——并按该语言的习惯命名。

## 安装

### Dart 与 Flutter

Flutter 应用：

```bash
flutter pub add nativeapi_flutter
```

纯 Dart 程序：

```bash
dart pub add nativeapi
```

### Rust

```bash
cargo add nativeapi
```

构建需要 CMake 和 C++17 编译器，核心由 `build.rs` 编译。Linux 上还需安装 `libgtk-3-dev libx11-dev libxi-dev`。

### C#、JavaScript / TypeScript、Python 与 Go

这些绑定尚未发布。克隆仓库及其 `core` 子模块，再按 **语言绑定** 下对应页面操作：

```bash
git clone --recursive https://github.com/libnativeapi/nativeapi.git
```

## 列出显示器

同一个程序的三种写法：

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

## 打开窗口

窗口需要原生事件循环。在 JavaScript / TypeScript 中，事件循环由 JS 事件循环驱动，定时器、Promise 和 I/O 都会照常运行：

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

## 示例

仓库的 [`examples/`](../../../../examples) 目录为每个绑定的每项功能都提供了示例应用，按绑定加前缀：`flutter_*`、`rust_*`、`csharp_*`、`js_*`、`deno_*`、`python_*`、`go_*` 和 `gpui_*`。
