# nativeapi

nativeapi 为 Dart/Flutter、Rust、C#、JavaScript/TypeScript、Python 和 Go 提供同一套 API，覆盖桌面应用的原生部分——窗口、托盘图标、菜单、显示器、键盘、对话框、存储等——并在每个平台上用 C++ 实现一次。

> [!NOTE]
> nativeapi 仍在积极开发中，API 可能还会变化。Dart 与 Rust 的包已经发布，其余绑定目前需从源码构建。

## 覆盖范围

| 领域 | 示例 |
| --- | --- |
| 窗口 | 创建、移动与缩放，标题栏样式，拖动移动与拖动缩放区域，视觉效果，异形窗口，多窗口 |
| 托盘图标与菜单 | 菜单栏与通知区域图标，带子菜单、勾选项和快捷键的原生菜单 |
| 显示器与输入 | 显示器边界、工作区与缩放比例，光标位置，全局快捷键，键盘监听 |
| 系统服务 | 消息与文件对话框、偏好设置、安全存储、剪贴板、开机启动、打开链接、辅助功能 |
| 应用 | 生命周期事件、退出确认、原生事件循环 |

## 语言绑定

| 绑定 | 包 | 状态 |
| --- | --- | --- |
| [Dart / Flutter](../../../../bindings/dart/README-ZH.md) | pub.dev 上的 `nativeapi`、`cnativeapi`、`nativeapi_flutter` | 已发布 |
| [Rust](../../../../bindings/rust/README.md) | crates.io 上的 `nativeapi`、`cnativeapi` | 已发布 |
| [C#](../../../../bindings/csharp/README.md) | `NativeAPI` | 从源码构建 |
| [JavaScript / TypeScript](../../../../bindings/js/README.md) | `nativeapi`（Node.js、Deno、Bun） | 从源码构建 |
| [Python](../../../../bindings/python/README.md) | `nativeapi`（`ctypes`，Python 3.10+） | 原型 |
| [Go](../../../../bindings/go/README.md) | `github.com/libnativeapi/nativeapi/bindings/go` | 从源码构建 |

## 下一步

- [快速开始](getting-started.md)——安装绑定并打开第一个窗口。
- [架构](architecture.md)——核心、C ABI 与生成的绑定如何协同。
