# 单例与注册表规范

> 状态：已实施（单例写法已统一为 Meyer's；能力型 API 形态与注册语义未决，见 §5）
> 适用范围：`core/src/` 全部 `GetInstance()` 类型
> 核实基准：2026-08-25，11 个单例全部为 Meyer's

本规范回答：**系统级资源由谁持有、怎么访问、跟 ID 查找表和句柄表是什么关系。**
对象本身的语义见 [object-model.md](object-model.md)。

## 1. 单例清单

| 单例 | 角色 |
|---|---|
| `WindowManager` | 窗口的创建、枚举、平台事件转发 |
| `DisplayManager` | 显示器枚举、实例缓存与 diff |
| `TrayManager` | 托盘图标集合 |
| `ShortcutManager` | 全局快捷键注册 |
| `Application` | 应用级生命周期与事件 |
| `AccessibilityManager` | 辅助功能权限 |
| `UrlOpener` | 打开外部 URL |
| `AppInfo` / `DeviceInfo` | 只读的应用 / 设备信息 |
| `WindowRegistry` | `WindowId → shared_ptr<Window>` 查找 |
| `HandleTable`（foundation） | C ABI 句柄表，见 [handle-ownership.md](handle-ownership.md) |

`KeyboardMonitor` **不是**单例——它是可多实例的 `EventEmitter`。旧文档把它列进 manager
清单是错的。

## 2. 唯一合法写法：Meyer's

```cpp
// 头文件
class WindowManager : public EventEmitter<WindowEvent> {
 public:
  static WindowManager& GetInstance();
  virtual ~WindowManager();

  WindowManager(const WindowManager&) = delete;
  WindowManager& operator=(const WindowManager&) = delete;
  WindowManager(WindowManager&&) = delete;
  WindowManager& operator=(WindowManager&&) = delete;

 private:
  WindowManager();                       // 私有构造
  class Impl;
  std::unique_ptr<Impl> pimpl_;
};

// 实现
WindowManager& WindowManager::GetInstance() {
  static WindowManager instance;         // C++11 起，初始化是线程安全的
  return instance;
}
```

规则：

1. **函数内 `static` 局部变量**，不用裸静态指针，不用 `std::once_flag`，不用堆分配。
   `DisplayManager` 曾用手写 `static DisplayManager* instance_`，已收敛。
2. 构造函数私有；拷贝/移动四件套全部 `delete`。当前只有
   `accessibility_manager.h` 少了移动两件（仅写了拷贝两件），属于待补齐的不一致。
3. `GetInstance()` 返回**引用**。调用方不得取地址存进 `shared_ptr`/`unique_ptr`，
   不得 `delete`。
4. 单例自身通常也用 PIMPL 或窄接缝（[platform-seam.md](platform-seam.md)）。

### 2.1 静态析构顺序

Meyer's 单例在首次调用时构造、在静态析构阶段销毁，**销毁顺序不确定**。因此：

- 一个单例的析构函数里不要调用另一个单例的 `GetInstance()`——对方可能已经没了。
- 全局对象/静态变量的析构函数里不要碰单例，同理。
- 需要确定性的关停顺序，就提供显式的 `Shutdown()` 由应用在退出前调用，
  不要依赖析构顺序。

Linux 的 `WindowManager` 保存并移除 GTK show/hide emission hook ID，同时断开逐窗口
信号和 weak ref；`ShortcutManager` 在成员销毁前停止并 join X11 工作线程。
真实 Flutter 退出回归见 `tools/gui/core_application_exit_flutter_linux.py`：X11 和 Wayland
均覆盖主动移除监听器、管理器存活至进程退出两条路径，以及次窗口关闭后的隐式 view 保留。

## 3. Manager 的职责

Manager 该做的：

1. **实例去重**：同一底层资源的重复查询返回同一个 `shared_ptr`
   （[object-model.md](object-model.md) §2 规则 3）。`DisplayManager` 按平台身份 key
   缓存即此。
2. **发领域事件**：manager 是 `EventEmitter`，平台回调转成库事件由它发射。
3. **管平台监控的启停**：通过 `Start`/`StopEventListening`
   （[event-system.md](event-system.md) §4），不在构造函数里无条件开启。

Manager 不该做的：

- 不承载与自身领域无关的能力（`DisplayManager::GetCursorPosition()` 属于越界——
  它该属于未来的 Cursor / Mouse API）。
- 不把平台内部机制开成公共方法（`WindowManager` 的 swizzle 钩子已被 codegen 原样
  导出成 8 个 C 函数；这组钩子正在退役，见 [api-style.md](api-style.md) §8）。

## 4. 三张表的分工

三者都是「按某个键找到对象」，但服务的对象不同，**不可互相替代**：

| | 键 | 值 | 服务谁 | 管什么 |
|---|---|---|---|---|
| Manager 缓存 | 平台身份（私有） | `shared_ptr<T>` | C++ 侧 | 实例去重 |
| `WindowRegistry` | `WindowId` | `shared_ptr<Window>` | C++ 侧 | 按 ID 查找 |
| `HandleTable` | `native_*_t`（uint64） | `shared_ptr<void>` + 类型 tag | C ABI | 跨语言引用生命周期 |

事件负载里带的是 `WindowId`，`WindowManager::Get(id)` 依赖 `WindowRegistry`，
这条查找路径句柄表替代不了。反过来，句柄的世代校验和类型校验也不是注册表的职责。
详见 [handle-ownership.md](handle-ownership.md) §2.3。

`WindowRegistry` 内部是 `ObjectRegistry<Window, WindowId>`
（`foundation/object_registry.h`）——一个加锁的 `unordered_map` 模板，当前只有这一个
使用者。需要第二张同形状的表时复用它，不要另写。

### 4.1 注册表条目的移除时机

`WindowRegistry` 是**以 WindowId 为键的包装对象缓存，不是所有权登记表**。同一个原生
窗口可以有多个 `Window` 包装对象共享一个 WindowId，因此**不能**在 `~Window()` 里注销
——任一临时包装对象析构都会把条目删掉，而原生窗口还活着。

正确的时机是原生窗口关闭（macOS `NSWindowWillCloseNotification`、Windows
`WM_NCDESTROY`、GTK `destroy`）。这项尚未在各平台接齐，进度见
[handle-ownership.md](handle-ownership.md) §2.4。

## 5. 未决

- **异步快捷键注册（待发布依赖回归）**：`ShortcutManager::Register` 同步返回已注册对象，
  不能表达 Wayland GlobalShortcuts Portal 等待用户授权的流程。不得通过阻塞 Flutter
  UI isolate 或先返回一个实际未注册的 `Shortcut` 来伪装完成。
  新增的 `RequestRegistration(options, registration_key) -> ShortcutId` 返回非零请求 ID，
  最终结果复用 `ShortcutRegisteredEvent` / `ShortcutRegistrationFailedEvent` 并携带同一 ID；
  重复或无效请求同步返回 0，不冒充另一个同组合请求的完成事件。请求 ID 同时是注册
  成功后的 ShortcutId，调用方必须按 ID 关联结果，不按 accelerator 字符串猜测。
  `registration_key` 是调用方提供、应用内稳定的语义键；`Shortcut` 提供只读
  `GetRegistrationKey()`，与运行期 `ShortcutId` 分开。人类可读的 `description`
  保留为授权界面的动作说明，不承载内部所有权前缀。
  `CancelRegistration(request_id) -> bool` 取消该次请求；
  `IsRegistrationPending(request_id) const` 查询待决状态。Portal 后端应关闭尚未完成的请求，
  `UnregisterAll` 和析构必须回收待决请求及已生效会话。延后完成回调使用
  `CreateGuardedCallback`，管理器析构先执行 `ShutdownEmitter()`；平台完成回调和
  资源释放均不得在 manager 锁内重入用户代码。Portal 的绑定由桌面确认和保存，
  accelerator 只是偏好值，不能承诺绕过权限或覆盖桌面保存的选择。
  协议依据：[GlobalShortcuts](https://flatpak.github.io/xdg-desktop-portal/docs/doc-org.freedesktop.portal.GlobalShortcuts.html)。
  通用生命周期、Portal 传输及六种绑定现已实现；隔离 D-Bus 服务覆盖了提前响应、
  取消、拒绝、桌面修改绑定、服务断开和真实 manager 的激活/抬键分发。
  能力查询也由 Portal 工作线程执行，交互式注册入口不能同步等待失联的桌面服务。
  `CancelPendingRegistrations(nonempty_key_prefix)` 可按模块回收热重启遗留的待决请求，
  不影响其他模块和调用前已生效的注册；取消期间刚批准的匹配请求也必须释放。
  Omarchy 已通过纯 Wayland 的 Flutter Release 端到端测试：两轮稳定动作键注册、
  Hyprland 目标动作 IPC 到 Dart 回调、AOT 标签和 SDK 句柄清理，不使用键盘输入。
  Ubuntu NUC 和 Omarchy 的 X11 回归均通过。Hyprland Portal 1.4.1 会保留动作目录，
  不能将目录条目等同于仍存活的客户端会话；隔离服务另行验证 Close 报文确实发出。
  上层取消、拒绝、热重启、服务失效及失败后重试已覆盖；仍需完成发布依赖后的跨平台回归。
- **能力型 API 形态分裂**：`UrlOpener` 是单例，`LaunchAtLogin` 是多实例，
  `AccessibilityManager` 是单例但只有两个方法（有 `Enable()` 无 `Disable()`，
  `IsEnabled()` 只读本地 flag 而不查系统授权状态）。无状态能力型 API 该统一成 static
  函数集还是保留单例，尚未选定；新模块先跟最近的 `AppInfo` / `DeviceInfo`（单例）。
- **对象创建 / 注册模式不统一**：`Window()` 构造即隐式注册进 `WindowRegistry`；
  `TrayIcon` 文档要求直接 `make_shared`，`TrayManager` 却又维护 `trays_` 映射，注册路径
  不明；`Shortcut` 必须走 `ShortcutManager::Register`；`Image` 只能走静态工厂。
  `WindowManager::Create(options)` 在多处文档示例里出现但**并不存在**——补上它还是清掉
  示例，二选一。
- **ID 计数器双轨**：`TrayManager::next_tray_id_`、`ShortcutManager::next_shortcut_id_`
  与 `IdAllocator` 并存，应统一走后者；`ShortcutId` 在 `shortcut.h` 与
  `shortcut_manager.h` 重复定义。
- 能力探测的命名与形态见 [api-style.md](api-style.md) §1.4。

## 6. 检查单

- [ ] 新单例用 Meyer's：函数内 `static`、私有构造、四件套 `delete`、返回引用。
- [ ] 析构函数不触碰其他单例。
- [ ] 平台监控走 `Start`/`StopEventListening`，不在构造函数里开。
- [ ] 需要按 ID 查找：先看能否复用 `ObjectRegistry`。
- [ ] 想加公共方法前，先确认它属于这个 manager 的领域（§3）。
- [ ] 真的需要单例吗——多实例类型（如 `KeyboardMonitor`）不要塞进这套模式。
