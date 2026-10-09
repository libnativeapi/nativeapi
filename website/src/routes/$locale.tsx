import { createFileRoute, notFound, redirect } from '@tanstack/react-router'
import { HomePage } from './index'
import { getGithubStats } from '@/lib/github-stats'
import {
  getMessages,
  isLocale,
  localePath,
  locales,
  type Locale,
} from '@/lib/i18n'

export const Route = createFileRoute('/$locale')({
  beforeLoad: ({ params }) => {
    if (params.locale === 'en') {
      throw redirect({ to: '/' })
    }
    if (!isLocale(params.locale)) {
      throw notFound()
    }
  },
  head: ({ params }) => {
    const locale = isLocale(params.locale) ? params.locale : 'en'
    const copy = getMessages(locale)

    return {
      meta: [
        { title: copy.metadata.title },
        { name: 'description', content: copy.metadata.description },
      ],
      links: locales.map(item => ({
        rel: 'alternate',
        hrefLang: item,
        href: localePath(item),
      })),
    }
  },
  loader: () => getGithubStats(),
  component: LocalizedHomePage,
})

function LocalizedHomePage() {
  const { locale } = Route.useParams()
  const githubStats = Route.useLoaderData()
  return <HomePage locale={locale as Locale} githubStats={githubStats} />
}
