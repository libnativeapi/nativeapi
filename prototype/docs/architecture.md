# Prototype 架构规则

本文件适用于 `prototype/` 下的开发与维护。

## 定位与边界

- Prototype（`prototype/`，基于 Storybook）承载 nativeapi 各示例应用（`examples/`）的界面原型、
  组件展示，使用本地示例数据与模拟行为。原型确定界面后，再由各绑定的示例按它实现。
- 一个原型对应一个示例，与 `examples/<binding>_<name>_example` 中去掉绑定前缀与 `_example` 后的
  `<name>` 同名（`tray_icon` → `src/examples/tray-icon/`）。各绑定的同名示例共用这一份原型。
- 不调用 nativeapi，不依赖任何绑定或 `core/`；原型描述示例的界面与它用到的 API，而不是调用它。
  涉及的 API 名称以 `core/src/` 的公开头文件为准。
- 控件来自 [DazzUI](https://github.com/dazzlabs/dazzui)：以 git 依赖 `@dazzlabs/dazzui`
  （DazzUI 的 `packages/js/dazzui`）引入，固定到某个提交，安装时由其 `prepare` 脚本构建；
  `pnpm-workspace.yaml` 的 `allowBuilds` 中必须有同一提交的条目，pnpm 才会执行该脚本。需要新控件或修改控件时
  改 DazzUI，再同时更新两处提交并运行 `pnpm install`。所有取值来自 DazzUI 的 token
  （`@dazzlabs/dazzui/style.css`），不写字面量。
- 主题与明暗是两个独立的工具栏选项：主题取 DazzUI 的 `data-theme`（DazzUI 自有系列、各桌面、Omarchy 各主题），
  明暗对所有主题生效。
- 仓库根目录是 npm workspace，不包含 `prototype/`；原型有自己的 `pnpm-workspace.yaml`，用 pnpm 管理。

## 目录职责

```text
prototype/
├── .storybook/                 # Story 扫描、全局装饰器、主题工具栏与排序
├── docs/
│   └── architecture.md         # 本文件
├── stories/                    # Introduction 文档
└── src/
    ├── styles/
    │   ├── globals.css         # 全局样式（只读 DazzUI token）
    │   └── themes.ts           # 主题与明暗外观的列表与切换逻辑
    ├── components/             # 跨示例共享组件及其 Story、局部样式（按需创建）
    └── examples/
        └── <example>/
            ├── <example>-view.tsx
            ├── <example>-view.stories.tsx
            ├── <example>-view.css
            ├── components/     # 示例内部组件及其 Story、局部样式
            ├── data.ts         # 静态示例数据
            ├── types.ts        # 示例内共享类型
            ├── index.ts        # 示例对外导出
            └── tray.ts         # 模拟行为，按具体职责命名
```

树中文件按需创建，不为尚不存在的职责建立空目录或空文件。

- 一个示例的完整界面由它的 `<Example>View` 承载，在侧边栏中排在该示例分组的第一项。
- 窗口用 DazzUI 的 `WindowFrame`（应用窗口再配 `WindowBody`、`WindowMain` 等），不自己画窗口框。
  `platform` 取 `windowPlatformOf(style)`（`src/styles/themes.ts`），Story 从 `context.globals.style`
  读到当前 Style，这样窗口控件跟随工具栏的桌面。
- `src/components/` 中的组件不依赖具体示例。示例内部组件放在所属示例的 `components/`，
  跨示例复用后再提升到 `src/components/`。
- Story 与对应组件就近存放；`stories/` 只放 Introduction 与贯穿多个示例的内容。

## 共享组件与界面规范

所有示例遵循同一套界面语言，由 `src/components/` 中的共享组件承载（各目录经 `index.ts` 导入）：

| 组件 | 职责 |
| --- | --- |
| `desktop-stage/` | `DesktopStage`：工具栏所选桌面（macOS 菜单栏、Windows 任务栏、GNOME、KDE、Omarchy 的 Waybar）与壁纸；`layout="center"` 居中摆一个窗口，`layout="free"` 由 `DesktopWindow` 按坐标摆放多个窗口；`usePointerDrag` 模拟窗口管理器接管的拖动 |
| `example-window/` | `ExampleWindow`：示例自己的应用窗口——全高侧栏、窗格标题带（标题、`SegmentedControl` 分页、尾部操作）、内容与底栏 |
| `event-bar/` | `EventBar` 与 `useEventLog`：底栏显示最近事件与调用日志 |
| `panel/` | 分页内容的紧凑设置列（`Panel`、`PanelPreferences` 等） |
| `read-back/` | `ReadBack`：调用后 getter 的读回，而不是界面输入的回显 |
| `platform.ts`、`use-now.ts` | 由桌面得出操作系统、栏的位置；按间隔刷新的时钟 |

- 示例站在 `DesktopStage` 上：API 的效果在桌面上实时可见（窗口移动、缩放、隐藏，对话框弹出，菜单打开，托盘或 Dock 变化，系统交给的应用打开）。
- 示例窗口用 `ExampleWindow`；以窗口本身为主题的示例（标签页、拖动区域、标题栏、异形窗口、浮动工具栏等）直接用 `WindowFrame` 画出主题窗口，控制面板居次。
- 平台差异如实呈现：不支持的调用禁用并注明原因，或返回 false 并记入日志。
- 全局样式和主题放入 `styles/`；组件与示例视图的局部样式留在组件旁。

## 命名

- 文件和目录使用小写 kebab-case，如 `tray-icon-view.tsx`、`menu-row.stories.tsx`。
- React 组件使用 PascalCase，示例入口统一为 `<Example>View`，不使用 `Screen` 后缀。
- Hook 使用 `useXxx`，对应文件使用 `use-xxx.ts`。
- 示例数据统一使用示例内的 `data.ts`，不使用 `fixtures.ts`，不集中到顶层数据目录。
- 模拟行为不是静态数据。按行为命名文件，函数使用 `simulateXxx` 等明确名称，不建立顶层 `mocks/`。

## 依赖与状态

- 依赖方向为：示例视图组合内部组件与共享组件，二者都使用 DazzUI。
- 使用 DazzUI 时从包入口 `@dazzlabs/dazzui` 导入，不深入导入其内部文件。
- 共享组件不反向依赖具体示例；示例不导入其他示例的内部实现。
- 外部使用示例组件和类型时通过示例的 `index.ts`；Story 可直接引用示例的 `data.ts` 和模拟行为。
- `index.ts` 只导出需要对外使用的组件与类型，不导出 Story。
- 示例数据在交互中作为初始值使用，不直接修改共享导出的数据。

## Storybook 侧边栏

Story 的 `title` 明确指定展示层级，不依赖文件路径自动生成。

| 内容 | title 约定 | 示例 |
| --- | --- | --- |
| 示例视图：完整、可交互的界面 | `Examples/<Example>/<Example> View` | `Examples/Tray Icon/Tray Icon View` |
| 组件：界面的一部分，逐个状态展示 | `Examples/<Example>/<Component>` | `Examples/Tray Icon/Menu Row` |
| 共享组件 | `Shared/<Component>` | `Shared/Event Log` |

- 示例文件夹名取示例目录名的可读形式（`tray-icon` → `Tray Icon`）。
- 在 `.storybook/preview.tsx` 中维护排序：`Introduction`、`Examples`（按 `examples/` 中的名称顺序）、`Shared`；
  示例内依次为视图、组件。
- Story 使用唯一、稳定的显式 `id`。单纯移动文件或调整分组时保留 ID。
- Story 应展示有意义的默认、空、运行或失败状态；仅为实际存在的状态添加场景。
- 交互演示使用本地状态和模拟回调；固定场景优先通过 args 配置。

## 修改与验证

- 移动或重命名时同步更新 import、`index.ts`、类型名、CSS 类名、Story title、排序及相关文档。
- 代码变更后在 `prototype/` 下运行：

  ```sh
  pnpm typecheck
  pnpm build
  ```

- 修改 Story 注册或分组后，检查构建产物 `prototype/storybook-static/index.json` 中的 title 与 ID。
- 不提交 `storybook-static/`、调试日志等生成产物。
