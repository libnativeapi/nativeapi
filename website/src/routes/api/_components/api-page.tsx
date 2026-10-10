import { CodeIcon } from 'lucide-react'
import { useEffect, type ReactNode } from 'react'
import { Container } from '@/components/container'
import { SiteFooter } from '@/components/site-footer'
import { SiteHeader } from '@/components/site-header'
import { Badge } from '@/components/ui/badge'
import {
  apiIndex,
  apiLanguage,
  apiLanguageStorageKey,
  apiPath,
  type ApiDoc,
  type ApiItem,
  type ApiMember,
  type ApiModule,
  type ApiSignature,
} from '@/lib/api'
import { getMessages, githubUrl, localePath, type Locale } from '@/lib/i18n'
import { cn } from '@/lib/utils'
import { getApiCopy, type ApiCopy } from './api-copy'

function rememberLanguage(lang: string) {
  try {
    window.localStorage.setItem(apiLanguageStorageKey, lang)
  } catch {
    // Storage may be unavailable; the choice still lives in the URL.
  }
}

function LanguageSwitcher({
  locale,
  lang,
  module,
  label,
}: {
  locale: Locale
  lang: string
  module?: string
  label: string
}) {
  return (
    <div>
      <p className="mb-3 text-xs font-semibold tracking-wide text-foreground uppercase">
        {label}
      </p>
      <div className="flex flex-wrap gap-1.5">
        {apiIndex.languages.map(language => {
          const active = language.id === lang
          return (
            <a
              key={language.id}
              href={apiPath(locale, language.id, module)}
              aria-current={active ? 'page' : undefined}
              onClick={event => {
                rememberLanguage(language.id)
                // Keep the reader on the same symbol.
                if (window.location.hash) {
                  event.preventDefault()
                  window.location.assign(
                    `${apiPath(locale, language.id, module)}${window.location.hash}`,
                  )
                }
              }}
              className={cn(
                'rounded-md border px-2.5 py-1 text-xs font-medium transition-colors',
                active
                  ? 'border-primary bg-primary text-primary-foreground'
                  : 'bg-background text-muted-foreground hover:border-primary/40 hover:text-foreground',
              )}
            >
              {language.label}
            </a>
          )
        })}
      </div>
    </div>
  )
}

function ModuleNav({
  locale,
  lang,
  activeModule,
}: {
  locale: Locale
  lang: string
  activeModule?: string
}) {
  return (
    <ul className="flex flex-col gap-2 text-sm">
      {apiIndex.modules.map(module => {
        const active = module.id === activeModule
        return (
          <li key={module.id}>
            <a
              href={apiPath(locale, lang, module.id)}
              className={cn(
                'block font-mono text-[13px] leading-5 transition-colors hover:text-foreground',
                active ? 'font-semibold text-primary' : 'text-muted-foreground',
              )}
            >
              {module.header}
            </a>
            {active
              ? (
                  <ul className="mt-2 mb-1 flex flex-col gap-1.5 border-l pl-3">
                    {module.items.map(item => (
                      <li key={item.key}>
                        <a
                          href={`#${item.anchor}`}
                          className={cn(
                            'block leading-5 transition-colors hover:text-foreground',
                            item.languages.includes(lang)
                              ? 'text-muted-foreground'
                              : 'text-muted-foreground/50 line-through',
                          )}
                        >
                          {item.key}
                        </a>
                      </li>
                    ))}
                  </ul>
                )
              : null}
          </li>
        )
      })}
    </ul>
  )
}

export function ApiLayout({
  locale,
  lang,
  module,
  title,
  toc,
  children,
}: {
  locale: Locale
  lang: string
  module?: ApiModule
  title: ReactNode
  toc?: { anchor: string, label: string }[]
  children: ReactNode
}) {
  const copy = getApiCopy(locale)
  const siteCopy = getMessages(locale)
  const language = apiLanguage(lang)

  useEffect(() => rememberLanguage(lang), [lang])

  const sidebar = (
    <div className="flex flex-col gap-8">
      <LanguageSwitcher
        locale={locale}
        lang={lang}
        module={module?.id}
        label={copy.language}
      />
      <nav aria-label={copy.modules}>
        <p className="mb-3 text-xs font-semibold tracking-wide text-foreground uppercase">
          {copy.modules}
        </p>
        <ModuleNav locale={locale} lang={lang} activeModule={module?.id} />
      </nav>
    </div>
  )

  return (
    <div className="relative min-h-svh overflow-x-clip bg-background">
      <SiteHeader locale={locale} pagePath={module ? `/api/${lang}/${module.id}` : `/api/${lang}`} />
      <div className="absolute inset-x-0 top-0 h-96 bg-linear-to-b from-primary/10 via-background to-background" />

      <main className="relative pt-(--header-height)">
        <Container className="pt-6 pb-16 md:pt-10 md:pb-24">
          <nav
            aria-label="Breadcrumb"
            className="mb-8 flex flex-wrap items-center gap-2 text-sm text-muted-foreground"
          >
            <a className="transition-colors hover:text-foreground" href={localePath(locale)}>
              {siteCopy.header.home}
            </a>
            <span aria-hidden="true">/</span>
            <a
              className="transition-colors hover:text-foreground"
              href={apiPath(locale, lang)}
            >
              {copy.breadcrumb}
            </a>
            <span aria-hidden="true">/</span>
            {module
              ? (
                  <>
                    <span>{language?.label}</span>
                    <span aria-hidden="true">/</span>
                    <span className="font-mono text-foreground">{module.header}</span>
                  </>
                )
              : <span className="text-foreground">{language?.label}</span>}
          </nav>

          <div className="grid gap-10 lg:grid-cols-[15rem_minmax(0,1fr)] lg:gap-14 xl:grid-cols-[15rem_minmax(0,1fr)_13rem]">
            <aside className="hidden lg:block">
              <div className="sticky top-[calc(var(--header-height)+2rem)] max-h-[calc(100svh-var(--header-height)-4rem)] overflow-y-auto pr-2 pb-8">
                {sidebar}
              </div>
            </aside>

            <details className="group rounded-lg border px-4 py-3 lg:hidden">
              <summary className="cursor-pointer text-sm font-medium select-none">
                {copy.language}: {language?.label} · {copy.modules}
              </summary>
              <div className="pt-4">{sidebar}</div>
            </details>

            <div className="min-w-0">
              <h1 className="text-3xl font-bold tracking-tight md:text-4xl">{title}</h1>
              {copy.englishNotice
                ? <p className="mt-3 text-sm text-muted-foreground">{copy.englishNotice}</p>
                : null}
              {children}
            </div>

            {toc && toc.length > 0
              ? (
                  <aside className="hidden xl:block">
                    <nav
                      aria-label={copy.onThisPage}
                      className="sticky top-[calc(var(--header-height)+2rem)] max-h-[calc(100svh-var(--header-height)-4rem)] overflow-y-auto"
                    >
                      <p className="mb-4 text-xs font-semibold tracking-wide text-foreground uppercase">
                        {copy.onThisPage}
                      </p>
                      <ol className="flex flex-col gap-2.5 border-l pl-4 text-sm">
                        {toc.map(entry => (
                          <li key={entry.anchor}>
                            <a
                              className="leading-5 text-muted-foreground transition-colors hover:text-foreground"
                              href={`#${entry.anchor}`}
                            >
                              {entry.label}
                            </a>
                          </li>
                        ))}
                      </ol>
                    </nav>
                  </aside>
                )
              : null}
          </div>
        </Container>
      </main>

      <SiteFooter locale={locale} />
    </div>
  )
}

/* ------------------------------------------------------------------ */
/*  /api/<lang> — every module with its items                         */
/* ------------------------------------------------------------------ */

export function ApiOverviewPage({ locale, lang }: { locale: Locale, lang: string }) {
  const copy = getApiCopy(locale)
  const language = apiLanguage(lang)
  return (
    <ApiLayout locale={locale} lang={lang} title={copy.title}>
      <p className="mt-4 max-w-2xl text-lg leading-8 text-muted-foreground">{copy.description}</p>
      <h2 className="mt-12 text-xl font-semibold">{copy.overviewTitle}</h2>
      <p className="mt-2 text-muted-foreground">{copy.overviewDescription}</p>
      <div className="mt-6 grid gap-4 md:grid-cols-2">
        {apiIndex.modules.map(module => (
          <a
            key={module.id}
            href={apiPath(locale, lang, module.id)}
            className="group rounded-lg border bg-background p-5 transition-colors hover:border-primary/40"
          >
            <p className="font-mono text-sm font-semibold group-hover:text-primary">
              {module.header}
            </p>
            <ul className="mt-3 flex flex-col gap-1.5">
              {module.items.map(item => {
                const available = item.languages.includes(lang)
                return (
                  <li key={item.key} className="flex items-baseline gap-2 text-sm">
                    <span
                      className={cn(
                        'font-mono',
                        available ? 'text-foreground' : 'text-muted-foreground/50 line-through',
                      )}
                      title={available ? undefined : copy.unavailable.replace('{language}', language?.label ?? lang)}
                    >
                      {item.key}
                    </span>
                    <span className="text-xs text-muted-foreground">{copy.itemKinds[item.kind]}</span>
                  </li>
                )
              })}
            </ul>
          </a>
        ))}
      </div>
    </ApiLayout>
  )
}

/* ------------------------------------------------------------------ */
/*  /api/<lang>/<module> — one header's items in full                 */
/* ------------------------------------------------------------------ */

export function ApiModulePage({
  locale,
  lang,
  module,
}: {
  locale: Locale
  lang: string
  module: ApiModule
}) {
  const copy = getApiCopy(locale)
  return (
    <ApiLayout
      locale={locale}
      lang={lang}
      module={module}
      title={<span className="font-mono">{module.header}</span>}
      toc={module.items.map(item => ({ anchor: item.anchor, label: item.key }))}
    >
      <a
        href={`${githubUrl.replace('/nativeapi', '/nativeapi-core')}/blob/main/src/${module.header}`}
        target="_blank"
        rel="noreferrer"
        className="mt-3 inline-flex items-center gap-1.5 text-sm text-muted-foreground transition-colors hover:text-foreground"
      >
        <CodeIcon className="size-4" />
        {copy.source}
      </a>
      <div className="mt-6 flex flex-col">
        {module.items.map(item => (
          <ItemSection key={item.key} item={item} lang={lang} copy={copy} />
        ))}
      </div>
    </ApiLayout>
  )
}

function ItemSection({ item, lang, copy }: { item: ApiItem, lang: string, copy: ApiCopy }) {
  const signature = item.signatures[lang]
  const name = signature?.name ?? item.key
  const variants = item.members.filter(member => member.kind === 'variant' && item.kind === 'event')
  const members = item.members.filter(member => !(member.kind === 'variant' && item.kind === 'event') && !member.parent)
  return (
    <section id={item.anchor} className="scroll-mt-[calc(var(--header-height)+1.5rem)] border-t py-10 first:border-t-0">
      <div className="flex flex-wrap items-center gap-3">
        <h2 className="font-mono text-2xl font-semibold">
          <a href={`#${item.anchor}`} className="hover:text-primary">{name}</a>
        </h2>
        <Badge variant="secondary">{copy.itemKinds[item.kind]}</Badge>
        {name !== item.key ? <span className="font-mono text-xs text-muted-foreground">{item.key}</span> : null}
      </div>
      <Signature signature={signature} lang={lang} copy={copy} />
      <DocBlock doc={item.doc} copy={copy} />

      {members.length > 0
        ? (
            <div className="mt-8">
              <h3 className="mb-2 text-sm font-semibold tracking-wide text-muted-foreground uppercase">
                {copy.members}
              </h3>
              <div className="flex flex-col divide-y rounded-lg border">
                {members.map(member => (
                  <MemberRow key={member.key} member={member} lang={lang} copy={copy} />
                ))}
              </div>
            </div>
          )
        : null}

      {variants.length > 0
        ? (
            <div className="mt-8">
              <h3 className="mb-2 text-sm font-semibold tracking-wide text-muted-foreground uppercase">
                {copy.eventTypes}
              </h3>
              <div className="flex flex-col divide-y rounded-lg border">
                {variants.map(variant => (
                  <MemberRow
                    key={variant.key}
                    member={variant}
                    lang={lang}
                    copy={copy}
                    fields={item.members.filter(member => member.parent === variant.key)}
                  />
                ))}
              </div>
            </div>
          )
        : null}
    </section>
  )
}

function MemberRow({
  member,
  lang,
  copy,
  fields = [],
}: {
  member: ApiMember
  lang: string
  copy: ApiCopy
  fields?: ApiMember[]
}) {
  const signature = member.signatures[lang]
  const cppName = member.signatures.cpp?.name ?? member.key
  return (
    <div
      id={member.anchor}
      className={cn(
        'scroll-mt-[calc(var(--header-height)+1.5rem)] px-4 py-4 target:bg-primary/5 md:px-5',
        !signature && 'opacity-60',
      )}
    >
      <div className="flex flex-wrap items-center gap-2">
        <a href={`#${member.anchor}`} className="font-mono text-[15px] font-semibold hover:text-primary">
          {signature?.name ?? cppName}
        </a>
        <span className="rounded border px-1.5 py-px text-[11px] text-muted-foreground">
          {copy.memberKinds[member.kind]}
        </span>
      </div>
      <Signature signature={signature} lang={lang} copy={copy} compact />
      <DocBlock doc={member.doc} copy={copy} compact />
      {fields.length > 0
        ? (
            <div className="mt-3 flex flex-col gap-2 border-l pl-4">
              {fields.map(field => (
                <div key={field.key} id={field.anchor} className="scroll-mt-[calc(var(--header-height)+1.5rem)]">
                  <Signature signature={field.signatures[lang]} lang={lang} copy={copy} compact />
                  <DocBlock doc={field.doc} copy={copy} compact />
                </div>
              ))}
            </div>
          )
        : null}
    </div>
  )
}

function Signature({
  signature,
  lang,
  copy,
  compact = false,
}: {
  signature?: ApiSignature
  lang: string
  copy: ApiCopy
  compact?: boolean
}) {
  if (!signature) {
    const label = apiLanguage(lang)?.label ?? lang
    return (
      <p className={cn('text-sm text-muted-foreground italic', compact ? 'mt-2' : 'mt-4')}>
        {copy.unavailable.replace('{language}', label)}
      </p>
    )
  }
  return (
    <div
      className={cn('api-signature', compact ? 'mt-2' : 'mt-4')}
      dangerouslySetInnerHTML={{ __html: signature.html }}
    />
  )
}

function DocBlock({ doc, copy, compact = false }: { doc?: ApiDoc, copy: ApiCopy, compact?: boolean }) {
  if (!doc) return null
  return (
    <div className={cn('docs-prose api-doc', compact ? 'mt-2' : 'mt-5')}>
      {doc.summary ? <p dangerouslySetInnerHTML={{ __html: doc.summary }} /> : null}
      {doc.details ? <div dangerouslySetInnerHTML={{ __html: doc.details }} /> : null}
      {doc.params.length > 0
        ? (
            <dl className="api-params">
              <dt>{copy.parameters}</dt>
              {doc.params.map(param => (
                <dd key={param.name}>
                  <code>{param.name}</code>
                  {param.html ? <> — <span dangerouslySetInnerHTML={{ __html: param.html }} /></> : null}
                </dd>
              ))}
            </dl>
          )
        : null}
      {doc.returns
        ? (
            <dl className="api-params">
              <dt>{copy.returns}</dt>
              <dd dangerouslySetInnerHTML={{ __html: doc.returns }} />
            </dl>
          )
        : null}
      {doc.notes.map((note, index) => (
        <blockquote
          key={index}
          className={cn('docs-alert', note.kind === 'warning' || note.kind === 'deprecated' ? 'docs-alert-warning' : 'docs-alert-note')}
          data-alert={(copy.noteKinds[note.kind] ?? note.kind).toUpperCase()}
          dangerouslySetInnerHTML={{ __html: note.html ?? '' }}
        />
      ))}
    </div>
  )
}
