import { notFound } from '@tanstack/react-router'
import index from '@/content/api-gen/index.json'
import { locales, localizedPath, type Locale } from '@/lib/i18n'

/*
 * The API reference, compiled by scripts/build-api.mjs from the JSON that
 * ./codegen writes into content/api/. Signatures arrive as highlighted HTML,
 * doc comments as HTML rendered from their Markdown.
 */

export interface ApiLanguage {
  id: string
  label: string
}

export interface ApiItemSummary {
  key: string
  anchor: string
  kind: ApiItemKind
  summary?: string
  languages: string[]
}

export interface ApiModuleSummary {
  id: string
  header: string
  items: ApiItemSummary[]
}

export type ApiItemKind = 'class' | 'singleton' | 'struct' | 'enum' | 'event' | 'alias'

export type ApiMemberKind =
  | 'constructor'
  | 'method'
  | 'static'
  | 'listener'
  | 'field'
  | 'constant'
  | 'variant'

export interface ApiDoc {
  summary?: string
  details?: string
  params: { name: string, html?: string }[]
  returns?: string
  notes: { kind: string, html?: string }[]
}

export interface ApiSignature {
  name: string
  html: string
}

export interface ApiMember {
  key: string
  anchor: string
  kind: ApiMemberKind
  parent?: string
  doc?: ApiDoc
  signatures: Record<string, ApiSignature>
}

export interface ApiItem {
  key: string
  anchor: string
  kind: ApiItemKind
  doc?: ApiDoc
  signatures: Record<string, ApiSignature>
  members: ApiMember[]
}

export interface ApiModule {
  id: string
  header: string
  items: ApiItem[]
}

interface ApiIndex {
  languages: ApiLanguage[]
  modules: ApiModuleSummary[]
}

export const apiIndex = index as unknown as ApiIndex

/** The binding a bare /api link opens. */
export const defaultApiLanguage = 'dart'

export const apiLanguageStorageKey = 'nativeapi-api-language'

const moduleLoaders = import.meta.glob('../content/api-gen/modules/*.json', {
  import: 'default',
}) as Record<string, () => Promise<ApiModule>>

export function apiLanguage(id: string) {
  return apiIndex.languages.find(language => language.id === id)
}

export function isApiLanguage(id: string) {
  return apiLanguage(id) !== undefined
}

export async function loadApiModule(lang: string, id: string): Promise<ApiModule> {
  const loader = moduleLoaders[`../content/api-gen/modules/${id}.json`]
  if (!isApiLanguage(lang) || !loader) throw notFound()
  return loader()
}

export function apiPath(locale: Locale, lang?: string, module?: string, anchor?: string) {
  const path = ['/api', lang, module].filter(Boolean).join('/')
  return `${localizedPath(locale, path)}${anchor ? `#${anchor}` : ''}`
}

/** Where `key` (an item or member key) lives: its module and anchor. */
export function findApiSymbol(key: string) {
  const owner = key.split('::')[0]
  for (const module of apiIndex.modules) {
    const item = module.items.find(entry => entry.key === owner)
    if (item) return { module: module.id, anchor: item.anchor }
  }
  return undefined
}

/** Title, description and hreflang links of an API page. */
export function apiHead(locale: Locale, lang: string, module?: ApiModule) {
  const label = apiLanguage(lang)?.label ?? lang
  const title = module
    ? `${module.header} (${label}) — nativeapi API`
    : `${label} API reference — nativeapi`
  const item = module?.items[0]
  const description =
    item?.doc?.summary?.replace(/<[^>]+>/g, '') ??
    `Every class, method, enum and event of nativeapi as the ${label} binding spells it.`
  return {
    meta: [{ title }, { name: 'description', content: description }],
    links: locales.map(other => ({
      rel: 'alternate',
      hrefLang: other,
      href: apiPath(other, lang, module?.id),
    })),
  }
}
