import { createFileRoute, notFound, redirect } from '@tanstack/react-router'
import { docsHead, loadDocsPage } from '@/lib/docs'
import { isLocale, type Locale } from '@/lib/i18n'
import { DocsPage } from './docs/_components/docs-page'

export const Route = createFileRoute('/$locale_/docs/')({
  beforeLoad: ({ params }) => {
    if (params.locale === 'en') throw redirect({ to: '/docs' })
    if (!isLocale(params.locale)) throw notFound()
  },
  loader: ({ params }) =>
    loadDocsPage(isLocale(params.locale) ? params.locale : 'en', ''),
  head: ({ params, loaderData }) => {
    const locale = isLocale(params.locale) ? params.locale : 'en'
    return docsHead(locale, '', loaderData)
  },
  component: LocalizedDocsIndexPage,
})

function LocalizedDocsIndexPage() {
  const { locale } = Route.useParams()
  const page = Route.useLoaderData()
  return <DocsPage locale={locale as Locale} page={page} />
}
