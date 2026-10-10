#!/usr/bin/env node
/**
 * Compile the API reference into JSON consumed by the /api routes.
 *
 * Source: website/content/api/ — written by `./codegen` from the core headers
 * (index.json + one <module>.json per header, with doc comments as Markdown and
 * every binding's signatures as plain text). Never edit those by hand.
 *
 * Output (gitignored, regenerated before dev/build):
 *   src/content/api-gen/index.json             languages + module/item navigation
 *   src/content/api-gen/modules/<module>.json  items with doc HTML and
 *                                              highlighted signatures
 */
import { existsSync, mkdirSync, readFileSync, readdirSync, rmSync, writeFileSync } from 'node:fs'
import { dirname, join } from 'node:path'
import { fileURLToPath } from 'node:url'
import rehypeShiki from '@shikijs/rehype'
import rehypeStringify from 'rehype-stringify'
import remarkGfm from 'remark-gfm'
import remarkParse from 'remark-parse'
import remarkRehype from 'remark-rehype'
import { createHighlighter } from 'shiki'
import { unified } from 'unified'

const appDir = join(dirname(fileURLToPath(import.meta.url)), '..')
const sourceDir = join(appDir, 'content/api')
const outDir = join(appDir, 'src/content/api-gen')

const THEMES = { light: 'github-light', dark: 'github-dark' }

/** reference language id -> Shiki grammar */
const GRAMMARS = {
  cpp: 'cpp',
  dart: 'dart',
  rust: 'rust',
  csharp: 'csharp',
  js: 'typescript',
  python: 'python',
  go: 'go',
}

/** `Window::SetTitle` -> `window-settitle`; `Application::Run/1` -> `application-run-1`. */
function anchor(key) {
  return key
    .toLowerCase()
    .replace(/::|\//g, '-')
    .replace(/[^a-z0-9-]/g, '')
}

const markdown = unified()
  .use(remarkParse)
  .use(remarkGfm)
  .use(remarkRehype)
  .use(rehypeShiki, { themes: THEMES, defaultColor: 'light' })
  .use(rehypeStringify)

async function md(text) {
  if (!text) return undefined
  return String(await markdown.process(text))
}

/** Markdown for a one-paragraph slot, without the wrapping <p>. */
async function mdInline(text) {
  const html = await md(text)
  return html?.replace(/^<p>([\s\S]*)<\/p>\n?$/, '$1')
}

async function compileDoc(doc) {
  if (!doc) return undefined
  return {
    summary: await mdInline(doc.summary),
    details: await md(doc.details),
    params: await Promise.all(
      (doc.params ?? []).map(async param => ({
        name: param.name,
        html: await mdInline(param.text),
      })),
    ),
    returns: await mdInline(doc.returns),
    notes: await Promise.all(
      (doc.notes ?? []).map(async note => ({ kind: note.kind, html: await md(note.text) })),
    ),
  }
}

async function main() {
  if (!existsSync(join(sourceDir, 'index.json'))) {
    throw new Error(`No API reference at ${sourceDir}; run ./codegen at the repository root.`)
  }
  const index = JSON.parse(readFileSync(join(sourceDir, 'index.json'), 'utf8'))
  const highlighter = await createHighlighter({
    themes: Object.values(THEMES),
    langs: [...new Set(Object.values(GRAMMARS))],
  })
  const highlight = (lang, code) =>
    highlighter.codeToHtml(code, {
      lang: GRAMMARS[lang] ?? 'text',
      themes: THEMES,
      defaultColor: 'light',
    })

  const compileSignatures = signatures =>
    Object.fromEntries(
      Object.entries(signatures).map(([lang, entry]) => [
        lang,
        { name: entry.name, html: highlight(lang, entry.signature) },
      ]),
    )

  rmSync(outDir, { recursive: true, force: true })
  mkdirSync(join(outDir, 'modules'), { recursive: true })

  const moduleFiles = readdirSync(sourceDir).filter(
    file => file.endsWith('.json') && file !== 'index.json',
  )
  let symbols = 0
  for (const file of moduleFiles) {
    const module = JSON.parse(readFileSync(join(sourceDir, file), 'utf8'))
    const items = []
    for (const item of module.items) {
      const members = []
      for (const member of item.members ?? []) {
        members.push({
          key: member.key,
          anchor: anchor(member.key),
          kind: member.kind,
          parent: member.parent,
          doc: await compileDoc(member.doc),
          signatures: compileSignatures(member.signatures),
        })
      }
      symbols += 1 + members.length
      items.push({
        key: item.key,
        anchor: anchor(item.key),
        kind: item.kind,
        doc: await compileDoc(item.doc),
        signatures: compileSignatures(item.signatures),
        members,
      })
    }
    writeFileSync(
      join(outDir, 'modules', file),
      JSON.stringify({ id: module.id, header: module.header, items }),
    )
  }

  writeFileSync(
    join(outDir, 'index.json'),
    JSON.stringify({
      languages: index.languages,
      modules: index.modules.map(module => ({
        ...module,
        items: module.items.map(item => ({ ...item, anchor: anchor(item.key) })),
      })),
    }),
  )
  console.log(
    `Compiled the API reference (${index.modules.length} modules, ${symbols} symbols) into src/content/api-gen`,
  )
}

main().catch(error => {
  console.error(error)
  process.exit(1)
})
