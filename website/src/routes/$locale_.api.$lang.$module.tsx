import { createFileRoute, notFound, redirect } from '@tanstack/react-router'
import { apiHead, loadApiModule } from '@/lib/api'
import { isLocale, type Locale } from '@/lib/i18n'
import { ApiModulePage } from './api/_components/api-page'

export const Route = createFileRoute('/$locale_/api/$lang/$module')({
  beforeLoad: ({ params }) => {
    if (params.locale === 'en') {
      throw redirect({
        to: '/api/$lang/$module',
        params: { lang: params.lang, module: params.module },
      })
    }
    if (!isLocale(params.locale)) throw notFound()
  },
  loader: ({ params }) => loadApiModule(params.lang, params.module),
  head: ({ params, loaderData }) =>
    apiHead(isLocale(params.locale) ? params.locale : 'en', params.lang, loaderData),
  component: LocalizedApiModuleRoute,
})

function LocalizedApiModuleRoute() {
  const { locale, lang } = Route.useParams()
  const module = Route.useLoaderData()
  return <ApiModulePage locale={locale as Locale} lang={lang} module={module} />
}
