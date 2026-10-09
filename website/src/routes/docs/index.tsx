import { createFileRoute } from '@tanstack/react-router'
import { docsHead, loadDocsPage } from '@/lib/docs'
import { DocsPage } from './_components/docs-page'

export const Route = createFileRoute('/docs/')({
  loader: () => loadDocsPage('en', ''),
  head: ({ loaderData }) => docsHead('en', '', loaderData),
  component: DocsIndexPage,
})

function DocsIndexPage() {
  const page = Route.useLoaderData()
  return <DocsPage locale="en" page={page} />
}
