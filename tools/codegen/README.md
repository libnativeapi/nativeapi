# codegen

从 C++ 头文件自动生成 **C ABI** 以及 **Rust / Dart / C# / JS / Python / Go** 各端 FFI 绑定的代码生成工具。

## 工作流程

```
┌──────────────┐   ┌──────────────────────┐   ┌────────────┐   ┌──────────────────┐
│  C++ Header  │──→│     codegen-capi     │──→│  IR (JSON) │──→│ codegen-bindings │
│   (core/src) │   │  libclang 解析 + IR   │   │            │   │   six bindings   │
│              │   │  生成 C ABI + 伞头文件 │   │            │   │                  │
└──────────────┘   └──────────────────────┘   └────────────┘   └──────────────────┘
```

只有 `codegen-capi` 依赖 libclang；`codegen-bindings` 消费它导出的 IR JSON。

生成 Dart bindings 时需要 `dart` 在 PATH 中。生成器在写入和只读校验前都执行
`dart format`，使用与 Flutter 包一致的 Dart 3.9 语言版本，避免生成后再次触发
Dart CI 的格式检查失败。Windows 上请将 Flutter SDK 的 `bin/cache/dart-sdk/bin`
加入 PATH。

C++ 枚举项转换成 Dart 的 lowerCamelCase 后若是保留字，会加尾部下划线，
例如 `WindowCornerPreference::Default` 对应 `WindowCornerPreference.default_`；
声明、数值转换与默认回退都使用这个名字。

## Crate 布局

```
tools/codegen/
├── Cargo.toml    # Cargo workspace
├── shared/       # codegen-shared：parser（libclang）、IR 模型与序列化、
│                 #   naming（命名工具 + 跨头文件类型索引）、文件写入/校验、API_HEADERS 清单
├── capi/         # codegen-capi（bin）：C API（.h + .cpp）与 umbrella header 生成
└── bindings/     # codegen-bindings（bin）：Rust / Dart / C# / JS / Python / Go FFI 绑定生成
```

## 运行

日常统一通过 workspace 根目录的 `./codegen` 脚本执行（它负责按 workspace 布局
传路径并串联两个 generator）：

```bash
./codegen                    # 全量：C ABI → 所有绑定
./codegen capi               # 只生成 C ABI
./codegen bindings           # 只生成绑定（--lang rust,dart,csharp,js,python,go 可选子集）
./codegen check              # 只读校验，生成物过期时非零退出（给 CI 用）
./codegen sync               # 改完 core 后的一键联动（见下）
./codegen --dump-ir ir.json  # 把中间 IR 保留到指定路径调试
```

### `./codegen sync`：core 改动的一键下游联动

改了 core 的头文件后执行，按序完成：

1. 全量重新生成（C ABI + 所有绑定）
2. 提交 core（消息用 `-m` 指定，默认 `Update API`）
3. rust 额外重跑 bindgen 刷新 `bindings/rust/cnativeapi/src/bindings.rs`；dart 额外运行 ffigen
   刷新 `bindings/dart/cnativeapi/lib/src/bindings_generated.dart`（单独执行：`./codegen ffigen`）
4. 提交 workspace，消息为 `Sync with core <sha>`：core 指针，以及 `bindings/`
   下的全部改动（所有 binding 都在 workspace 仓库里）

默认只提交不推送；`--push` 会按 core → workspace 的顺序推送
（保证远端的 submodule 指针不悬空）。任一仓库处于 detached HEAD 会直接报错。
bindgen / dart 未安装时对应步骤跳过并告警。

所有 binding 都直接基于仓库里的 `core/` 构建，没有各自的 core 副本需要更新。

生成产物：

1. C ABI → `core/src/capi/`
2. umbrella header → `core/include/nativeapi.h`
3. Rust FFI → `bindings/rust/nativeapi/src/`
4. Dart FFI → `bindings/dart/nativeapi/lib/src/`
5. C# FFI → `bindings/csharp/src/CNativeAPI/generated/`（raw 层）+ `bindings/csharp/src/NativeAPI/`（公开层）
6. JS / TS → `bindings/js/src/` + `bindings/js/lib/`
7. Python → `bindings/python/nativeapi/`
8. Go → `bindings/go/nativeapi.gen.go` + `bindings/go/bridge.gen.h`

生成或校验 Go 绑定需要 Go SDK（`gofmt`）在 PATH 中；写入和只读校验前都会格式化。
Go 的公开层遵循 Go 命名与错误约定：getter 去掉 `Get`，`ToString` 转为 `String`，
参数使用 lowerCamelCase，构造函数重载只保留区分参数；构造函数/工厂返回 `(*T, error)`，
操作型 `bool` 返回值转为 `error`，`Is` / `Has` / `Can` / `Contains` 判断保留 `bool`。
判断前缀必须按单词匹配，`Cancel` 是操作，不是 `Can` 判断。对象 getter 的 `nil` 仍表示缺失；
C ABI 没有详细原因的失败只包装 `ErrOperationFailed`，不猜测平台错误。
单例生成导出的无状态值及其方法，例如 `DisplayManager.All()`、`Application.Run()`；
实现类型保持私有，调用不需要创建或释放 Manager。
Go 的原生构建、主线程约束及示例见 [bindings/go/README.md](../../bindings/go/README.md)。

`core/src/capi/` 的公开 ABI 由本工具生成，手写支持层为
`string_utils_c.{h,cpp}`（字符串 / 值容器）、`user_data.h`（回调资源）和
`event_delivery.h`（异步交付负载与隐式请求投票）。生成物首行都带
`// AUTO-GENERATED. DO NOT EDIT.`，改头文件再重新生成，不要改生成物。
没有该 banner 的已存在文件永远不会被覆盖（保护手写代码）。

要纳入生成的头文件清单 `API_HEADERS` 定义在 `shared/src/lib.rs`。

## 生成之后（必要的手工步骤）

C ABI 是所有绑定的地基，新增符号后需要同步下游：

1. **类型标签**：新增的句柄类型需要在 `core/src/foundation/id_allocator.h`
   的 `IdTypeTag<T>` 注册表里追加一个编号（**只追加，不改已有编号**）。漏了会在
   编译期报错，不会静默出问题。
2. **core 指针**：binding 直接引用仓库里的 `core/`，提交 core 后只需在 workspace
   里提交新的 `core` 指针（`./codegen sync` 会做）。发布包自带的 `cxx_impl/` 只在
   release workflow 里临时生成，不入库
3. **Rust raw FFI**：`bindings/rust/cnativeapi/src/bindings.rs` 由 bindgen 生成并入库，
   新增 C 符号后需重新生成（在 workspace 根目录执行）：

   ```bash
   bindgen core/include/nativeapi.h \
     --allowlist-function 'native_.*|free_c_str' --allowlist-type 'native_.*' \
     --allowlist-var 'NATIVE_.*' \
     --with-derive-default --no-layout-tests --no-prepend-enum-name \
     --raw-line '#![allow(non_upper_case_globals)]' \
     --raw-line '#![allow(non_camel_case_types)]' \
     --raw-line '#![allow(non_snake_case)]' \
     -o bindings/rust/cnativeapi/src/bindings.rs \
     -- -x c -isysroot "$(xcrun --show-sdk-path)" -Icore/src -Icore/include
   ```

4. **模块声明**：Rust 侧新增的 `xxx.rs` 需要在 `bindings/rust/nativeapi/src/lib.rs` 中 `pub mod`
