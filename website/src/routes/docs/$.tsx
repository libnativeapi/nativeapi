import { createFileRoute } from '@tanstack/react-router'
import { docsHead, loadDocsPage, normalizeDocsSlug } from '@/lib/docs'
import { DocsPage } from './_components/docs-page'

export const Route = createFileRoute('/docs/$')({
  loader: ({ params }) => loadDocsPage('en', normalizeDocsSlug(params._splat)),
  head: ({ params, loaderData }) =>
    docsHead('en', normalizeDocsSlug(params._splat), loaderData),
  component: DocsDetailPage,
})

function DocsDetailPage() {
  const page = Route.useLoaderData()
  return <DocsPage locale="en" page={page} />
}
