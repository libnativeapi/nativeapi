import { ArrowRightIcon, PencilLineIcon } from 'lucide-react'
import { Container } from '@/components/container'
import { SiteFooter } from '@/components/site-footer'
import { SiteHeader } from '@/components/site-header'
import { Button } from '@/components/ui/button'
import {
  Card,
  CardContent,
  CardDescription,
  CardHeader,
  CardTitle,
} from '@/components/ui/card'
import {
  docsPath,
  docsSourceUrl,
  getDocsNav,
  type DocsNavItem,
  type DocsPageData,
} from '@/lib/docs'
import { getMessages, localePath, localizedPath, type Locale } from '@/lib/i18n'
import { cn } from '@/lib/utils'
import { getDocsCopy } from './docs-copy'

function navLinkClass(active: boolean) {
  return cn(
    'block leading-5 transition-colors hover:text-foreground',
    active ? 'font-medium text-primary' : 'text-muted-foreground',
  )
}

function DocsNav({
  locale,
  activeSlug,
  docsHomeLabel,
}: {
  locale: Locale
  activeSlug: string
  docsHomeLabel: string
}) {
  const renderItem = (item: DocsNavItem) => {
    if (item.type === 'page') {
      return (
        <li key={item.slug}>
          <a
            className={navLinkClass(activeSlug === item.slug)}
            href={docsPath(locale, item.slug)}
          >
            {item.title}
          </a>
        </li>
      )
    }
    return (
      <li key={item.slug}>
        {item.hasIndex ? (
          <a
            className={cn(
              navLinkClass(activeSlug === item.slug),
              'font-medium',
              activeSlug === item.slug ? 'text-primary' : 'text-foreground',
            )}
            href={docsPath(locale, item.slug)}
          >
            {item.title}
          </a>
        ) : (
          <span className="block font-medium leading-5 text-foreground">
            {item.title}
          </span>
        )}
        <ul className="mt-3 flex flex-col gap-2.5 border-l pl-4">
          {item.children.map(child => (
            <li key={child.slug}>
              <a
                className={navLinkClass(activeSlug === child.slug)}
                href={docsPath(locale, child.slug)}
              >
                {child.title}
              </a>
            </li>
          ))}
        </ul>
      </li>
    )
  }

  return (
    <ul className="flex flex-col gap-3 text-sm">
      <li>
        <a className={navLinkClass(activeSlug === '')} href={docsPath(locale, '')}>
          {docsHomeLabel}
        </a>
      </li>
      {getDocsNav(locale).map(renderItem)}
    </ul>
  )
}

export function DocsPage({
  locale,
  page,
}: {
  locale: Locale
  page: DocsPageData
}) {
  const copy = getDocsCopy(locale)
  const siteCopy = getMessages(locale)

  // Compiled links carry the content language's route prefix. When English
  // content is served as a fallback under another locale, re-prefix its
  // internal links so readers stay inside their locale.
  const isFallbackContent = page.locale !== locale
  const html =
    isFallbackContent && page.locale === 'en' && locale !== 'en'
      ? page.html.replaceAll('href="/docs', `href="/${locale}/docs`)
      : page.html

  const tocItems = page.toc.filter(item => item.depth === 2)

  return (
    <div className="relative min-h-svh overflow-x-clip bg-background">
      <SiteHeader locale={locale} pagePath="/docs" />
      <div className="absolute inset-x-0 top-0 h-96 bg-linear-to-b from-primary/10 via-background to-background" />

      <main className="relative pt-(--header-height)">
        <Container className="pt-6 pb-16 md:pt-10 md:pb-24">
          <nav
            aria-label="Breadcrumb"
            className="mb-8 flex items-center gap-2 text-sm text-muted-foreground"
          >
            <a
              className="transition-colors hover:text-foreground"
              href={localePath(locale)}
            >
              {siteCopy.header.home}
            </a>
            <span aria-hidden="true">/</span>
            {page.slug === '' ? (
              <span className="text-foreground">{copy.breadcrumb}</span>
            ) : (
              <>
                <a
                  className="transition-colors hover:text-foreground"
                  href={docsPath(locale, '')}
                >
                  {copy.breadcrumb}
                </a>
                <span aria-hidden="true">/</span>
                <span className="text-foreground">{page.title}</span>
              </>
            )}
          </nav>

          <div className="grid gap-10 lg:grid-cols-[15rem_minmax(0,1fr)] lg:gap-14 xl:grid-cols-[15rem_minmax(0,1fr)_13rem]">
            <aside className="hidden lg:block">
              <nav
                aria-label={copy.navLabel}
                className="lg:sticky lg:top-[calc(var(--header-height)+2rem)]"
              >
                <p className="mb-4 text-xs font-semibold tracking-wide text-foreground uppercase">
                  {copy.navLabel}
                </p>
                <DocsNav
                  locale={locale}
                  activeSlug={page.slug}
                  docsHomeLabel={copy.docsHome}
                />
              </nav>
            </aside>

            <details className="group rounded-lg border px-4 py-3 lg:hidden">
              <summary className="cursor-pointer text-sm font-medium select-none">
                {copy.navLabel}
              </summary>
              <div className="pt-4">
                <DocsNav
                  locale={locale}
                  activeSlug={page.slug}
                  docsHomeLabel={copy.docsHome}
                />
              </div>
            </details>

            <div className="min-w-0">
              {isFallbackContent && copy.languageNotice ? (
                <p className="mb-8 rounded-lg border border-primary/30 bg-primary/5 px-4 py-3 text-sm text-muted-foreground">
                  {copy.languageNotice}
                </p>
              ) : null}

              <article
                className="docs-prose"
                dangerouslySetInnerHTML={{ __html: html }}
              />

              <div className="mt-10 border-t pt-6">
                <a
                  className="inline-flex items-center gap-2 text-sm text-muted-foreground transition-colors hover:text-foreground"
                  href={docsSourceUrl(page)}
                  rel="noreferrer"
                  target="_blank"
                >
                  <PencilLineIcon className="size-4" />
                  {copy.editOnGitHub}
                </a>
              </div>

              <Card className="mt-10">
                <CardHeader>
                  <CardTitle>{copy.supportLabel}</CardTitle>
                  <CardDescription>{copy.supportDescription}</CardDescription>
                </CardHeader>
                <CardContent>
                  <Button asChild>
                    <a href={localizedPath(locale, '/support')}>
                      {copy.supportAction}
                      <ArrowRightIcon data-icon="inline-end" />
                    </a>
                  </Button>
                </CardContent>
              </Card>
            </div>

            {tocItems.length > 0 ? (
              <aside className="hidden xl:block">
                <nav
                  aria-label={copy.onThisPage}
                  className="sticky top-[calc(var(--header-height)+2rem)]"
                >
                  <p className="mb-4 text-xs font-semibold tracking-wide text-foreground uppercase">
                    {copy.onThisPage}
                  </p>
                  <ol className="flex flex-col gap-2.5 border-l pl-4 text-sm">
                    {tocItems.map(item => (
                      <li key={item.id}>
                        <a
                          className="leading-5 text-muted-foreground transition-colors hover:text-foreground"
                          href={`#${item.id}`}
                        >
                          {item.text}
                        </a>
                      </li>
                    ))}
                  </ol>
                </nav>
              </aside>
            ) : null}
          </div>
        </Container>
      </main>

      <SiteFooter locale={locale} />
    </div>
  )
}
