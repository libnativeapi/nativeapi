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
- 过滤方式：
  - `// bindgen:ignore`：显式忽略
  - 可选白名单/黑名单（按正则匹配函数/类型名）
  - 可选排除目录（例如 `capi`）
- 只处理（MVP）：
  - `extern "C"` 函数
  - C++ class/struct 的 public 方法（不含继承、多态、异常语义）
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

- IR 输出为 `path -> IRFile` 的 map（不再包含 `headers` 节点）
- `IRFile` 结构包含：
  - types
  - enums
  - functions
  - classes
  - constants
  - aliases
- `source_path`：结构体/枚举/函数/别名/常量/类/方法/字段所在头文件路径（如 `src/foundation/color.h`）

### IR JSON 示例（最小）

```json
{
  "src/foundation/geometry.h": {
    "types": [
      {
        "kind": "struct",
        "name": "Point",
        "source_path": "src/foundation/geometry.h",
        "fields": [
          {"name": "x", "type": {"kind": "float32"}, "source_path": "src/foundation/geometry.h"},
          {"name": "y", "type": {"kind": "float32"}, "source_path": "src/foundation/geometry.h"}
        ]
      }
    ],
    "functions": [
      {
        "name": "na_distance",
        "source_path": "src/foundation/geometry.h",
        "return_type": {"kind": "float32"},
        "params": [
          {"name": "a", "type": {"kind": "pointer", "to": {"kind": "struct", "name": "Point"}}, "nullable": false},
          {"name": "b", "type": {"kind": "pointer", "to": {"kind": "struct", "name": "Point"}}, "nullable": false}
        ]
      }
    ],
    "enums": [],
    "classes": [],
    "constants": [],
    "aliases": []
  },
  "src/foundation/error.h": {
    "types": [],
    "enums": [],
    "functions": [],
    "classes": [],
    "constants": [
      {"name": "NA_OK", "type": {"kind": "int32"}, "value": 0, "source_path": "src/foundation/error.h"}
    ],
    "aliases": []
  },
  "src/foundation/types.h": {
    "types": [],
    "enums": [],
    "functions": [],
    "classes": [],
    "constants": [],
    "aliases": [
      {"name": "NAHandle", "target": {"kind": "uintptr"}, "source_path": "src/foundation/types.h"}
    ]
  }
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
        *.j2           # 全局模板（渲染一次）
        file/
          *.j2         # 按文件模板（每个 IR 文件渲染一次）
        partials/
          type.j2      # 结构体片段模板
          enum.j2      # 枚举片段模板
          function.j2  # 函数片段模板
          class.j2     # 类片段模板
          constant.j2  # 常量片段模板
          alias.j2     # 类型别名片段模板
    README.md
```

**模板目录结构说明**

- `template/*.j2` - 全局模板，只渲染一次，生成单个输出文件
- `template/file/*.j2` - 按文件模板，为每个 IR 源文件渲染一次
- `template/partials/*.j2` - 片段模板（宏），可被其他模板导入使用

**使用 partials 片段模板**

片段模板定义了可复用的宏，用于渲染单个 IR 元素：

```jinja2
{# 在 file/*.j2 中导入并使用 partials #}
{% from 'partials/type.j2' import render_type %}
{% from 'partials/enum.j2' import render_enum %}
{% from 'partials/class.j2' import render_class %}

{% for item in types %}
{{ render_type(item) }}
{% endfor %}
```

**语言映射配置（示例）**

语言相关配置统一写在 `config.yaml` 的 `mapping` 字段中，包括类型映射：

```yaml
mapping:
  language: example
  
  # 类型映射配置
  types:
    void: void
    bool: bool
    int: int
    float: double
    "void*": "Pointer<Void>"
    "const char*": "Pointer<Utf8>"
  
  pointer_format: "Pointer<{inner}>"
  void_pointer_type: "Pointer<Void>"
  const_char_pointer_type: "Pointer<Utf8>"
  
  # 自定义选项（模板可访问）
  options:
    enum_as_int: true
    naming_function: camel
```

**类型映射配置**

类型映射配置直接写在 `mapping` 字段中，用于将 C/C++ 类型映射到目标语言类型。支持以下配置项：

```yaml
mapping:
  language: dart
  
  # 直接类型名映射: C 类型名 -> 目标语言类型
  types:
    # 基本类型
    void: void
    bool: bool
    int: int
    float: double
    double: double
    
    # 固定宽度整数
    int8_t: int
    int16_t: int
    int32_t: int
    int64_t: int
    uint8_t: int
    uint16_t: int
    uint32_t: int
    uint64_t: int
    size_t: int
    
    # 指针类型快捷映射
    "void*": "Pointer<Void>"
    "char*": "Pointer<Utf8>"
    "const char*": "Pointer<Utf8>"
    
    # 函数指针
    function_pointer: "Pointer<NativeFunction>"
  
  # 指针类型格式，使用 {inner} 作为占位符
  # 例如 Dart FFI: "Pointer<{inner}>"
  pointer_format: "Pointer<{inner}>"
  
  # const 指针类型格式
  const_pointer_format: "Pointer<{inner}>"
  
  # 数组类型格式，使用 {element} 和 {length} 作为占位符
  array_format: "Array<{element}>"
  
  # 引用类型格式
  reference_format: "{inner}"
  
  # 未找到映射时的默认类型（可选）
  # default_type: dynamic
  
  # 未找到映射时是否保留原始类型名
  passthrough_unknown: true
  
  # 映射后类型名的前缀/后缀（可选）
  # type_prefix: ""
  # type_suffix: ""
  
  # void* 的特殊映射（常用作不透明句柄）
  void_pointer_type: "Pointer<Void>"
  
  # const char* 的特殊映射（常用作字符串）
  const_char_pointer_type: "Pointer<Utf8>"
```

**类型映射过滤器**

在模板中可使用以下过滤器处理类型映射：

| 过滤器 | 说明 | 示例 |
|--------|------|------|
| `map_type` | 将 IRType 映射为目标语言类型字符串 | `{{ param.type \| map_type }}` |
| `map_type_name` | 简单类型名查找映射 | `{{ "int" \| map_type_name }}` |
| `format_type` | 自定义格式的类型映射 | `{{ type \| format_type(pointer_fmt="*{inner}") }}` |
| `is_pointer_type` | 检查是否为指针类型 | `{% if type \| is_pointer_type %}` |
| `is_array_type` | 检查是否为数组类型 | `{% if type \| is_array_type %}` |
| `is_void_type` | 检查是否为 void 类型 | `{% if type \| is_void_type %}` |
| `get_inner_type` | 获取指针/引用的内部类型 | `{{ type \| get_inner_type \| map_type }}` |
| `get_element_type` | 获取数组的元素类型 | `{{ type \| get_element_type \| map_type }}` |

**模板中使用类型映射示例**

```jinja2
{# 函数参数和返回值类型映射 #}
fn {{ item.name }}(
{%- for param in item.params %}
    {{ param.name }}: {{ param.type | map_type }}{% if not loop.last %},{% endif %}
{%- endfor %}
) -> {{ item.return_type | map_type }};

{# 结构体字段类型映射 #}
struct {{ item.name }} {
{%- for field in item.fields %}
    {{ field.name }}: {{ field.type | map_type }};
{%- endfor %}
}

{# 条件处理指针类型 #}
{% if param.type | is_pointer_type %}
// This is a pointer parameter
{% endif %}
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

- `module`: 当前模块信息（含 `files`）
- `files`: path -> IRFile 的 map
- `file_paths`: 排序后的文件路径列表
- `types`: 结构体/枚举/别名列表（已扁平化并排序）
- `functions`: 函数列表（已过滤并扁平化）
- `classes`: C++ 类/struct 列表（public methods）
- `constants`: 常量列表（扁平化）
- `mapping`: 语言映射配置（MappingConfig 对象，包含 language、types、pointer_format 等）

**按文件模板渲染规则**

`template/file/*.j2` 中的模板会按 IR 的 `file_paths` 逐文件渲染：

- 渲染时 `types/enums/functions/classes/constants/aliases` 仅包含当前文件的内容
- 额外提供全量列表：`all_types/all_enums/all_functions/all_classes/all_constants/all_aliases`
- `file_path` 与 `file` 提供当前文件信息

**输出路径规则**

输出路径由 IR 文件路径和模板名称决定：

```
out/<ir_dir>/<ir_stem>.<template_stem>
```

示例：
- IR 文件：`src/foundation/geometry.h` + 模板：`file/bindings.j2` → `out/src/foundation/geometry.bindings`
- IR 文件：`src/window.h` + 模板：`file/dart.j2` → `out/src/window.dart`
- IR 文件：`src/menu.h` + 模板：`file/rs.j2` → `out/src/menu.rs`

**优点**

- 新增语言只需新增模板 + 映射配置
- 语言差异集中可见，维护成本低
- 测试可按模板/映射分层
- 支持一比一文件映射，保持源码结构

### Templates (External)

- 本仓库仅保留示例模板用于验证
- 实际多语言绑定由使用方在自己的仓库提供模板与映射配置

## 配置与注解

### YAML 配置文件

```yaml
clang_flags:
  - -std=c11
mapping:
  language: example
filters:
  allowlist_regex: []
  denylist_regex: []
  exclude_dirs:
    - capi
```

说明：`entry_headers` 将自动从 `src/` 目录下收集所有 `.h`（忽略 `platform/`），`include_paths` 也会默认指向 `src/`，无需在配置中手动列出。模板目录默认读取 `config.yaml` 同路径下的 `template/`。

### 注解建议（可扩展）

- 使用宏或注释标注 API，如：
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

### 生成后格式化（Post Formatters）

可在 `mapping.options.formatters` 配置生成完成后的格式化命令：

```yaml
mapping:
  options:
    formatters:
      - name: swiftformat-generated
        cmd: ["swiftformat", "{out_dir}/src"]
        continue_on_error: true
      - name: dart-format-generated
        cmd: ["dart", "format", "{out_dir}/src"]
        continue_on_error: true
```

说明：

- `cmd`：必填，命令与参数数组（不走 shell）。
- `continue_on_error`：可选，默认 `true`。失败时仅 warning 并继续。
- `enabled`：可选，默认 `true`，可按项关闭 formatter。
- 占位符支持：
  - `{out_dir}`：`--out` 指定的输出目录（绝对路径）
  - `{config_dir}`：`config.yaml` 所在目录
  - `{project_dir}`：执行 bindgen 时的当前工作目录

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

- **宏复杂度高**：MVP 仅处理简单宏
- **C++ 语义**：默认 `extern "C"` 入口
- **跨平台 ABI**：需在生成器中区分平台（Windows/Linux/macOS）

## 内置过滤器

### 命名转换过滤器

| 过滤器 | 说明 | 示例 |
|--------|------|------|
| `snake_case` | 转换为 snake_case | `{{ "MyClass" \| snake_case }}` → `my_class` |
| `camel_case` | 转换为 camelCase | `{{ "my_function" \| camel_case }}` → `myFunction` |
| `pascal_case` | 转换为 PascalCase | `{{ "my_class" \| pascal_case }}` → `MyClass` |
| `screaming_snake_case` | 转换为 SCREAMING_SNAKE_CASE | `{{ "myConst" \| screaming_snake_case }}` → `MY_CONST` |
| `kebab_case` | 转换为 kebab-case | `{{ "MyClass" \| kebab_case }}` → `my-class` |
| `strip_prefix` | 移除前缀 | `{{ "na_init" \| strip_prefix("na_") }}` → `init` |
| `strip_suffix` | 移除后缀 | `{{ "WindowHandle" \| strip_suffix("Handle") }}` → `Window` |
| `add_prefix` | 添加前缀 | `{{ "Window" \| add_prefix("NA") }}` → `NAWindow` |
| `add_suffix` | 添加后缀 | `{{ "Window" \| add_suffix("Impl") }}` → `WindowImpl` |

### 类型映射过滤器

| 过滤器 | 说明 | 示例 |
|--------|------|------|
| `map_type` | IRType 映射为目标类型 | `{{ param.type \| map_type }}` |
| `map_type_name` | 类型名直接映射 | `{{ "int32_t" \| map_type_name }}` |
| `format_type` | 自定义格式映射 | `{{ type \| format_type(pointer_fmt="*{inner}") }}` |
| `is_pointer_type` | 是否指针类型 | `{% if type \| is_pointer_type %}` |
| `is_array_type` | 是否数组类型 | `{% if type \| is_array_type %}` |
| `is_void_type` | 是否 void 类型 | `{% if type \| is_void_type %}` |
| `get_inner_type` | 获取指针内部类型 | `{{ type \| get_inner_type }}` |
| `get_element_type` | 获取数组元素类型 | `{{ type \| get_element_type }}` |

## 下一步

- 补充更多模板示例
- 扩展 C++ 类方法语义（如继承信息、构造/析构、const 重载等）
