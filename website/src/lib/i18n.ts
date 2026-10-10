export const locales = ['en', 'ja', 'zh'] as const

export type Locale = (typeof locales)[number]

export const defaultLocale: Locale = 'en'

export const githubUrl = 'https://github.com/libnativeapi/nativeapi'

export const localeStorageKey = 'nativeapi-locale'

export function isLocale(value: string): value is Locale {
  return locales.includes(value as Locale)
}

export function localePath(locale: Locale) {
  return localizedPath(locale, '/')
}

export function localizedPath(locale: Locale, pathname: string) {
  const normalizedPath = pathname.startsWith('/') ? pathname : `/${pathname}`
  if (locale === defaultLocale) return normalizedPath
  return normalizedPath === '/' ? `/${locale}` : `/${locale}${normalizedPath}`
}

export function localeFromPath(pathname: string): Locale {
  const segment = pathname.split('/')[1]
  return isLocale(segment) ? segment : defaultLocale
}

const sharedLanguages = {
  en: 'English',
  ja: '日本語',
  zh: '简体中文',
} as const

const messages = {
  en: {
    languageName: 'English',
    languages: sharedLanguages,
    metadata: {
      title: 'nativeapi - Native desktop APIs for every language',
      description:
        'nativeapi gives Dart/Flutter, Rust, C#, JavaScript/TypeScript, Python and Go one native API for windows, tray icons, menus, displays, keyboard, dialogs and storage, built on a single C++ core.',
    },
    header: {
      menu: 'Menu',
      openMenu: 'Open menu',
      closeMenu: 'Close menu',
      home: 'Home',
      features: 'Features',
      docs: 'Docs',
      api: 'API',
      install: 'Get started',
      language: 'Language',
    },
    home: {
      badge: 'Open source · Work in progress',
      heroLine1: 'Native desktop APIs',
      heroLine2: 'for ',
      heroHighlight: 'every language',
      heroDescription:
        'Windows, tray icons, menus, displays, keyboard, dialogs and storage — implemented once in C++ on each platform, with generated bindings for Dart/Flutter, Rust, C#, JavaScript/TypeScript, Python and Go.',
      installCta: 'Get started',
      readDocs: 'Read the docs',
      heroNote: 'macOS, Windows and Linux — plus Android and iOS from Dart',
      trustLabels: ['Language bindings', 'Platforms', 'GitHub Stars', 'License'],
      problem:
        'Every UI toolkit ends up rewriting the same platform glue: window chrome, tray icons, native menus, multi-monitor geometry — once per language, once per OS.',
      problemContinuation:
        'The behaviour drifts apart, every bug is fixed three times, and the next binding starts from zero.',
      promise: 'nativeapi writes it once, in C++, and generates the rest.',
      featuresEyebrow: 'What it covers',
      featuresTitle: 'The native layer your framework is missing',
      featuresDescription:
        'The parts of a desktop app that live outside the canvas, with the same names and behaviour in every binding.',
      featureItems: [
        {
          category: 'Windows',
          title: 'Windows you can shape',
          details:
            'Create, move and resize windows, hide or restyle the title bar, add drag-to-move and drag-to-resize areas, apply vibrancy and shaped outlines, and open as many windows as you need.',
        },
        {
          category: 'Tray icons & menus',
          title: 'Live in the menu bar and the system tray',
          details:
            'Put an icon in the macOS menu bar, the Windows notification area or a Linux StatusNotifier host, and attach native menus with submenus, check items and keyboard accelerators.',
        },
        {
          category: 'Displays & input',
          title: 'Know the screens, catch the keys',
          details:
            'Enumerate displays with their bounds, work areas and scale factors, follow the cursor across monitors, and register global shortcuts that fire while your app is in the background.',
        },
        {
          category: 'System services',
          title: 'The rest of the operating system',
          details:
            'Message and file dialogs, preferences, secure storage, the clipboard, launch at login, opening URLs, accessibility checks and application lifecycle events.',
        },
        {
          category: 'Generated bindings',
          title: 'One header, six idiomatic APIs',
          details:
            'The C++ headers are the source of truth. A code generator derives the C ABI and every binding from them, following each language’s own conventions, so a new API reaches all six at once.',
        },
      ],
      viewDocs: 'View documentation',
      benefits: [
        {
          title: 'Truly native',
          description:
            'Calls AppKit, Win32 and GTK directly — no embedded browser and no runtime of its own, just your language talking to the platform.',
        },
        {
          title: 'Consistent everywhere',
          description:
            'The same objects, events and semantics in every language and on every OS, so knowledge and bug fixes carry across.',
        },
        {
          title: 'Always in sync',
          description:
            'Bindings are regenerated from the core headers, so they never lag behind the native API or drift from each other.',
        },
      ],
      stepsTitle: 'Start in three steps',
      steps: [
        {
          title: 'Add the package',
          description:
            'Install the binding for your language: from pub.dev and crates.io, or built from source for the others.',
        },
        {
          title: 'Call native APIs',
          description:
            'Open a window, add a tray icon, read the displays — with the types and naming your language expects.',
        },
        {
          title: 'Run on every desktop',
          description:
            'The same code drives the native implementation on macOS, Windows and Linux.',
        },
      ],
      nativeCli: 'One repository, one core, six bindings — all MIT licensed',
      communityTitle: 'Built in the open',
      communityDescription:
        'nativeapi is developed on GitHub: the C++ core, the code generator, every binding and the examples. Report a native bug, request an API, or help bring a binding to production.',
      viewGithub: 'View on GitHub',
      faqTitle: 'Frequently asked questions',
      faqs: [
        {
          question: 'What is nativeapi?',
          answer:
            'nativeapi is a C++ library with native implementations of desktop system APIs — windows, tray icons, menus, displays, keyboard, dialogs, storage and more — plus generated bindings that expose it to Dart/Flutter, Rust, C#, JavaScript/TypeScript, Python and Go.',
        },
        {
          question: 'Which languages and platforms are supported?',
          answer:
            'Every binding runs on macOS, Windows and Linux. The Dart binding also runs on Android and iOS. The Dart and Rust packages are published; the C#, JavaScript, Python and Go bindings are built from source for now.',
        },
        {
          question: 'Is it ready for production?',
          answer:
            'It is a work in progress and the API is still changing. The Dart and Rust packages are released regularly and already power the leanflutter plugins; check each binding’s status in the documentation.',
        },
        {
          question: 'How does it relate to window_manager and tray_manager?',
          answer:
            'Those leanflutter plugins are built on nativeapi. If you only need one feature in a Flutter app, they remain the simplest choice; nativeapi_flutter gives you the whole API surface directly.',
        },
        {
          question: 'Is nativeapi open source?',
          answer:
            'Yes. nativeapi is open source under the MIT License. The core, the generator and every binding are on GitHub.',
        },
      ],
      installTitle: 'Get started',
      installDescription:
        'Pick your language. Published packages install with one command; the others build from a source checkout.',
      selectBinding: 'Select a language binding',
      copied: 'Copied',
      copyCommand: 'Copy install command',
      quickStart: 'Read the quick start',
      bindingDescriptions: [
        'Flutter apps on desktop and mobile — published on pub.dev.',
        'Plain Dart programs, no Flutter required — published on pub.dev.',
        'Safe Rust API over the FFI crate — published on crates.io.',
        'Not yet on NuGet — build the native library from source.',
        'Node.js, Deno and Bun — not yet on npm, build from source.',
        'ctypes, Python 3.10+ — prototype, build from source.',
        'cgo, Go 1.22+ — build the shared library from source.',
      ],
    },
    footer: {
      tagline: 'Native desktop APIs for every language.',
      taglineSecond: 'One C++ core, generated bindings.',
      columns: [
        {
          title: 'Product',
          links: ['Get started', 'Features', 'GitHub'],
        },
        {
          title: 'Support',
          links: ['FAQ', 'Documentation', 'Report an issue'],
        },
        {
          title: 'Ecosystem',
          links: ['nativeapi-core', 'window_manager', 'tray_manager'],
        },
      ],
    },
  },
  ja: {
    languageName: '日本語',
    languages: sharedLanguages,
    metadata: {
      title: 'nativeapi - あらゆる言語のためのネイティブデスクトップ API',
      description:
        'nativeapi は、ウィンドウ、トレイアイコン、メニュー、ディスプレイ、キーボード、ダイアログ、ストレージのネイティブ API を、1つの C++ コアから Dart/Flutter、Rust、C#、JavaScript/TypeScript、Python、Go に提供します。',
    },
    header: {
      menu: 'メニュー',
      openMenu: 'メニューを開く',
      closeMenu: 'メニューを閉じる',
      home: 'ホーム',
      features: '機能',
      docs: 'ドキュメント',
      api: 'API',
      install: '始める',
      language: '言語',
    },
    home: {
      badge: 'オープンソース · 開発中',
      heroLine1: 'ネイティブデスクトップ API を',
      heroLine2: '',
      heroHighlight: 'あらゆる言語へ',
      heroDescription:
        'ウィンドウ、トレイアイコン、メニュー、ディスプレイ、キーボード、ダイアログ、ストレージ。各プラットフォーム向けに C++ で一度だけ実装し、Dart/Flutter、Rust、C#、JavaScript/TypeScript、Python、Go のバインディングを自動生成します。',
      installCta: '始める',
      readDocs: 'ドキュメントを読む',
      heroNote: 'macOS、Windows、Linux に対応。Dart では Android と iOS も',
      trustLabels: ['言語バインディング', 'プラットフォーム', 'GitHub Stars', 'ライセンス'],
      problem:
        'UI ツールキットはどれも、ウィンドウの装飾、トレイアイコン、ネイティブメニュー、マルチモニターの座標といった同じプラットフォーム処理を、言語ごと・OS ごとに書き直しています。',
      problemContinuation:
        '挙動は少しずつずれ、同じバグを何度も直し、新しいバインディングは毎回ゼロから始まります。',
      promise: 'nativeapi は C++ で一度だけ書き、残りは生成します。',
      featuresEyebrow: '対応範囲',
      featuresTitle: 'フレームワークに足りないネイティブ層',
      featuresDescription:
        'キャンバスの外にあるデスクトップアプリの機能を、すべてのバインディングで同じ名前と挙動で提供します。',
      featureItems: [
        {
          category: 'ウィンドウ',
          title: '思いどおりに形づくれるウィンドウ',
          details:
            'ウィンドウの作成、移動、リサイズ、タイトルバーの非表示やスタイル変更、ドラッグ移動・ドラッグリサイズ領域、ビブランシーや任意形状、複数ウィンドウに対応します。',
        },
        {
          category: 'トレイアイコンとメニュー',
          title: 'メニューバーとシステムトレイに常駐',
          details:
            'macOS のメニューバー、Windows の通知領域、Linux の StatusNotifier にアイコンを置き、サブメニュー、チェック項目、キーボードアクセラレーター付きのネイティブメニューを付けられます。',
        },
        {
          category: 'ディスプレイと入力',
          title: '画面を知り、キーを捉える',
          details:
            'ディスプレイの範囲、作業領域、スケール係数を取得し、モニターをまたいでカーソルを追跡。アプリがバックグラウンドでも動くグローバルショートカットを登録できます。',
        },
        {
          category: 'システムサービス',
          title: 'OS のその他の機能',
          details:
            'メッセージ・ファイルダイアログ、設定、セキュアストレージ、クリップボード、ログイン時の起動、URL を開く、アクセシビリティの確認、アプリのライフサイクルイベント。',
        },
        {
          category: '生成されるバインディング',
          title: '1つのヘッダーから、6つの自然な API',
          details:
            'C++ ヘッダーが唯一の情報源です。コードジェネレーターが C ABI と各バインディングを各言語の慣習に沿って生成するため、新しい API は 6 言語すべてに同時に届きます。',
        },
      ],
      viewDocs: 'ドキュメントを見る',
      benefits: [
        {
          title: '本物のネイティブ',
          description:
            'AppKit、Win32、GTK を直接呼び出します。組み込みブラウザーも独自ランタイムもありません。',
        },
        {
          title: 'どこでも一貫',
          description:
            'すべての言語と OS で同じオブジェクト、イベント、セマンティクス。知識もバグ修正もそのまま共有できます。',
        },
        {
          title: '常に同期',
          description:
            'バインディングはコアのヘッダーから再生成されるため、ネイティブ API に遅れることも互いにずれることもありません。',
        },
      ],
      stepsTitle: '3ステップで開始',
      steps: [
        {
          title: 'パッケージを追加',
          description:
            '使う言語のバインディングをインストール。pub.dev と crates.io で公開中、その他はソースからビルドします。',
        },
        {
          title: 'ネイティブ API を呼び出す',
          description:
            'ウィンドウを開き、トレイアイコンを追加し、ディスプレイを読み取る。言語に合った型と命名で。',
        },
        {
          title: 'すべてのデスクトップで実行',
          description:
            '同じコードが macOS、Windows、Linux のネイティブ実装を動かします。',
        },
      ],
      nativeCli: '1つのリポジトリ、1つのコア、6つのバインディング。すべて MIT ライセンス',
      communityTitle: 'オープンに開発',
      communityDescription:
        'C++ コア、コードジェネレーター、各バインディング、サンプルまで、すべて GitHub で開発しています。ネイティブのバグ報告、API の要望、バインディングの本番対応への協力を歓迎します。',
      viewGithub: 'GitHub を見る',
      faqTitle: 'よくある質問',
      faqs: [
        {
          question: 'nativeapi とは？',
          answer:
            'ウィンドウ、トレイアイコン、メニュー、ディスプレイ、キーボード、ダイアログ、ストレージなどのデスクトップシステム API をネイティブ実装した C++ ライブラリと、それを Dart/Flutter、Rust、C#、JavaScript/TypeScript、Python、Go に公開する生成バインディングです。',
        },
        {
          question: 'どの言語とプラットフォームに対応していますか？',
          answer:
            'すべてのバインディングが macOS、Windows、Linux で動作し、Dart バインディングは Android と iOS にも対応します。Dart と Rust のパッケージは公開済みで、C#、JavaScript、Python、Go は現在ソースからビルドします。',
        },
        {
          question: '本番環境で使えますか？',
          answer:
            '開発中のため API はまだ変わります。Dart と Rust のパッケージは定期的にリリースされ、leanflutter のプラグインで既に使われています。各バインディングの状況はドキュメントで確認してください。',
        },
        {
          question: 'window_manager や tray_manager との関係は？',
          answer:
            'これらの leanflutter プラグインは nativeapi の上に作られています。Flutter アプリで1つの機能だけ必要ならそれらが最も手軽で、nativeapi_flutter では API 全体を直接使えます。',
        },
        {
          question: 'nativeapi はオープンソースですか？',
          answer:
            'はい。MIT ライセンスのオープンソースで、コア、ジェネレーター、すべてのバインディングが GitHub にあります。',
        },
      ],
      installTitle: '始める',
      installDescription:
        '言語を選んでください。公開済みパッケージは1つのコマンドで、その他はソースからビルドします。',
      selectBinding: '言語バインディングを選択',
      copied: 'コピー済み',
      copyCommand: 'インストールコマンドをコピー',
      quickStart: 'クイックスタートを読む',
      bindingDescriptions: [
        'デスクトップとモバイルの Flutter アプリ向け。pub.dev で公開中。',
        'Flutter 不要の Dart プログラム向け。pub.dev で公開中。',
        'FFI クレートの上の安全な Rust API。crates.io で公開中。',
        'NuGet は未公開。ネイティブライブラリをソースからビルドします。',
        'Node.js、Deno、Bun 向け。npm は未公開、ソースからビルドします。',
        'ctypes、Python 3.10+。プロトタイプ、ソースからビルドします。',
        'cgo、Go 1.22+。共有ライブラリをソースからビルドします。',
      ],
    },
    footer: {
      tagline: 'ネイティブデスクトップ API をあらゆる言語へ。',
      taglineSecond: '1つの C++ コアと生成バインディング。',
      columns: [
        { title: '製品', links: ['始める', '機能', 'GitHub'] },
        {
          title: 'サポート',
          links: ['よくある質問', 'ドキュメント', '問題を報告'],
        },
        {
          title: 'エコシステム',
          links: ['nativeapi-core', 'window_manager', 'tray_manager'],
        },
      ],
    },
  },
  zh: {
    languageName: '简体中文',
    languages: sharedLanguages,
    metadata: {
      title: 'nativeapi - 面向所有语言的原生桌面 API',
      description:
        'nativeapi 基于同一个 C++ 核心，为 Dart/Flutter、Rust、C#、JavaScript/TypeScript、Python 和 Go 提供窗口、托盘图标、菜单、显示器、键盘、对话框与存储等原生 API。',
    },
    header: {
      menu: '菜单',
      openMenu: '打开菜单',
      closeMenu: '关闭菜单',
      home: '首页',
      features: '功能',
      docs: '文档',
      api: 'API',
      install: '开始使用',
      language: '语言',
    },
    home: {
      badge: '开源 · 开发中',
      heroLine1: '原生桌面 API',
      heroLine2: '',
      heroHighlight: '适用于每一种语言',
      heroDescription:
        '窗口、托盘图标、菜单、显示器、键盘、对话框与存储——在各个平台上用 C++ 实现一次，再为 Dart/Flutter、Rust、C#、JavaScript/TypeScript、Python 和 Go 生成绑定。',
      installCta: '开始使用',
      readDocs: '阅读文档',
      heroNote: '支持 macOS、Windows 和 Linux，Dart 还支持 Android 与 iOS',
      trustLabels: ['语言绑定', '支持平台', 'GitHub Stars', '开源许可'],
      problem:
        '每个 UI 框架最终都要重写同一套平台胶水代码：窗口外观、托盘图标、原生菜单、多显示器坐标——每种语言一遍，每个系统一遍。',
      problemContinuation:
        '行为逐渐分叉，同一个 bug 要修三次，新的绑定又得从零开始。',
      promise: 'nativeapi 用 C++ 只写一次，其余全部生成。',
      featuresEyebrow: '覆盖范围',
      featuresTitle: '你的框架所缺少的原生层',
      featuresDescription:
        '桌面应用中画布之外的那部分能力，在每个绑定里都有相同的命名与行为。',
      featureItems: [
        {
          category: '窗口',
          title: '随心塑造的窗口',
          details:
            '创建、移动和调整窗口大小，隐藏或改变标题栏样式，添加拖动移动与拖动缩放区域，应用毛玻璃效果和异形轮廓，并按需打开多个窗口。',
        },
        {
          category: '托盘图标与菜单',
          title: '常驻菜单栏与系统托盘',
          details:
            '在 macOS 菜单栏、Windows 通知区域或 Linux StatusNotifier 中放置图标，并挂上带子菜单、勾选项和键盘快捷键的原生菜单。',
        },
        {
          category: '显示器与输入',
          title: '了解每块屏幕，捕捉每个按键',
          details:
            '枚举显示器的边界、工作区与缩放比例，跨显示器跟踪光标，注册在应用处于后台时也能触发的全局快捷键。',
        },
        {
          category: '系统服务',
          title: '操作系统的其余部分',
          details:
            '消息与文件对话框、偏好设置、安全存储、剪贴板、开机启动、打开链接、辅助功能检查以及应用生命周期事件。',
        },
        {
          category: '生成的绑定',
          title: '一份头文件，六套地道的 API',
          details:
            'C++ 头文件是唯一的事实来源。代码生成器从中导出 C ABI 和每个绑定，并遵循各语言自身的惯例，新 API 会同时抵达全部六种语言。',
        },
      ],
      viewDocs: '查看使用文档',
      benefits: [
        {
          title: '真正原生',
          description:
            '直接调用 AppKit、Win32 和 GTK——没有内嵌浏览器，也没有自带运行时，只有你的语言与平台直接对话。',
        },
        {
          title: '处处一致',
          description:
            '在每种语言、每个系统上都是相同的对象、事件和语义，经验与修复都能复用。',
        },
        {
          title: '始终同步',
          description:
            '绑定由核心头文件重新生成，永远不会落后于原生 API，也不会彼此走样。',
        },
      ],
      stepsTitle: '三步开始',
      steps: [
        {
          title: '添加依赖',
          description:
            '安装你所用语言的绑定：从 pub.dev 和 crates.io 获取，其余从源码构建。',
        },
        {
          title: '调用原生 API',
          description:
            '打开窗口、添加托盘图标、读取显示器信息——使用你的语言习惯的类型和命名。',
        },
        {
          title: '运行在每个桌面',
          description: '同一份代码在 macOS、Windows 和 Linux 上驱动原生实现。',
        },
      ],
      nativeCli: '一个仓库、一个核心、六种绑定，全部以 MIT 许可开源',
      communityTitle: '开放开发',
      communityDescription:
        'C++ 核心、代码生成器、每个绑定和示例都在 GitHub 上开发。欢迎报告原生问题、提出 API 需求，或帮助某个绑定走向生产可用。',
      viewGithub: '查看 GitHub',
      faqTitle: '常见问题',
      faqs: [
        {
          question: 'nativeapi 是什么？',
          answer:
            'nativeapi 是一个 C++ 库，原生实现了窗口、托盘图标、菜单、显示器、键盘、对话框、存储等桌面系统 API，并通过生成的绑定提供给 Dart/Flutter、Rust、C#、JavaScript/TypeScript、Python 和 Go。',
        },
        {
          question: '支持哪些语言和平台？',
          answer:
            '所有绑定都运行在 macOS、Windows 和 Linux 上，Dart 绑定还支持 Android 和 iOS。Dart 与 Rust 的包已经发布；C#、JavaScript、Python 和 Go 目前需从源码构建。',
        },
        {
          question: '可以用于生产环境吗？',
          answer:
            '项目仍在开发中，API 还会变化。Dart 与 Rust 包会定期发布，并已支撑 leanflutter 系列插件；各绑定的状态请查看文档。',
        },
        {
          question: '它和 window_manager、tray_manager 是什么关系？',
          answer:
            '这些 leanflutter 插件构建在 nativeapi 之上。如果 Flutter 应用只需要其中一项功能，它们依然是最简单的选择；nativeapi_flutter 则让你直接使用完整的 API。',
        },
        {
          question: 'nativeapi 是开源项目吗？',
          answer:
            '是。nativeapi 以 MIT 许可证开源，核心、生成器和所有绑定都在 GitHub 上。',
        },
      ],
      installTitle: '开始使用',
      installDescription:
        '选择你的语言。已发布的包一条命令即可安装，其余从源码检出构建。',
      selectBinding: '选择语言绑定',
      copied: '已复制',
      copyCommand: '复制安装命令',
      quickStart: '阅读快速上手',
      bindingDescriptions: [
        '面向桌面与移动端的 Flutter 应用，已发布到 pub.dev。',
        '无需 Flutter 的纯 Dart 程序，已发布到 pub.dev。',
        '基于 FFI crate 的安全 Rust API，已发布到 crates.io。',
        '尚未发布到 NuGet，需从源码构建原生库。',
        '支持 Node.js、Deno 和 Bun，尚未发布到 npm，需从源码构建。',
        'ctypes，Python 3.10+，原型阶段，需从源码构建。',
        'cgo，Go 1.22+，需从源码构建共享库。',
      ],
    },
    footer: {
      tagline: '面向所有语言的原生桌面 API。',
      taglineSecond: '一个 C++ 核心，自动生成绑定。',
      columns: [
        { title: '产品', links: ['开始使用', '功能', 'GitHub'] },
        { title: '支持', links: ['常见问题', '使用文档', '问题反馈'] },
        {
          title: '生态',
          links: ['nativeapi-core', 'window_manager', 'tray_manager'],
        },
      ],
    },
  },
} as const

export function getMessages(locale: Locale) {
  return messages[locale]
}

export function docsUrl(locale: Locale, gettingStarted = false) {
  return localizedPath(locale, gettingStarted ? '/docs/getting-started' : '/docs')
}

/**
 * Runs synchronously in <head>, before the body paints, so a returning
 * ja/zh visitor lands on their locale without ever seeing the English
 * homepage flash. Must stay dependency-free plain JS (inlined into HTML).
 */
export function homeLocaleRedirectScript() {
  return `(function () {
  try {
    if (window.location.pathname !== '/') return
    var supported = ${JSON.stringify(locales)}
    var key = ${JSON.stringify(localeStorageKey)}
    var stored = window.localStorage.getItem(key)
    var preferred = stored && supported.indexOf(stored) !== -1 ? stored : null
    if (!preferred) {
      var languages = navigator.languages || [navigator.language || '']
      for (var i = 0; i < languages.length; i++) {
        var short = (languages[i] || '').split('-')[0]
        if (supported.indexOf(short) !== -1) {
          preferred = short
          break
        }
      }
    }
    if (preferred && preferred !== 'en') {
      window.location.replace('/' + preferred + window.location.hash)
    } else {
      window.localStorage.setItem(key, 'en')
    }
  } catch (e) {}
})()`
}
