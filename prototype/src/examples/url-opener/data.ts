import type { Handler, UrlOpenErrorCode } from './types'

export const INITIAL_URL = 'https://flutter.dev'

/** The presets: each only fills the field. `note` names it in the sidebar. */
export const PRESETS: readonly { url: string; note: string }[] = [
  { url: 'https://flutter.dev', note: 'Web page' },
  { url: 'https://github.com/libnativeapi', note: 'Repository' },
  { url: 'mailto:hello@example.com', note: 'Mail address' },
  { url: 'tel:+15551234567', note: 'Phone number' },
  { url: 'file:///Users/me/Downloads', note: 'Folder' },
  { url: 'not a url', note: 'No scheme' },
  { url: 'gopher://example.com', note: 'Unknown scheme' },
]

/** The schemes the system has a handler for, and which app takes each. */
export const HANDLERS: Record<string, Handler> = {
  http: 'browser',
  https: 'browser',
  mailto: 'mail',
  tel: 'phone',
  file: 'files',
}

/** Every `UrlOpenErrorCode`, with when it comes back. */
export const ERROR_CODES: readonly { code: UrlOpenErrorCode; name: string; when: string }[] = [
  { code: 'none', name: 'kNone', when: 'The system took the URL.' },
  { code: 'invalidUrlEmpty', name: 'kInvalidUrlEmpty', when: 'The URL is an empty string.' },
  { code: 'invalidUrlMissingScheme', name: 'kInvalidUrlMissingScheme', when: 'No scheme before a colon: "not a url".' },
  {
    code: 'invalidUrlUnsupportedScheme',
    name: 'kInvalidUrlUnsupportedScheme',
    when: 'A scheme no application is registered for.',
  },
  { code: 'unsupportedPlatform', name: 'kUnsupportedPlatform', when: 'UrlOpener.isSupported() is false here.' },
  { code: 'invocationFailed', name: 'kInvocationFailed', when: 'The handler was found but refused or failed to start.' },
]

export const errorName = (code: UrlOpenErrorCode) => ERROR_CODES.find(entry => entry.code === code)?.name ?? code
