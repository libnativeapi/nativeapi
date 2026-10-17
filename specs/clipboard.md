# 剪贴板

`Clipboard` 是使用私有 PIMPL 的系统单例，无公共资源 ID。`ClipboardData` 是可拷贝
值结构，包含可选 UTF-8 文本、HTML 片段、图片和本地绝对文件 / 目录路径列表。
公共 API 不提供格式枚举和单独的格式查询；快捷读取只请求对应格式，`Read` 请求全部。

桌面读取在 UI 线程发起；移动端实现将调用转交宿主 UI 线程，回调在后续 UI 事件循环执行恰好一次。首参 false 表示访问、
转换或读取期间内容变化，负载为默认值且不返回部分内容；不自动重试。格式缺失正常
成功，使用 nullopt / nullptr / 空列表；存在的空文本保持 optional("")。

每次写入替换整个剪贴板，返回 bool。提交前校验 UTF-8（拒绝非法编码及内嵌 NUL）、
绝对路径，并完成平台转换；macOS/Windows/Linux 不检查文件存在性，OpenHarmony 仍遵守沙箱可读性要求。HTML 不自动转换为纯文本；需要同时
提供时使用 Write。空包、空图片、空文件列表清空内容。发布持有独立副本；提交失败
不保证恢复旧内容。读取文件路径忽略非本地 URI；Android 图片可以从有授权的 content URI 解码，文件路径只返回本地 file URL。没有自定义格式、PRIMARY、历史或文件剪切。

变化事件不含内容或来源，可合并，包含自身写入和清空，不保证逐次通知。订阅不发送
初始事件；首个监听器启动监控，最后一个移除时停止。

macOS 使用 NSPasteboard，以 100ms 定时检查 changeCount。Windows 使用 Unicode
文本、HTML Format、PNG / DIBV5 / DIB、CF_HDROP，隐藏消息窗口接收变化通知。
Linux 使用 GTK 3 CLIPBOARD 的异步读取和多 target 提供器，按 GDK 能力报告
owner-change 支持；访问受 Wayland 合成器限制，退出后的持久化尽力而为。
Android 使用 C++ 经 JNI 调用系统 ClipboardManager，不带 Java/Kotlin companion、provider 或
Flutter 插件。原生宿主在 `ANativeActivity_onCreate` 的 UI 线程调用平台内部接缝
`android::Initialize(activity)`，该接缝接入宿主 ALooper 并转发既有焦点/销毁回调。
监听期间以 100ms timerfd 检查元数据修订；失焦不采样，恢复焦点立即检查，订阅不发初始事件。
API 26 以下没有时间戳，前台检查 ClipData 的值；Android 10+ 的系统访问限制仍适用。
宿主初始化之前探测/写入 false，读取立即失败。文本和 text+HTML 写入可用，清空需要
API 28+；有授权的图片 URI 可读取，本地 file URL 可读取路径。图片/文件跨应用发布需要
宿主 content provider，本实现返回 false，不生成私有 file URI、不写公共 MediaStore。
普通 Flutter 宿主没有此原生初始化接缝时不报告 Android 支持。
iOS 使用 UIPasteboardChangedNotification 和 UIApplicationDidBecomeActiveNotification
检查 changeCount，不因事件自动读取内容或触发粘贴提示；读取遵循 iOS 的权限策略，
后台变化在恢复前台时合并通知。图片持有独立所有权，支持 ARC 和 MRC 编译。
OpenHarmony 使用公开 NDK Pasteboard / UDMF，支持文本、HTML、独立 PixelMap 图片、本地文件 / 目录、多格式写入、清空和本地变化通知。
UIAbility 的 N-API 模块初始化时须在 ArkTS UI 线程调用平台内部接缝
`ohos::Initialize(env)`；它注册 N-API thread-safe function，并在环境清理时停止投递。
读取延后到 UI 事件循环，工作线程写入同步转交 UI 线程；读取遵循
`ohos.permission.READ_PASTEBOARD` 和系统粘贴授权策略。图片通过公开 Image NDK 转为独立像素副本。
文件写入先检查当前沙箱访问权限，再由公开 FileUri NDK 转换；权限仍由系统 Pasteboard 管理。
文件读取只返回实际可用的本地路径；其他应用的私有 URI 不能被当作本应用同名沙箱路径，无法
映射的非本地 URI 被忽略。跨应用文件授权与互操作仍需独立接收应用验证。
初始化前能力探测 / 写入 false、读取立即失败。最后一个监听器移除时解绑原生 observer。

C ABI 的复杂读取负载使用交付 lease；高级绑定在确认前复制值并 retain 图片。
Dart / JS / Python / C# 提供 Future / Promise / awaitable / Task 便利方法，失败抛出
绑定通用错误；Rust / Go 保留回调接口。

验证：自动测试校验 UTF-8、空值、绝对路径和句柄 retain。macOS 手动集成测试快照并
恢复所有原生剪贴板类型，覆盖多格式往返、并行 / 延后读取、变化失败、lease 与监听。
跨进程编辑器 / 浏览器 / 文件管理器及 X11 / Wayland 互操作仍需在相应真实桌面验证。

移动端回归：`core/tests/clipboard_ios_test.mm` 为 C++/UIKit 模拟器宿主，覆盖空值、Unicode、
多格式、图片、文件 URL 与监听生命周期；Android 使用 NativeActivity C++ 宿主，验证初始化、
异步文本读取、焦点恢复和监听生命周期。测试只在独立模拟器/仿真器中运行，不向真实设备
或用户桌面注入输入。缺失最终 PASS 标记、构建成功但未运行、系统拒绝访问均不算通过。
鸿蒙集成宿主位于 `core/tests/ohos_smoke/`，同时验证设备、显示、偏好存储和剪贴板。
