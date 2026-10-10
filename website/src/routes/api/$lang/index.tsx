import { createFileRoute, notFound } from '@tanstack/react-router'
import { apiHead, isApiLanguage } from '@/lib/api'
import { ApiOverviewPage } from '../_components/api-page'

export const Route = createFileRoute('/api/$lang/')({
  beforeLoad: ({ params }) => {
    if (!isApiLanguage(params.lang)) throw notFound()
  },
  head: ({ params }) => apiHead('en', params.lang),
  component: ApiOverviewRoute,
})

function ApiOverviewRoute() {
  const { lang } = Route.useParams()
  return <ApiOverviewPage locale="en" lang={lang} />
}
