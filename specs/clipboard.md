# 剪贴板

`Clipboard` 是使用私有 PIMPL 的系统单例，无公共资源 ID。`ClipboardData` 是可拷贝
值结构，包含可选 UTF-8 文本、HTML 片段、图片和本地绝对文件 / 目录路径列表。
公共 API 不提供格式枚举和单独的格式查询；快捷读取只请求对应格式，`Read` 请求全部。

桌面读取在 UI 线程发起，回调在后续 UI 事件循环执行恰好一次。首参 false 表示访问、
转换或读取期间内容变化，负载为默认值且不返回部分内容；不自动重试。格式缺失正常
成功，使用 nullopt / nullptr / 空列表；存在的空文本保持 optional("")。

每次写入替换整个剪贴板，返回 bool。提交前校验 UTF-8（拒绝非法编码及内嵌 NUL）、
绝对路径，并完成平台转换；不检查文件存在性。HTML 不自动转换为纯文本；需要同时
提供时使用 Write。空包、空图片、空文件列表清空内容。发布持有独立副本；提交失败
不保证恢复旧内容。读取忽略非本地 URI。没有自定义格式、PRIMARY、历史或文件剪切。

变化事件不含内容或来源，可合并，包含自身写入和清空，不保证逐次通知。订阅不发送
初始事件；首个监听器启动监控，最后一个移除时停止。

macOS 使用 NSPasteboard，以 250ms 定时检查 changeCount。Windows 使用 Unicode
文本、HTML Format、PNG / DIBV5 / DIB、CF_HDROP，隐藏消息窗口接收变化通知。
Linux 使用 GTK 3 CLIPBOARD 的异步读取和多 target 提供器，按 GDK 能力报告
owner-change 支持；访问受 Wayland 合成器限制，退出后的持久化尽力而为。
Android、iOS、OpenHarmony 保留符号但能力探测 / 写入 false，无事件，读取立即失败。

C ABI 的复杂读取负载使用交付 lease；高级绑定在确认前复制值并 retain 图片。
Dart / JS / Python / C# 提供 Future / Promise / awaitable / Task 便利方法，失败抛出
绑定通用错误；Rust / Go 保留回调接口。

验证：自动测试校验 UTF-8、空值、绝对路径和句柄 retain。macOS 手动集成测试快照并
恢复所有原生剪贴板类型，覆盖多格式往返、并行 / 延后读取、变化失败、lease 与监听。
跨进程编辑器 / 浏览器 / 文件管理器及 X11 / Wayland 互操作仍需在相应真实桌面验证。
