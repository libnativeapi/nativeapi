import type { Locale } from '@/lib/i18n'

export interface SupportCopy {
  metaTitle: string
  metaDescription: string
  eyebrow: string
  title: string
  description: string
  cards: readonly {
    title: string
    description: string
    action: string
  }[]
  faqEyebrow: string
  faqTitle: string
  faqDescription: string
  contactTitle: string
  contactDescription: string
  contactAction: string
}

const supportCopy: Record<Locale, SupportCopy> = {
  en: {
    metaTitle: 'Support — nativeapi',
    metaDescription: 'Get help with nativeapi, browse the documentation, and find answers to common questions.',
    eyebrow: 'Support',
    title: 'How can we help?',
    description: 'Find documentation, community support, and answers to frequently asked questions.',
    cards: [
      {
        title: 'Documentation',
        description: 'Installation and quick starts for every binding.',
        action: 'Read the docs',
      },
      {
        title: 'Issues',
        description: 'Report a native bug or request an API on GitHub.',
        action: 'Open an issue',
      },
      {
        title: 'FAQ',
        description: 'Quick answers to the most common questions.',
        action: 'View FAQ',
      },
    ],
    faqEyebrow: 'FAQ',
    faqTitle: 'Frequently Asked Questions',
    faqDescription: 'Quick answers to the most common questions about nativeapi.',
    contactTitle: 'Still need help?',
    contactDescription: 'Start a discussion or open an issue on GitHub. We\'re happy to help.',
    contactAction: 'Visit GitHub',
  },
  zh: {
    metaTitle: '支持 — nativeapi',
    metaDescription: '获取 nativeapi 的帮助，浏览文档并查找常见问题的答案。',
    eyebrow: '支持',
    title: '有什么可以帮你的？',
    description: '查找文档、社区支持和常见问题解答。',
    cards: [
      {
        title: '使用文档',
        description: '每个绑定的安装与快速上手。',
        action: '阅读文档',
      },
      {
        title: 'Issues',
        description: '在 GitHub 上报告原生问题或提出 API 需求。',
        action: '提交 issue',
      },
      {
        title: '常见问题',
        description: '快速查找最常见问题的答案。',
        action: '查看 FAQ',
      },
    ],
    faqEyebrow: '常见问题',
    faqTitle: '常见问题解答',
    faqDescription: '关于 nativeapi 最常见问题的快速解答。',
    contactTitle: '仍然需要帮助？',
    contactDescription: '在 GitHub 上发起讨论或提交 issue，我们很乐意提供帮助。',
    contactAction: '访问 GitHub',
  },
  ja: {
    metaTitle: 'サポート — nativeapi',
    metaDescription: 'nativeapi のヘルプを入手し、ドキュメントを参照し、よくある質問への回答を見つけます。',
    eyebrow: 'サポート',
    title: 'どのようなお手伝いができますか？',
    description: 'ドキュメント、コミュニティサポート、よくある質問への回答をご覧いただけます。',
    cards: [
      {
        title: 'ドキュメント',
        description: '各バインディングのインストールとクイックスタート。',
        action: 'ドキュメントを見る',
      },
      {
        title: 'Issues',
        description: 'GitHub でネイティブのバグ報告や API の要望ができます。',
        action: 'Issue を作成',
      },
      {
        title: 'よくある質問',
        description: '最も一般的な質問への迅速な回答。',
        action: 'FAQ を見る',
      },
    ],
    faqEyebrow: 'よくある質問',
    faqTitle: 'よくある質問',
    faqDescription: 'nativeapi に関する最も一般的な質問への回答。',
    contactTitle: 'まだお困りですか？',
    contactDescription: 'GitHub でディスカッションや Issue をお気軽にどうぞ。',
    contactAction: 'GitHub を見る',
  },
}

export function getSupportCopy(locale: Locale) {
  return supportCopy[locale]
}
