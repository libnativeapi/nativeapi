import { createFileRoute, notFound, redirect } from '@tanstack/react-router'
import { apiHead, isApiLanguage } from '@/lib/api'
import { isLocale, type Locale } from '@/lib/i18n'
import { ApiOverviewPage } from './api/_components/api-page'

export const Route = createFileRoute('/$locale_/api/$lang/')({
  beforeLoad: ({ params }) => {
    if (params.locale === 'en') {
      throw redirect({ to: '/api/$lang', params: { lang: params.lang } })
    }
    if (!isLocale(params.locale) || !isApiLanguage(params.lang)) throw notFound()
  },
  head: ({ params }) =>
    apiHead(isLocale(params.locale) ? params.locale : 'en', params.lang),
  component: LocalizedApiOverviewRoute,
})

function LocalizedApiOverviewRoute() {
  const { locale, lang } = Route.useParams()
  return <ApiOverviewPage locale={locale as Locale} lang={lang} />
}
