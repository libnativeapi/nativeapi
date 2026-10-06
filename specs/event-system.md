# 事件系统规范

> 状态：已实施（事件归属模型尚未统一，见 §6）
> 适用范围：`core/src/foundation/event.h`、`event_emitter.h`、`dispatcher.h`
>   及 11 个 `EventEmitter` 派生类
> 核实基准：2026-08-25

本规范回答：**事件类怎么定义、怎么发、怎么收、什么时候在哪个线程跑。**
事件里怎么引用对象见 [object-model.md](object-model.md) §5。

## 1. 事件类

全部继承 `Event`（`foundation/event.h`）。基类只给两样东西：构造时记录的
`steady_clock` 时间戳，和**纯虚**的 `GetTypeName()`。

```cpp
class MyEvent : public Event {
 public:
  explicit MyEvent(std::string data) : data_(std::move(data)) {}
  const std::string& GetData() const { return data_; }
  std::string GetTypeName() const override { return "MyEvent"; }   // 纯虚，必须实现
 private:
  std::string data_;
};
```

规则：

1. `GetTypeName()` 是 `= 0`，漏写是编译错误而非运行期缺陷，不必额外检查。
2. **按领域建层级，不要直接继承 `Event`。** 每个领域先有一个基类
   （`WindowEvent`、`WindowDragEvent`、`DragSourceEvent`、`DropTargetEvent`、`DisplayEvent`、
   `TrayIconEvent`、`MenuEvent`、`ShortcutEvent`、`KeyboardEvent`、`ApplicationEvent`），具体事件再
   继承它。这十个基类里九个定义在
   `src/<模块>.h`，只有 `KeyboardEvent` 在 `foundation/keyboard.h`——foundation 层
   本不该有领域事件，找不到时按这里查。派发靠
   `dynamic_cast` 匹配，层级就是「监听基类可收到全部子类事件」这一能力的来源。
3. 事件本身是**值对象**：可拷贝、可跨线程传递。不得按值内嵌身份对象
   （[object-model.md](object-model.md) §5）。

## 2. `EventEmitter<Base>`

发事件的类继承 `EventEmitter<领域基类>`。当前 11 个：`WindowManager`、`WindowDragSession`、`DragSource`、`DropTarget`、
`DisplayManager`、`ShortcutManager`、`KeyboardMonitor`、`Application`、
`TrayIcon`、`Menu`、`MenuItem`。

模板内有 `static_assert` 保证 `Base` 派生自 `Event`；`AddListener<T>` 另有
`static_assert` 保证 `T` 派生自 `Base`。

### 2.1 公开面 vs 受保护面

这条边界是设计意图，不是偶然：

| 可见性 | 成员 | 含义 |
|---|---|---|
| public | `AddListener<T>(回调 或 EventListener<T>*)` → `size_t` | 任何人可订阅 |
| public | `RemoveListener(id)`、`RemoveAllListeners<T>()`、`RemoveAllListeners()` | 任何人可退订 |
| public | `GetListenerCount<T>()`、`GetTotalListenerCount()`、`HasListeners<T>()` | 只读 |
| **protected** | `Emit(...)` / `EmitAsync(...)` | **只有类自己能发自己的事件** |
| **protected** | `StartEventListening()` / `StopEventListening()` | 惰性监听钩子 |
| **protected** | `ShutdownEmitter()` | 析构协议 |
| **protected** | `CreateGuardedCallback(...)` | producer 的延后完成回调；析构后不再执行 |

外部代码无法伪造某个对象的事件——这是 `Emit` 受保护带来的性质，改动可见性前先想清楚。

### 2.2 派发语义

- `Emit` 在**锁外**回调：先在锁内取监听器快照，再逐个 `Invoke`。因此回调里可以安全
  地重入 emitter（增删监听器）。
- 快照期间被移除的监听器带 `removed` 标记，`Invoke` 前跳过——回调 A 里
  `RemoveListener(B)` 之后，B 不会再被调到。
- 匹配用 `dynamic_cast<const T*>`：注册 `AddListener<WindowEvent>` 会收到全部
  `WindowXxxEvent`；注册 `AddListener<WindowMovedEvent>` 只收该一种。

## 3. 线程模型

**这是最容易记错的一条：`EmitAsync` 投递到平台主线程 / UI 线程，不是后台线程。**

| | `Emit` | `EmitAsync` |
|---|---|---|
| 回调线程 | 调用 `Emit` 的那个线程 | 平台主线程 |
| 时机 | 同步，函数返回前跑完 | 永远延后，即使已在主线程 |
| 底层 | 直接调用 | `RunOnMainThread()`（`foundation/dispatcher.h`） |

选择依据：

- 事件产生自后台线程 → `EmitAsync`。用户回调多半要碰 UI。
- **持锁时发事件 → `EmitAsync`。** `ShortcutManager` 就是这个场景：它在自己的互斥
  量内发射，延后投递才不会让用户回调跑在一把它并不知情的锁下面。
- 其余情况 → `Emit`。

`EmitAsync` 「即使已在主线程也延后」是刻意的：调用点的同步/异步语义不随线程变化，
避免出现「只在某些线程上才重入」的时序 bug。

**Android / OHOS 例外**：这两个平台尚无主线程投递机制，`EmitAsync` 会退化为
**在调用线程上同步投递**——静默丢事件比线程错更糟。跨平台代码不得假设
`EmitAsync` 的回调一定落在主线程。

当前只有 `ShortcutManager` 使用 `EmitAsync`。

### 3.1 用了 `EmitAsync` 就必须处理析构竞态

已投递但未执行的 `EmitAsync` 会落在一个正在析构的对象上。`~EventEmitter()` 里调
`ShutdownEmitter()` 太晚了——那时派生类部分已经析构完。

**规则：任何调用 `EmitAsync` 或保留 `CreateGuardedCallback` 的派生类，必须在自己析构函数的第一行调
`ShutdownEmitter()`。** 它会阻塞到进行中的派发结束，把 emitter 标记为死亡，之后到达
的投递变成 no-op；幂等，多调无害。

### 3.2 dispatcher 契约（`foundation/dispatcher.h`）

`EmitAsync` 底下是一层可插拔的主线程投递机制，绑定作者与嵌入方需要知道四件事：

- **主线程的认定**：Apple 平台由 OS 判定；其余平台默认认定「加载本库的线程」为
  主线程。假设不成立时（比如从工作线程加载的插件），在使用任何 dispatcher 功能
  之前调 `SetMainThread()`。Windows 上启动时调一次还会顺带初始化派发用的
  message-only 窗口。
- **能力探测**：`IsMainThreadDispatchSupported()` 在 Android / OHOS 返回 false；
  `RunOnMainThread()` 排不进队列时返回 false 且**不执行**传入的函数（本节开头
  `EmitAsync` 的退化行为即源于此）。
- **自带主循环的宿主**：Qt、游戏引擎等用 `SetMainThreadDispatcher()` 把投递接进
  自己的调度器；测试也用它确定性地排空队列。必须在其他线程开始使用 dispatcher
  之前设置——覆盖不做并发同步。
- **没有 UI 循环的消费者**：控制台工具、纯 C 调用方必须周期性调
  `RunMainThreadLoopFor(timeout_ms)`，否则排队的工作永远不会执行。已经在跑
  Cocoa / Win32 / GTK / Flutter 循环的应用**不得**调它——嵌套第二个循环会引入
  重入 bug。

## 4. 惰性监听：`Start` / `StopEventListening`

平台事件监控（全局快捷键钩子、`NSNotification` 观察者、GTK 信号）应该按需开关，
不在构造函数里无条件启动。

`EventEmitter` 在**第一个监听器加入**时调 `StartEventListening()`，在**最后一个移除**
时调 `StopEventListening()`。两者都在锁外调用，实现里可以安全回调 emitter。

```cpp
// tray_icon.h
class TrayIcon : public EventEmitter<TrayIconEvent>, public NativeObjectProvider {
 protected:
  void StartEventListening() override;   // pimpl_->SetupEventMonitoring()
  void StopEventListening() override;    // pimpl_->CleanupEventMonitoring()
};
```

**构造函数里不要再调 `SetupEventMonitoring()`。** 当前重写了这对钩子的有
`WindowManager`、`ShortcutManager`、`TrayIcon`、`DropTarget`（第一个监听器加入时才把窗口
注册为放置目标）、`Application`（macOS 宿主的退出委托与生命周期通知）。

macOS 的 `Application` 不替换已有的 `NSApplicationDelegate`，也不更改宿主的 activation
policy。原生 AppKit 退出查询先走可取消 producer，批准后保留宿主的
`NSTerminateCancel` / `NSTerminateLater` / `NSTerminateNow` 原始决定。最后一个监听器移除
后停止生命周期通知；已经持有的投票仍完成原宿主动作。私有委托钩子保留弱关联，用于
待决请求和宿主替换的失效处理；没有监听器且没有待决请求时直接调用原方法。
首次在绑定工作线程构造 `Application` 时，初始化排到实际 UI 线程；plain Dart 接管
UI 后的 IsRunning 查询和 Run 入口也会完成尚未执行的初始化，不能在没有 NSApp 时
开始泵循环。程序调用 `Quit()`、对象级窗口关闭和 macOS 原生退出查询均已接入 §5.1。

平台层的启停实现同样要幂等——`RemoveAllListeners()` 之后再 `AddListener` 会走第二轮
启动。


## 5. 监听器生命周期

- `AddListener` 返回 `size_t` id，退订只认 id。
- 传 `EventListener<T>*` 裸指针的重载**不接管所有权**：监听器对象必须活到退订之后。
  能用 lambda 就用 lambda 重载。
- 在持有 id 的对象析构时退订（RAII）。

### 5.1 可取消请求的共享状态

`foundation/event_request.h` 的 `EventRequest` / `EventDecision` 是身份对象，
表示一次请求及一次延后投票。事件本身仍是值对象，携带 `shared_ptr<EventRequest>`，
不能携带复制后各自独立的取消布尔值。底座、程序调用 `Application::Quit()` 和
三桌面的窗口关闭和 macOS 原生应用退出查询已实施；绑定回调所有权与实际桌面手势的回归仍须按平台验证。

- 监听器在同步回调中 `Cancel()` 否决，或者先 `Defer()` 获取拥有的 `EventDecision`，
  回调返回后再从任意线程 `Accept()` / `Cancel()`。未表态的同步监听器默认同意。
- 同步派发结束且全部延后投票同意才执行动作；任意一次否决胜过其他所有同意。
  同步派发结束之前不能执行 continuation，避免第一个监听器同意后就提前关闭。
- 延后投票最后一份引用被丢弃且未答复时取消请求；分配投票失败也取消，不能静默同意。
  重复或迟到的答复返回 false。请求完成后不会被重新开启。
- 不可取消的系统请求 `IsCancelable()` 为 false，拒绝 `Cancel()` 和 `Defer()`；
  它不能被异步确认拖住。原生目标销毁或被强制请求替代时，producer 必须使旧请求失效。
- `event_request_dispatch.h` 是私有 producer 协议，不进入 API_HEADERS。状态锁只保护
  状态转换，continuation 及其捕获对象的析构都在锁外。continuation 在最后答复所在
  线程执行；平台层必须投递回 UI 线程，并再次检查目标身份与存活状态。

C ABI 的异步事件交付与 Dart 接入已实施：交付保留负载、借用句柄和 user_data，
消费端完成后确认派发；可取消请求为每个异步监听器取得隐式延后投票。Dart 使用
`NativeCallable.listener` 切换到注册 isolate，等待 `FutureOr<void>` 回调完成，
异常或消费前退订时取消交付。真实原生 worker → Dart isolate 回归覆盖负载、
Future、借用对象、移除和异常。细节见 [c-abi.md](c-abi.md) §6.2。

JS 的跨线程排队 / Promise 接入已实施。原生借用句柄保留到 Promise 结束；
异常先否决并释放交付，再进入 JS 错误处理器。环境清理取消该环境的全部未决交付及显式 owned EventDecision，释放已完成的投票句柄，
退订原生事件监听器，包括排队后未调用的回调和永不结束的 Promise。回调状态、线程和
投票所有权按 napi_env 隔离；关闭 Node Worker 不得取消其他环境的投票或交付。
N-API producer 临时引用保护排队与环境 finalizer 的并发；原生 user_data 使用不复用的
私有 token，TSFN 单独保留真实 Callback，迟到的 release 不得命中新环境的分配地址。
实际 Node 环境销毁回归验证原生投票取消、句柄归零、Worker 隔离及迟到释放。

程序调用 `Application::Quit()` 产生非空的可取消请求；所有同步 listener 派发完毕，
且所有显式 / 隐式延后投票同意后，才执行平台 Quit。等待确认以及 UI completion 排队
期间的重复调用共用请求，更新退出码；取消后可重新请求。投票完成在 UI 线程执行，
析构首先 `ShutdownEmitter()`，再使未决请求失效。排队失败不能在 worker 上关闭；
下一次 UI 请求可恢复。监听器抛异常按否决处理。没有监听器则立即执行原有平台 Quit。

JS `Application.quit()` 复用同一 producer；确认完成后发送退出事件，再结束 JS 自己的
循环，避免误用宿主进程终止路径。私有 `application_quit_dispatch.h` 不进入
API_HEADERS，JS 的 completion 用 RAII 保留回调，并通过现有宿主 UI 投递入口运行。
每轮 JS run 有独立弱 owner；stopEventLoop 与环境销毁使旧确认及 completion 失效，
旧投票不能结束新一轮循环。真实 Node / Dart
回归验证请求对象的借用寿命、异步确认、否决和重试；三桌面 C++ 回归验证 UI 投递和去重。

Python `run_async()` 的 quit helper 也复用同一 producer，批准后才完成 asyncio Future。
原生平台已发出的退出信号直接完成循环，不进行第二次确认。每次 asyncio 运行有独立的
generation 与私有弱 owner：任务取消后立即使请求失效，排队中的旧请求 / completion
不能结束下一轮，旧取消也不能使新 owner 的请求失效。completion 的内部 ctypes 回调
在最后一个原生引用释放时直接归还，避免平台 pump 停止后仍等待下一轮才释放。
运行结束及解释器 atexit 清理还会提前释放仍在原生队列中的内部回调，并使其 context
失效；C++ 对象在 Python 销毁后释放时不能再次调用 ctypes trampoline。
Python listener 异常先否决可取消请求再上报；async def listener 被拒绝，异步确认通过
同步 listener 取得 owned EventDecision 后调度 asyncio task。三桌面真实 ctypes
回归使用同一 DSO，仅替换平台启动 / pump；不创建窗口，不模拟 producer。

`Window` 继承 `EventEmitter<WindowEvent>`，每个原生身份共享关闭请求；多个包装对象的
显式 / 隐式投票必须全部同意。`Close()`、macOS `performClose:`、Windows `SC_CLOSE`
和 GTK `delete-event` 接同一 producer，批准后在原生窗口所属 UI 线程继续宿主处理，
宿主仍可拒绝。强制的 `close` / `WM_CLOSE` / GTK destroy 与 Windows session 通知
只发不可取消请求，继续原始操作并使旧投票失效。GTK 保留原有 delete handler 的
顺序和 block count，不接管全局 GDK 事件分发。WindowManager 继续只发观察事件；
对象的观察事件复用现有监控，并保持原有“未显示过的窗口不发 Closed”语义。
Android / iOS / OpenHarmony 的 Close 暂不支持，返回 false。

真实 Flutter 3.47.6 多窗口 + Dart FFI 回归验证 `nativeWindowOf`、对象监听器及显式投票：
批准后继续原 `RegularWindowControllerDelegate`，宿主拒绝仍保留窗口，宿主批准自行
`destroy()`，旧包装对象不能再关闭已销毁的窗口。原生 `NSWindow.close` 关闭 OS 窗口，
Flutter `destroy()` 拥有视图 / 控制器清理；两种强制路径均不得等待投票，并使旧投票失效。
测试直接加载 bundle build 生成的 native asset，不在 C++ fixture 中链接第二份 core。
窗口保持未显示，不发送输入；实际桌面输入手势的验收仍按平台单独执行。

macOS 原生 `applicationShouldTerminate:` 与程序 `Quit()` 共用请求槽；nativeapi 的
全部投票批准后才查询宿主。宿主的 `NSTerminateLater` 保留请求至对应 AppKit reply；
拒绝释放请求，替换 delegate 或必须继续的退出使旧投票失效。最终 WillTerminate
通知与 Flutter required exit 发不可取消请求；它们不等待投票或调度第二次退出。

Flutter 的 `shouldTerminate` 在发出 Dart 请求时就设为 true，不能作为批准凭据。
私有适配器验证实际 runtime 方法签名与 ivar 编码，按每次 engine request 的独立 token
重放原 terminator，保留其 sender 与回调；只有对应的 nativeapi 批准作用域可以继续退出。
Dart cancel 释放对应请求；迟到的已失效响应不能借用新请求的批准。无人监听时保持
原始 Flutter 路径。未知 Flutter runtime 布局不安装适配器，保留原始宿主行为；新版本
需要重跑真实 headless engine + Dart AppLifecycleListener 回归，而不是依赖 flag 猜测批准。

## 6. 未决：事件归属模型

同一库里并存四种事件 / 回调模型，新代码按「目标」一列做，存量不要照抄：

| 类 | 现状 | 目标 |
|---|---|---|
| `Window` | 继承 `EventEmitter<WindowEvent>`，原生身份共享关闭请求；保留 manager 全局观察 | 已符合：对象事件挂对象，manager 不发关闭请求 |
| `TrayIcon` / `Menu` / `MenuItem` | 各自继承 `EventEmitter`，对象自己发 | 已符合 |
| `Shortcut` | 双轨：`ShortcutOptions::callback` 的 `std::function` + `ShortcutManager` 的 `ShortcutActivatedEvent` | 收敛为事件模型 |
| `KeyboardMonitor` | 手动 `Start()` / `Stop()`，不走惰性监听；公开了 `GetInternalEventEmitter()` | 改用 `Start`/`StopEventListening`，移除该公开方法 |

规则：**对象级事件挂对象，系统级事件（增删、全局监控）挂 manager。** 它会传导到
C ABI——`native_tray_icon_add_listener`、`native_window_add_listener` 挂在对象上，
`native_window_manager_add_listener` 保留全局窗口观察。

事件类结构上的存量缺口：`WindowEvent` 基类存 id，而 `TrayIconEvent` / `MenuEvent` 基类
为空、各子类重复一份 id 字段（C 侧后果是 id 在每个 union 分支里重复）；Menu 缺
`MenuItemEvent` 中间基类，`MenuId` 与 `MenuItemId` 事件混在同层。新领域一律「基类存 id」。

## 7. 检查单

- [ ] 新事件继承的是**领域基类**，不是 `Event`。
- [ ] `GetTypeName()` 已实现。
- [ ] 事件不按值内嵌身份对象，改带 `shared_ptr` 或整数 ID。
- [ ] 发射者继承 `EventEmitter<领域基类>`；`Emit` 保持受保护。
- [ ] 后台线程或持锁场景用 `EmitAsync`。
- [ ] 用了 `EmitAsync`：析构函数第一行是 `ShutdownEmitter()`。
- [ ] 平台监控放进 `Start`/`StopEventListening`，不放构造函数，且启停幂等。
