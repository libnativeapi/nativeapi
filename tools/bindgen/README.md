# Bindgen 设计文档（Python）

本目录目标：使用 Python 实现一个面向 C++ Headers 的 bindgen，自动解析头文件并生成多语言绑定。各语言 API **统一调用 C API**，再由 C API 调用 C++ 实现，确保 ABI 稳定与可移植性。模板不内置于本仓库，由使用方在自己的仓库提供；本仓库仅保留示例模板用于验证与参考。

本文档说明核心需求、架构、数据模型、生成流程、CLI、可扩展性与里程碑计划。

## 目标与非目标

**目标**

- 解析 C++ 公开 API（以头文件为主）
- 生成多语言 bindings（模板由使用方仓库提供）
- 所有语言绑定统一调用 C API 层（避免直接绑定 C++ ABI）
- 统一的中间表示（IR），减少多语言实现成本
- 可配置、可扩展、可测试
- 与现有仓库结构和 CMake 构建保持良好配合

**非目标（第一阶段）**

- 不做完整 C++ 语义分析（模板深度推导、宏复杂展开等）
- 不自动绑定 C++ class 的复杂继承、多态或异常（需经 C API 封装）
- 不直接生成 GUI/业务层封装，仅做 FFI 级别桥接

## 总体架构

```text
C++ Headers
      |
      v
   Parser (libclang/clang.cindex)
      |
      v
   AST Normalizer
      |
      v
   IR (统一中间表示)
      |
      v
 Code Generator (通用生成器)
      |
      |
      v
  External Templates
      |
      v
  Bindings Output
                |
                v
            C API (extern "C")
                |
                v
          C++ Implementation
```

- **Parser**：基于 `libclang`/`clang.cindex` 解析头文件，生成 AST
- **Normalizer**：将 AST 规约为可生成的 IR（类型、函数、常量、结构体）
- **IR**：语言无关的描述结构
- **Code Generator**：通用生成器，负责加载模板与语言映射配置并输出 bindings
- **Templates**：使用方自定义模板 + 映射配置（类型映射、命名风格、ABI 约定）
- **C API**：稳定 ABI 层，`extern "C"` 函数
- **C++ Implementation**：真实实现层，仅由 C API 访问

## 输入与约束

- 入口：一组 C++ 头文件 + include paths + compile flags
- 建议约定宏：用于筛选可导出 API（例如 `NATIVEAPI_EXPORT`）
- 过滤方式：
  - `export_macro`：只导出带宏标记的符号
  - `// bindgen:ignore`：显式忽略
  - 可选白名单/黑名单（按正则匹配函数/类型名）
- 只处理（MVP）：
  - 可导出的 C API 函数（`extern "C"`）
  - `struct` + POD 类型
  - `enum` 常量
  - `#define` 的简单常量
- 入口头文件推荐指定“主头文件”（统一 include 顺序，避免重复解析）

**约定**：C++ 头文件中的导出 API，必须能映射为 C API（如 `extern "C"` wrapper），bindgen 只对 C API 进行绑定生成。

## IR 设计

### 类型系统（IRType）

- 基础类型：`void`, `bool`, `int8/16/32/64`, `uint8/16/32/64`, `float32/64`
- 扩展基础类型：`size_t`, `ssize_t`, `intptr`, `uintptr`
- 类型别名：`typedef/using` 归一化为 `alias`
- 指针类型：`pointer<T>`
- 数组类型：`array<T, N>`
- 结构体：`struct { fields }`
- 枚举：`enum { values }`
- 字符串：`char*` 规范化为 `cstring`
- 函数指针：`fnptr(ret, params)`（MVP 可忽略，后续支持）
- 限定符：`const/volatile` 记录在 `qualifiers`
- C++ `enum class` 归一化为 `enum`（保留 `scoped` 标记）

### 函数（IRFunction）

- 名称、返回类型、参数列表
- 参数可包含 `in/out` 注解
- 支持 `nullable` 标注
- 可记录 `callconv` 与 `variadic`（默认不支持变参）

### 模块（IRModule）

- headers
- types
- functions
- constants
- aliases

### IR JSON 示例（最小）

```json
{
  "headers": ["include/nativeapi.h"],
  "types": [
    {
      "kind": "struct",
      "name": "Point",
      "fields": [
        {"name": "x", "type": {"kind": "float32"}},
        {"name": "y", "type": {"kind": "float32"}}
      ]
    }
  ],
  "functions": [
    {
      "name": "na_distance",
      "return_type": {"kind": "float32"},
      "params": [
        {"name": "a", "type": {"kind": "pointer", "to": {"kind": "struct", "name": "Point"}}, "nullable": false},
        {"name": "b", "type": {"kind": "pointer", "to": {"kind": "struct", "name": "Point"}}, "nullable": false}
      ]
    }
  ],
  "constants": [
    {"name": "NA_OK", "type": {"kind": "int32"}, "value": 0}
  ],
  "aliases": [
    {"name": "NAHandle", "target": {"kind": "uintptr"}}
  ]
}
```

## 代码生成策略

**绑定链路约束**

- 生成的 bindings **只调用 C API**，不直接依赖 C++ 符号
- C API 负责：参数规约、ABI 稳定、与 C++ 实现交互
- C++ 实现层可自由演进，但对外 ABI 需保持稳定

## 通用模板生成器设计

Generator 可以做成通用的模板渲染器，通过“语言模板 + 语言映射配置”驱动输出，不把语言差异硬编码在 Python 里。

**设计原则**

- 生成器只做：加载 IR → 选择语言 → 渲染模板 → 写文件
- 语言差异放在模板与映射配置（类型映射、命名风格、调用约定）
- 复杂逻辑尽量前置到 Normalizer（保证模板尽量“无脑”）
- 模板约定：模板依赖的上下文字段必须稳定并有文档

**目录结构（建议）**

```
tools/bindgen/
  parser.py            # 解析 C/C++ headers -> AST
  normalizer.py        # AST -> IR 规约
  ir/
    model.py           # IR 数据结构
    serializer.py      # IR JSON 读写
  codegen/
    generator.py       # 通用生成器入口
    context.py         # 模板上下文构建
  example/
    bindgen/
      config.yaml      # 示例配置
      template/
        example.txt.j2 # 示例目标语言模板（用于验证与参考）
    README.md
```

**语言映射配置（示例）**

语言相关配置统一写在 `config.yaml` 的 `mapping` 字段中：

```yaml
mapping:
  language: example
  conventions:
    enum_as_int: true
  naming:
    function: camel
```

**通用生成器伪代码**

```python
ir = load_ir(...)
lang = load_lang_config(...)
templates = load_templates(lang)
context = build_context(ir, lang)
render_to_files(templates, context, out_dir)
```

**模板上下文约定（建议）**

- `module`: 当前模块信息（headers, namespaces）
- `types`: 结构体/枚举/别名列表（已排序）
- `functions`: 函数列表（已过滤）
- `constants`: 常量列表
- `mapping`: 语言映射配置（types/conventions/naming）

**优点**

- 新增语言只需新增模板 + 映射配置
- 语言差异集中可见，维护成本低
- 测试可按模板/映射分层

### Templates (External)

- 本仓库仅保留示例模板用于验证
- 实际多语言绑定由使用方在自己的仓库提供模板与映射配置

## 配置与注解

### YAML 配置文件

```yaml
clang_flags:
  - -std=c11
  - -DNATIVEAPI_EXPORT
mapping:
  language: example
filters:
  export_macro: NATIVEAPI_EXPORT
  allowlist_regex: []
  denylist_regex: []
```

说明：`entry_headers` 将自动从 `src/` 目录下收集所有 `.h`（忽略 `platform/`），`include_paths` 也会默认指向 `src/`，无需在配置中手动列出。模板目录默认读取 `config.yaml` 同路径下的 `template/`。

### 注解建议（可扩展）

- 使用宏或注释标注 API，如：
  - `NATIVEAPI_EXPORT` 标记导出
  - `// bindgen:ignore` 跳过
  - `// bindgen:nullable` 标记可为空
  - `// bindgen:rename <NewName>` 重命名

## CLI 设计

```
cd tools/bindgen/example
PYTHONPATH=../.. python3 -m bindgen \
  --config bindgen/config.yaml \
  --dump-ir out/ir.json \
  --out out
```

示例运行后将使用 `bindgen/template` 中的模板，并把 IR 与生成结果都写到 `out`。

常用参数：

- `--config` 配置文件
- `--out` 输出目录
- `--dump-ir` 输出 IR JSON 便于调试

## 开发计划（里程碑）

1. **MVP**
   - 支持解析函数 + 基础类型
   - 生成示例模板输出
   - IR JSON dump
   - 最小可用示例（单头文件 -> bindings）

2. **扩展**
   - 支持 struct、enum
   - 按需扩展多语言模板

3. **增强**
   - 增加注解/宏过滤
   - 增加类型映射配置

4. **稳定化**
   - 兼容性测试
   - CI 脚本

## 关键技术点

- `libclang` 解析：需要确保 clang 能找到 include paths
- 类型映射：不同语言对 `size_t`, `bool` 的对应关系不一致
- ABI 兼容：必须保证 `repr(C)`/`ffi.Struct` 对齐一致

## 风险与规避

- **宏复杂度高**：MVP 仅处理简单宏与 export 宏
- **C++ 语义**：默认 `extern "C"` 入口
- **跨平台 ABI**：需在生成器中区分平台（Windows/Linux/macOS）

## 下一步

- 补充 `bindgen.yaml` 示例
- 实现 `parser.py` 与 `ir.py` 的最小骨架
- 选择模板引擎（如 `jinja2`）用于 codegen
