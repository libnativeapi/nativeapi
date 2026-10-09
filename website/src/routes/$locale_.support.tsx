import { createFileRoute, notFound, redirect } from '@tanstack/react-router'
import { isLocale, locales, localizedPath, type Locale } from '@/lib/i18n'
import { getSupportCopy } from './support/_components/support-copy'
import { SupportPage } from './support/_components/support-page'

export const Route = createFileRoute('/$locale_/support')({
  beforeLoad: ({ params }) => {
    if (params.locale === 'en') throw redirect({ to: '/support' })
    if (!isLocale(params.locale)) throw notFound()
  },
  head: ({ params }) => {
    const locale = isLocale(params.locale) ? params.locale : 'en'
    const copy = getSupportCopy(locale)
    return {
      meta: [
        { title: copy.metaTitle },
        { name: 'description', content: copy.metaDescription },
      ],
      links: [
        ...locales.map(item => ({
          rel: 'alternate',
          hrefLang: item,
          href: localizedPath(item, '/support'),
        })),
      ],
    }
  },
  component: LocalizedSupportPage,
})

function LocalizedSupportPage() {
  const { locale } = Route.useParams()
  return <SupportPage locale={locale as Locale} />
}
