import { notFound } from '@tanstack/react-router'
import manifest from '@/content/docs-gen/manifest.json'
import { locales, localizedPath, type Locale } from '@/lib/i18n'

export interface DocsTocItem {
  id: string
  text: string
  depth: 2 | 3
}

export interface DocsPageData {
  slug: string
  /** Language the content was compiled from (may differ from the UI locale). */
  locale: string
  title: string
  description: string
  html: string
  toc: DocsTocItem[]
  sourcePath: string
}

export interface DocsNavChild {
  slug: string
  title: string
}

export type DocsNavItem =
  | { type: 'page'; slug: string; title: string }
  | {
      type: 'group'
      slug: string
      title: string
      hasIndex: boolean
      children: DocsNavChild[]
    }

interface DocsLocaleManifest {
  indexTitle: string
  nav: DocsNavItem[]
  slugs: string[]
}

interface DocsManifest {
  repo: string
  ref: string
  locales: Record<string, DocsLocaleManifest>
}

export const docsManifest = manifest as unknown as DocsManifest

const pageModules = import.meta.glob('../content/docs-gen/pages/*/*.json', {
  import: 'default',
}) as Record<string, () => Promise<DocsPageData>>

/** Some locales have no content of their own; fall back to English there. */
export function docsContentLocale(locale: Locale): string {
  return locale in docsManifest.locales ? locale : 'en'
}

export function getDocsNav(locale: Locale): DocsNavItem[] {
  return docsManifest.locales[docsContentLocale(locale)].nav
}

export function normalizeDocsSlug(splat: string | undefined) {
  return (splat ?? '').replace(/^\/+|\/+$/g, '')
}

export async function loadDocsPage(
  locale: Locale,
  slug: string,
): Promise<DocsPageData> {
  const file = slug === '' ? 'index' : slug.replaceAll('/', '__')
  const contentLocale = docsContentLocale(locale)
  const loader =
    pageModules[`../content/docs-gen/pages/${contentLocale}/${file}.json`] ??
    // A page missing in a translation falls back to English rather than 404.
    pageModules[`../content/docs-gen/pages/en/${file}.json`]
  if (!loader) throw notFound()
  return loader()
}

export function docsPath(locale: Locale, slug: string) {
  return localizedPath(locale, slug === '' ? '/docs' : `/docs/${slug}`)
}

export function docsSourceUrl(page: DocsPageData) {
  return `https://github.com/${docsManifest.repo}/blob/${docsManifest.ref}/${page.sourcePath}`
}

export function docsHead(locale: Locale, slug: string, page?: DocsPageData) {
  if (!page) return {}
  return {
    meta: [
      { title: `${page.title} — nativeapi` },
      { name: 'description', content: page.description },
    ],
    links: locales.map(item => ({
      rel: 'alternate',
      hrefLang: item,
      href: docsPath(item, slug),
    })),
  }
}
