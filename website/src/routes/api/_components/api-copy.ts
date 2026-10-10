import type { ApiItemKind, ApiMemberKind } from '@/lib/api'
import type { Locale } from '@/lib/i18n'

export interface ApiCopy {
  title: string
  description: string
  breadcrumb: string
  language: string
  modules: string
  onThisPage: string
  /** `{language}` is replaced by the binding's label. */
  unavailable: string
  overviewTitle: string
  overviewDescription: string
  parameters: string
  returns: string
  members: string
  eventTypes: string
  source: string
  englishNotice: string
  itemKinds: Record<ApiItemKind, string>
  memberKinds: Record<ApiMemberKind, string>
  noteKinds: Record<string, string>
}

const apiCopy: Record<Locale, ApiCopy> = {
  en: {
    title: 'API reference',
    description:
      'Every class, method, enum and event of nativeapi, as each binding spells it. Generated from the core headers.',
    breadcrumb: 'API',
    language: 'Binding',
    modules: 'Modules',
    onThisPage: 'On this page',
    unavailable: 'Not available in {language}',
    overviewTitle: 'Modules',
    overviewDescription:
      'One module per core header. Switch the binding to see the same API in another language.',
    parameters: 'Parameters',
    returns: 'Returns',
    members: 'Members',
    eventTypes: 'Event types',
    source: 'View header on GitHub',
    englishNotice: '',
    itemKinds: {
      class: 'class',
      singleton: 'singleton',
      struct: 'struct',
      enum: 'enum',
      event: 'event',
      alias: 'type alias',
    },
    memberKinds: {
      constructor: 'constructor',
      method: 'method',
      static: 'static',
      listener: 'listener',
      field: 'field',
      constant: 'constant',
      variant: 'value',
    },
    noteKinds: {
      note: 'Note',
      warning: 'Warning',
      see: 'See also',
      thread_safety: 'Thread safety',
      deprecated: 'Deprecated',
    },
  },
  ja: {
    title: 'API リファレンス',
    description:
      'nativeapi のすべてのクラス、メソッド、列挙型、イベントを各バインディングの表記で掲載。コアのヘッダーから生成しています。',
    breadcrumb: 'API',
    language: 'バインディング',
    modules: 'モジュール',
    onThisPage: 'このページの内容',
    unavailable: '{language} では利用できません',
    overviewTitle: 'モジュール',
    overviewDescription:
      'コアのヘッダーごとに1モジュール。バインディングを切り替えると、同じ API を別の言語で表示します。',
    parameters: 'パラメーター',
    returns: '戻り値',
    members: 'メンバー',
    eventTypes: 'イベントの種類',
    source: 'GitHub でヘッダーを見る',
    englishNotice: 'API の説明はコアのヘッダーから生成しているため英語です。',
    itemKinds: {
      class: 'クラス',
      singleton: 'シングルトン',
      struct: '構造体',
      enum: '列挙型',
      event: 'イベント',
      alias: '型エイリアス',
    },
    memberKinds: {
      constructor: 'コンストラクター',
      method: 'メソッド',
      static: '静的',
      listener: 'リスナー',
      field: 'フィールド',
      constant: '定数',
      variant: '値',
    },
    noteKinds: {
      note: '注意',
      warning: '警告',
      see: '関連',
      thread_safety: 'スレッド安全性',
      deprecated: '非推奨',
    },
  },
  zh: {
    title: 'API 参考',
    description:
      'nativeapi 的全部类、方法、枚举和事件，按每个绑定的写法列出，由核心头文件生成。',
    breadcrumb: 'API',
    language: '绑定',
    modules: '模块',
    onThisPage: '本页内容',
    unavailable: '{language} 中不可用',
    overviewTitle: '模块',
    overviewDescription: '每个核心头文件对应一个模块。切换绑定即可查看同一 API 在另一种语言中的写法。',
    parameters: '参数',
    returns: '返回值',
    members: '成员',
    eventTypes: '事件类型',
    source: '在 GitHub 上查看头文件',
    englishNotice: 'API 说明由核心头文件的注释生成，因此为英文。',
    itemKinds: {
      class: '类',
      singleton: '单例',
      struct: '结构体',
      enum: '枚举',
      event: '事件',
      alias: '类型别名',
    },
    memberKinds: {
      constructor: '构造',
      method: '方法',
      static: '静态',
      listener: '监听',
      field: '字段',
      constant: '常量',
      variant: '取值',
    },
    noteKinds: {
      note: '注意',
      warning: '警告',
      see: '参见',
      thread_safety: '线程安全',
      deprecated: '已弃用',
    },
  },
}

export function getApiCopy(locale: Locale) {
  return apiCopy[locale]
}
