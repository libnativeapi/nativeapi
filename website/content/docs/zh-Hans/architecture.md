# 架构

nativeapi 是一个 C++ 库，外面包着生成的绑定。原生 API 的改动只需在核心头文件里做一次，重新生成绑定后即可抵达所有语言。

## 核心

`core/`（[nativeapi-core](https://github.com/libnativeapi/nativeapi-core)）是 API 的唯一事实来源。`core/src/` 中的每个公开头文件声明一个模块——`window.h`、`tray_icon.h`、`menu.h`、`display_manager.h` 等——每个模块在各平台各有实现：macOS 用 AppKit，Windows 用 Win32，Linux 用 GTK，Dart 绑定需要时还有 Android 与 iOS。

公开 API 的设计规则——对象模型、命名、事件系统、管理器与 C ABI——见 [`specs/`](../../../../specs/README.md)。

## C ABI

所有绑定都通过扁平的 C ABI 与核心通信：对象用句柄表示，值用普通结构体，函数形如 `native_window_set_title`。它由 C++ 头文件生成，并附带一个总头文件。

## 生成器

`tools/codegen` 用 libclang 将头文件解析为中间表示，由此生成 C ABI，再从同一份表示生成每个绑定。每个生成器都遵循所属语言的惯例——同一个 setter 在 Dart 和 Python 中是 `window.title = …`，在 Rust 中是 `set_title`，在 C# 和 Go 中是 `SetTitle`，在 TypeScript 中是 `setTitle`。

```bash
./codegen            # 先生成 C ABI，再生成全部绑定
./codegen check      # 检查生成结果是否过期（CI 模式）
```

生成的文件以 `AUTO-GENERATED. DO NOT EDIT.` 开头；手写的运行时代码（事件循环、内存管理、组件）与之并存。

## 绑定

| 绑定 | 接入 C ABI 的方式 |
| --- | --- |
| Dart / Flutter | `dart:ffi`；`cnativeapi` 的构建钩子编译核心 |
| Rust | 安全的 `nativeapi` crate 之下是原始 FFI `cnativeapi`；`build.rs` 编译核心 |
| C# | 对 CMake 构建的原生库做 P/Invoke |
| JavaScript / TypeScript | Node-API 插件加上带类型的 TypeScript 层 |
| Python | 基于共享库的 `ctypes`，无需编译扩展模块 |
| Go | 通过 cgo 调用 CMake 构建的共享库 |
