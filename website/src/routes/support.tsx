import { createFileRoute } from '@tanstack/react-router'
import { locales, localizedPath } from '@/lib/i18n'
import { getSupportCopy } from './support/_components/support-copy'
import { SupportPage } from './support/_components/support-page'

const copy = getSupportCopy('en')

export const Route = createFileRoute('/support')({
  head: () => ({
    meta: [
      { title: copy.metaTitle },
      { name: 'description', content: copy.metaDescription },
    ],
    links: [
      ...locales.map(locale => ({
        rel: 'alternate',
        hrefLang: locale,
        href: localizedPath(locale, '/support'),
      })),
    ],
  }),
  component: () => <SupportPage locale="en" />,
})
