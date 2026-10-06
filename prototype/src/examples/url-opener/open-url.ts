import type { Os } from '../../components/platform'
import { HANDLERS } from './data'
import type { Handler, UrlOpenResult } from './types'

/** The scheme of a URL, or null when it has none. */
export function schemeOf(url: string): string | null {
  const match = /^([a-z][a-z0-9+.-]*):/i.exec(url.trim())
  return match ? match[1]!.toLowerCase() : null
}

/** `UrlOpener.canOpen`: a well-formed URL whose scheme has a handler. */
export function simulateCanOpen(url: string, os: Os): boolean {
  const scheme = schemeOf(url)
  return scheme !== null && scheme in HANDLERS && !(scheme === 'tel' && os === 'linux')
}

/**
 * `UrlOpener.open`, simulated: the checks core makes before it hands the URL
 * over (`NSWorkspace openURL:`, `ShellExecuteW`, `xdg-open`), and what the
 * handler does with it.
 */
export function simulateOpen(url: string, os: Os): { result: UrlOpenResult; handler: Handler | null } {
  const fail = (errorCode: UrlOpenResult['errorCode'], errorMessage: string) => ({
    result: { success: false, errorCode, errorMessage },
    handler: null,
  })
  if (url.trim() === '') return fail('invalidUrlEmpty', 'URL is empty')
  const scheme = schemeOf(url)
  if (scheme === null) return fail('invalidUrlMissingScheme', 'URL has no scheme')
  if (!(scheme in HANDLERS)) return fail('invalidUrlUnsupportedScheme', `No application handles ${scheme}:`)
  if (scheme === 'tel' && os === 'linux') return fail('invocationFailed', 'xdg-open exited with status 3')
  return { result: { success: true, errorCode: 'none', errorMessage: '' }, handler: HANDLERS[scheme]! }
}
