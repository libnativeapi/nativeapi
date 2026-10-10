import { createFileRoute } from '@tanstack/react-router'
import { apiHead, loadApiModule } from '@/lib/api'
import { ApiModulePage } from '../_components/api-page'

export const Route = createFileRoute('/api/$lang/$module')({
  loader: ({ params }) => loadApiModule(params.lang, params.module),
  head: ({ params, loaderData }) => apiHead('en', params.lang, loaderData),
  component: ApiModuleRoute,
})

function ApiModuleRoute() {
  const { lang } = Route.useParams()
  const module = Route.useLoaderData()
  return <ApiModulePage locale="en" lang={lang} module={module} />
}
