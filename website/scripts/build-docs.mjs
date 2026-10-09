#!/usr/bin/env node
/**
 * Compile the documentation Markdown into JSON consumed by the docs routes.
 *
 * Sources, per site locale:
 *   website/content/docs/<lang>/**\/*.md   hand-written pages (en, zh-Hans)
 *   bindings/<lang>/README.md (README-ZH.md)  one page per binding, slug
 *                                          "bindings/<lang>" — the binding
 *                                          READMEs stay the single source
 *
 * Output (gitignored, regenerated before dev/build):
 *   src/content/docs-gen/manifest.json               per-locale nav trees + slug index
 *   src/content/docs-gen/pages/<locale>/<slug>.json  { slug, locale, title, ... }
 *
 * Slugs mirror the file layout inside a language directory: `getting-started.md`
 * -> "getting-started", any `README.md` -> its directory ("" for the language
 * root). In page filenames "/" is encoded as "__". Relative links are resolved
 * against the source file's path in the repository: a link to another compiled
 * page becomes its route (locale prefix baked in, "/docs/..." for en,
 * "/zh/docs/..." for zh), anything else points at the file on GitHub.
 */
import { existsSync, mkdirSync, readdirSync, readFileSync, rmSync, writeFileSync } from 'node:fs'
import { dirname, join, posix } from 'node:path'
import { fileURLToPath } from 'node:url'
import rehypeShiki from '@shikijs/rehype'
import { toString as hastToString } from 'hast-util-to-string'
import { toString as mdastToString } from 'mdast-util-to-string'
import rehypeSlug from 'rehype-slug'
import rehypeStringify from 'rehype-stringify'
import remarkGfm from 'remark-gfm'
import remarkParse from 'remark-parse'
import remarkRehype from 'remark-rehype'
import { unified } from 'unified'
import { visit } from 'unist-util-visit'

const appDir = join(dirname(fileURLToPath(import.meta.url)), '..')
const repoRoot = join(appDir, '..')
const docsDir = 'website/content/docs'
const outDir = join(appDir, 'src/content/docs-gen')

const REPO = 'libnativeapi/nativeapi'
const REF = 'main'
const UPSTREAM_BLOB_URL = `https://github.com/${REPO}/blob/${REF}`

/** site locale -> language directory under content/docs/ */
const LOCALE_DIRS = { en: 'en', zh: 'zh-Hans' }

/** site locale -> route prefix baked into compiled links */
const PATH_PREFIX = { en: '/docs', zh: '/zh/docs' }

/**
 * Binding READMEs published as docs pages, in nav order, with a short nav
 * title (the READMEs' own headings are long). A locale missing from `files`
 * falls back to English at runtime.
 */
const BINDINGS = [
  { slug: 'bindings/dart', navTitle: 'Dart / Flutter', files: { en: 'bindings/dart/README.md', zh: 'bindings/dart/README-ZH.md' } },
  { slug: 'bindings/rust', navTitle: 'Rust', files: { en: 'bindings/rust/README.md' } },
  { slug: 'bindings/csharp', navTitle: 'C#', files: { en: 'bindings/csharp/README.md' } },
  { slug: 'bindings/js', navTitle: 'JavaScript / TypeScript', files: { en: 'bindings/js/README.md' } },
  { slug: 'bindings/python', navTitle: 'Python', files: { en: 'bindings/python/README.md' } },
  { slug: 'bindings/go', navTitle: 'Go', files: { en: 'bindings/go/README.md' } },
]

const NAV_TITLES = new Map(BINDINGS.map(binding => [binding.slug, binding.navTitle]))

/** Preferred ordering; unknown entries are appended alphabetically. */
const ROOT_ORDER = ['getting-started', 'bindings', 'architecture']

/** Preferred ordering of group children; unknown ones follow alphabetically. */
const CHILD_ORDER = BINDINGS.map(binding => binding.slug)

/** Titles for directories that have no README.md of their own. */
const GROUP_TITLES = {
  en: { bindings: 'Bindings' },
  zh: { bindings: '语言绑定' },
}

const GITHUB_ALERTS = new Set(['NOTE', 'TIP', 'IMPORTANT', 'WARNING', 'CAUTION'])

/** file path relative to a language root -> route slug */
function fileToSlug(relPath) {
  const noExt = relPath.replace(/\.md$/i, '')
  if (posix.basename(noExt) === 'README') return posix.dirname(noExt).replace(/^\.$/, '')
  return noExt
}

function slugToPageFile(slug) {
  return `${slug === '' ? 'index' : slug.replace(/\//g, '__')}.json`
}

/**
 * Rewrite relative markdown links (resolved against the page's repository
 * path) to docs routes or GitHub URLs, and tag GitHub-style alert blockquotes
 * ("> [!NOTE]") so CSS can style them.
 */
function remarkDocsTransforms({ sourcePath, routesByPath }) {
  return tree => {
    visit(tree, 'link', node => {
      const url = node.url ?? ''
      if (/^(?:[a-z][a-z0-9+.-]*:|\/\/|#|\/)/i.test(url)) return

      const [pathPart, hash] = url.split('#')
      if (!pathPart) return
      const suffix = hash ? `#${hash}` : ''
      const resolved = posix.normalize(posix.join(posix.dirname(sourcePath), pathPart))
      const route = routesByPath.get(resolved) ?? routesByPath.get(posix.join(resolved, 'README.md'))
      node.url = route ? `${route}${suffix}` : `${UPSTREAM_BLOB_URL}/${resolved.replace(/\/$/, '')}${suffix}`
    })

    visit(tree, 'blockquote', node => {
      const first = node.children?.[0]
      if (first?.type !== 'paragraph') return
      const text = first.children?.[0]
      if (text?.type !== 'text') return
      const match = /^\[!(NOTE|TIP|IMPORTANT|WARNING|CAUTION)\]\s*\n?/.exec(text.value)
      if (!match || !GITHUB_ALERTS.has(match[1])) return

      text.value = text.value.slice(match[0].length)
      if (!text.value && first.children.length === 1) node.children.shift()
      node.data ??= {}
      node.data.hProperties = {
        ...node.data.hProperties,
        class: `docs-alert docs-alert-${match[1].toLowerCase()}`,
        'data-alert': match[1],
      }
    })
  }
}

/** Collect h2/h3 headings (id + text) from the final hast tree. */
function rehypeExtractToc({ toc }) {
  return tree => {
    visit(tree, 'element', node => {
      if (node.tagName !== 'h2' && node.tagName !== 'h3') return
      if (!node.properties?.id) return
      toc.push({
        id: String(node.properties.id),
        text: hastToString(node),
        depth: node.tagName === 'h2' ? 2 : 3,
      })
    })
  }
}

function collectMarkdownFiles(dir, prefix = '') {
  const files = []
  for (const entry of readdirSync(dir, { withFileTypes: true })) {
    const rel = prefix ? `${prefix}/${entry.name}` : entry.name
    if (entry.isDirectory()) files.push(...collectMarkdownFiles(join(dir, entry.name), rel))
    else if (/\.md$/i.test(entry.name)) files.push(rel)
  }
  return files
}

function extractTitleAndDescription(mdast, fallbackTitle) {
  let title = fallbackTitle
  let description = ''
  for (const node of mdast.children) {
    if (node.type === 'heading' && node.depth === 1 && title === fallbackTitle) {
      title = mdastToString(node)
    } else if (node.type === 'paragraph' && !description) {
      description = mdastToString(node).replace(/\s+/g, ' ').trim()
    }
    if (title !== fallbackTitle && description) break
  }
  return { title, description }
}

function orderSlugs(slugs, preferred) {
  const rank = new Map(preferred.map((slug, index) => [slug, index]))
  return [...slugs].sort((a, b) => {
    const ra = rank.get(a) ?? preferred.length
    const rb = rank.get(b) ?? preferred.length
    return ra === rb ? a.localeCompare(b) : ra - rb
  })
}

function navTitle(pages, slug) {
  return NAV_TITLES.get(slug) ?? pages.get(slug).title
}

/** `pages` holds every page the locale can serve, English fallbacks included. */
function buildNav(locale, pages) {
  const rootEntries = new Set()
  const groups = new Map()
  for (const slug of pages.keys()) {
    if (slug === '') continue
    const [head, ...rest] = slug.split('/')
    rootEntries.add(head)
    if (rest.length > 0) {
      if (!groups.has(head)) groups.set(head, [])
      groups.get(head).push(slug)
    }
  }

  return orderSlugs([...rootEntries], ROOT_ORDER).map(slug => {
    const children = groups.get(slug)
    if (!children) {
      return { type: 'page', slug, title: navTitle(pages, slug) }
    }
    const hasIndex = pages.has(slug)
    return {
      type: 'group',
      slug,
      title: hasIndex
        ? pages.get(slug).title
        : (GROUP_TITLES[locale]?.[slug] ?? slug),
      hasIndex,
      children: orderSlugs(children, CHILD_ORDER).map(child => ({
        slug: child,
        title: navTitle(pages, child),
      })),
    }
  })
}

/** Every source page of a locale: { slug, sourcePath } (repo-relative). */
function collectSources(locale) {
  const sources = []
  const localeRoot = posix.join(docsDir, LOCALE_DIRS[locale])
  if (existsSync(join(repoRoot, localeRoot))) {
    for (const rel of collectMarkdownFiles(join(repoRoot, localeRoot))) {
      sources.push({ slug: fileToSlug(rel), sourcePath: posix.join(localeRoot, rel) })
    }
  }
  for (const binding of BINDINGS) {
    const file = binding.files[locale]
    if (file && existsSync(join(repoRoot, file))) {
      sources.push({ slug: binding.slug, sourcePath: file })
    }
  }
  return sources
}

async function compileLocale(locale, sources, routesByPath) {
  const pages = new Map()

  mkdirSync(join(outDir, 'pages', locale), { recursive: true })

  for (const { slug, sourcePath } of sources) {
    const source = readFileSync(join(repoRoot, sourcePath), 'utf8')
    const toc = []

    const processor = unified()
      .use(remarkParse)
      .use(remarkGfm)
      .use(remarkDocsTransforms, { sourcePath, routesByPath })
      .use(remarkRehype)
      .use(rehypeSlug)
      .use(rehypeShiki, {
        themes: { light: 'github-light', dark: 'github-dark' },
        defaultColor: 'light',
      })
      .use(rehypeExtractToc, { toc })
      .use(rehypeStringify)

    const mdast = processor.parse(source)
    const fallbackTitle = slug === '' ? 'Documentation' : posix.basename(slug)
    const { title, description } = extractTitleAndDescription(mdast, fallbackTitle)
    const hast = await processor.run(mdast)
    const html = processor.stringify(hast)

    const page = { slug, locale, title, description, html, toc, sourcePath }
    pages.set(slug, page)
    writeFileSync(
      join(outDir, 'pages', locale, slugToPageFile(slug)),
      JSON.stringify(page),
    )
  }

  return pages
}

async function main() {
  const sourcesByLocale = {}
  for (const locale of Object.keys(LOCALE_DIRS)) {
    const sources = collectSources(locale)
    if (sources.length > 0) sourcesByLocale[locale] = sources
  }
  if (!sourcesByLocale.en) {
    throw new Error(`No English docs found under ${docsDir}/en`)
  }

  // Map every source file to its route up front so links between pages (and
  // across languages) can be rewritten while compiling.
  const routesByPath = new Map()
  for (const [locale, sources] of Object.entries(sourcesByLocale)) {
    for (const { slug, sourcePath } of sources) {
      routesByPath.set(sourcePath, `${PATH_PREFIX[locale]}${slug ? `/${slug}` : ''}`)
    }
  }

  rmSync(outDir, { recursive: true, force: true })
  mkdirSync(join(outDir, 'pages'), { recursive: true })

  const pagesByLocale = {}
  let total = 0
  for (const [locale, sources] of Object.entries(sourcesByLocale)) {
    pagesByLocale[locale] = await compileLocale(locale, sources, routesByPath)
    total += pagesByLocale[locale].size
  }

  // A translation's nav also lists the English pages it lacks; the docs
  // routes serve those in English with a notice.
  const localeManifests = {}
  for (const [locale, pages] of Object.entries(pagesByLocale)) {
    const servable = new Map([...pagesByLocale.en, ...pages])
    localeManifests[locale] = {
      indexTitle: servable.get('')?.title ?? 'Documentation',
      nav: buildNav(locale, servable),
      slugs: [...servable.keys()].sort(),
    }
  }

  const manifest = { repo: REPO, ref: REF, locales: localeManifests }
  writeFileSync(join(outDir, 'manifest.json'), JSON.stringify(manifest, null, 2))

  console.log(
    `Compiled ${total} docs pages (${Object.keys(sourcesByLocale).join(', ')}) into src/content/docs-gen`,
  )
}

main().catch(error => {
  console.error(error)
  process.exit(1)
})
