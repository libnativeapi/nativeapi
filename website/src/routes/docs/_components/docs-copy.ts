import type { Locale } from '@/lib/i18n'

export interface DocsCopy {
  breadcrumb: string
  docsHome: string
  navLabel: string
  onThisPage: string
  editOnGitHub: string
  /** Shown when a page's content falls back to a different language. */
  languageNotice: string
  supportLabel: string
  supportDescription: string
  supportAction: string
}

const docsCopy: Record<Locale, DocsCopy> = {
  en: {
    breadcrumb: 'Documentation',
    docsHome: 'Overview',
    navLabel: 'Documentation',
    onThisPage: 'On this page',
    editOnGitHub: 'Edit this page on GitHub',
    languageNotice: '',
    supportLabel: 'Need help?',
    supportDescription:
      'Stuck on a binding, a platform quirk or a missing API? Ask the community or open an issue.',
    supportAction: 'Get support',
  },
  ja: {
    breadcrumb: 'ドキュメント',
    docsHome: '概要',
    navLabel: 'ドキュメント',
    onThisPage: 'このページの内容',
    editOnGitHub: 'GitHub でこのページを編集',
    languageNotice:
      'このページの日本語版は準備中のため、英語版を表示しています。',
    supportLabel: 'サポートが必要ですか？',
    supportDescription:
      'バインディング、プラットフォーム固有の挙動、足りない API で困ったら、コミュニティに質問するか Issue を作成してください。',
    supportAction: 'サポートを見る',
  },
  zh: {
    breadcrumb: '文档',
    docsHome: '总览',
    navLabel: '文档',
    onThisPage: '本页内容',
    editOnGitHub: '在 GitHub 上编辑此页',
    languageNotice: '本页暂无中文翻译，以下显示英文原文。',
    supportLabel: '需要帮助？',
    supportDescription: '在绑定、平台差异或缺失的 API 上遇到问题？欢迎在社区提问或提交 issue。',
    supportAction: '获取帮助',
  },
}

export function getDocsCopy(locale: Locale): DocsCopy {
  return docsCopy[locale]
}
