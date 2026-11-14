# 自定义协议（深链接）

> 状态：已实施（桌面实机、移动端模拟器的验证见 §6）
> 适用范围：`core/src/protocol_handler.h`、`protocol_handler.cpp`、`protocol_handler_dispatch.h`、
>   `core/src/platform/*/protocol_handler_*`，以及 leanflutter `protocol_handler` 插件
>   （子模块 `bindings/dart/protocol_handler`）的原生部分
> 核实基准：2026-11-14

`ProtocolHandler` 处理的是**入站** URL：应用自己的 scheme 被系统交给本进程。出站链接是
`UrlOpener` 的事，两者不共享代码。

## 1. 对象模型

系统单例，私有 PIMPL，无资源 ID，继承 `EventEmitter<ProtocolHandlerEvent>`。

- **初始 URL**：启动本进程的那个 URL，`GetInitialUrl()` 读取，只设置一次。之后到达的每个
  URL 都发 `ProtocolUrlReceivedEvent`（`GetUrl()`），经 `EmitAsync` 落在 UI 线程。
- **晚到的启动 URL**：系统可能在绑定的启动代码之后才交付启动 URL（macOS 实测：Dart 读
  `getInitialUrl()` 比 AppKit 交付 GURL 早约 300 ms）。只要 `GetInitialUrl()` 已经回答过
  nullopt，晚到的启动 URL 除了成为初始 URL，还会作为事件再发一次，不会丢。
- **无人监听时排队**：没有监听器时到达的 URL 进入有界队列（32 条，满了丢最旧的），第一个
  监听器加入时（`StartEventListening`）按顺序补发。监听器全部移除后再到达的 URL 同样排队。
  订阅本身不发初始事件，初始 URL 只能主动读。
- `Register(scheme)` 先按 RFC 3986 校验并转小写，再交平台。各平台的含义不同（§3），
  但 `false` 一律表示「调用后这个 scheme 的 URL 不会交给本应用」。

## 2. 为什么需要宿主接入

启动 URL 在大多数绑定加载 core **之前**就到了：Flutter 在引擎启动后、Dart 第一次调用时
才加载 `cnativeapi`；Android 的 intent 和 iOS 的 `openURL` 交给宿主拥有的 Activity /
delegate。所以 core 提供两条宿主通道，由一段在引擎启动时就运行的原生代码接上：

| 平台 | 宿主做什么 | core 侧 |
|---|---|---|
| macOS | 插件注册时 `dlopen` `cnativeapi.framework` 并调 `nativeapi_protocol_handler_host_attach()` | 构造单例时装 `kAEGetURL` 处理器；`NSApplicationDidFinishLaunchingNotification` 之前到达的 GURL 算启动 URL |
| iOS | 同上，再把 launch options / scene connection options 的 URL 以 `is_launch=1`、`openURL` / `openURLContexts` 以 `is_launch=0` 交给 `nativeapi_protocol_handler_host_deliver` | 只缓存与派发 |
| Android | Java 类 `dev.libnativeapi.nativeapi.ProtocolHandlerBridge` 调 `nativeAttach(Context)` / `nativeDeliver(String, boolean)` | `Java_dev_libnativeapi_nativeapi_ProtocolHandlerBridge_*` 两个 JNI 入口 |
| Windows | 第二个实例在 `wWinMain` 开头把 URL 转给正在运行的实例后退出 | 读命令行得到启动 URL；消息窗口接收转发 |

宿主约定是稳定 ABI，改名就是破坏性变更：

- C 符号（`protocol_handler_dispatch.h`，不进 `API_HEADERS`）：
  `void nativeapi_protocol_handler_host_attach(void)`、
  `void nativeapi_protocol_handler_host_deliver(const char* url, int is_launch)`。
  Apple / Android 的默认符号可见性让它们从 `cnativeapi` 导出。
- JNI：类名与两个 `static native` 方法签名见上表。
- Windows 转发：找类名为 `NativeAPI.ProtocolHandler`、标题为接收方完整可执行文件路径（小写）
  的 message-only 窗口，发 `WM_COPYDATA`，`dwData = 0x4E415055`（'NAPU'），负载是不带结尾
  NUL 的 UTF-8 URL。接收窗口建在拥有窗口、泵 Win32 消息的那个线程上（`FindUiThreadId`），
  并对 `WM_COPYDATA` 放开 UIPI 过滤。

Flutter 侧的接入在 leanflutter `protocol_handler` 0.3（保留 0.2 的监听器与流 API，底层是本模块）。
其中 `register()` 的返回值改为 `Future<bool>`；逐项对应与各平台配置见
[迁移指南](https://github.com/leanflutter/protocol_handler/blob/main/MIGRATION.md)。
iOS 的 application / scene delegate 都由 Flutter 弱引用；插件须调用
`registrar.publish(instance)` 让 engine 持有实例，否则注册结束后 URL 回调会丢失。
此生命周期修复尚未发布到 pub.dev，暂用插件 `main`（见其 CHANGELOG 的 Unreleased）。
其他宿主（原生 C++ 应用）在 macOS 上于应用完成启动前调 `ProtocolHandler::GetInstance()` 即可，
在移动端自行实现上表的转发。

## 3. 平台行为

| 平台 | `IsSupported()` | `Register()` | URL 来源 |
|---|---|---|---|
| macOS | true | 检查 `CFBundleURLTypes` 是否声明；声明了就 `LSRegisterURL` 并返回 true，不改用户的默认处理程序 | `kAEGetURL` Apple event |
| Windows | true | 写 `HKCU\Software\Classes\<scheme>`（`URL Protocol`、`DefaultIcon`、`shell\open\command = "<exe>" "%1"`），`SHChangeNotify`；MSIX 包内返回 false，由清单的 `uap:Protocol` 声明 | 命令行中第一个 scheme 至少两个字符的参数（排除 `C:\…` 路径）；`WM_COPYDATA` |
| Android | 宿主接入后 true | `PackageManager.queryIntentActivities` 查本包是否有该 scheme 的 VIEW 过滤器 | 宿主转发的 VIEW intent |
| iOS | 宿主接入后 true | 检查 `CFBundleURLTypes` | 宿主转发的 app / scene delegate 回调 |
| Linux、OpenHarmony、WASM | false | false | 无 |

Android 的 Context 优先取桥接类给的，其次取 NativeActivity 宿主 `android::Initialize` 的。

## 4. 绑定

C ABI 与各语言绑定由 codegen 生成：`register`、`getInitialUrl`（可空字符串）与事件监听。
Dart 的 `protocol_handler` 门面在其上提供 `ProtocolListener`、`listeners` / `hasListeners`、
`onUrlReceived` 流；流与监听器共用一个原生监听，最后一个取消时退订，之后到达的 URL 回到 core 排队。

## 5. 未决

- **Linux**：桌面文件的 `MimeType=x-scheme-handler/<scheme>` 与单实例转发（D-Bus）都未做；
  `protocol_handler` 原本也不支持 Linux。
- **注销**：没有 `Unregister`。源插件也没有；Windows 的注册表项留在 HKCU。
- **Android 冷启动去重**：插件以 intent 对象身份和 `FLAG_ACTIVITY_LAUNCHED_FROM_HISTORY`
  避免最近任务重开时重放启动 intent；进程被杀后系统恢复 Activity 的情形未验证。

## 6. 验证

- `core/tests/protocol_handler_test.cpp`：经宿主通道验证初始 URL、晚到启动 URL、32 条队列
  溢出与顺序、监听器移除后排队与补发、scheme 校验。
- `protocol_handler` 的 `flutter test`：监听器的增删与快照、`getInitialUrl`、非法 scheme。
- 2026-11-14 的四平台系统 URL 验证及边界见
  [验证报告](../tools/gui/protocol_handler_verification.md)，复现入口为
  `tools/gui/verify_protocol_handler.py`。通过系统投递验证普通/冷/热启动、编码 URL、重复
  激活、对象监听器与流、取消订阅、队列补发和初始 URL 稳定性；不使用宿主桥模拟入站事件。
- Windows 是未打包应用；MSIX 配置已记录，打包激活未实测。移动端使用模拟器，
  不等同于物理设备或所有生命周期场景的验证。

## 7. 检查单

- [ ] 改宿主约定（C 符号、JNI 类、Windows 窗口类 / `dwData`）时同步改插件，并在两边的
      CHANGELOG 标为破坏性变更。
- [ ] 新平台的 URL 来源区分「启动」与「运行中」，并在单例构造时就开始捕获。
- [ ] 晚到的启动 URL 不能丢：保持 §1 的「已回答 nullopt 则同时发事件」规则。
