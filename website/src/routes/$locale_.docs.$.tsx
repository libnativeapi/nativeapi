import { createFileRoute, notFound, redirect } from '@tanstack/react-router'
import { docsHead, loadDocsPage, normalizeDocsSlug } from '@/lib/docs'
import { isLocale, type Locale } from '@/lib/i18n'
import { DocsPage } from './docs/_components/docs-page'

export const Route = createFileRoute('/$locale_/docs/$')({
  beforeLoad: ({ params }) => {
    if (params.locale === 'en') {
      throw redirect({ href: `/docs/${normalizeDocsSlug(params._splat)}` })
    }
    if (!isLocale(params.locale)) throw notFound()
  },
  loader: ({ params }) =>
    loadDocsPage(
      isLocale(params.locale) ? params.locale : 'en',
      normalizeDocsSlug(params._splat),
    ),
  head: ({ params, loaderData }) => {
    const locale = isLocale(params.locale) ? params.locale : 'en'
    return docsHead(locale, normalizeDocsSlug(params._splat), loaderData)
  },
  component: LocalizedDocsDetailPage,
})

function LocalizedDocsDetailPage() {
  const { locale } = Route.useParams()
  const page = Route.useLoaderData()
  return <DocsPage locale={locale as Locale} page={page} />
}
