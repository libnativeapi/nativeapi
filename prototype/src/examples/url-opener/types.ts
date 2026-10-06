/** `UrlOpenErrorCode` (`url_opener.h`). */
export type UrlOpenErrorCode =
  | 'none'
  | 'invalidUrlEmpty'
  | 'invalidUrlMissingScheme'
  | 'invalidUrlUnsupportedScheme'
  | 'unsupportedPlatform'
  | 'invocationFailed'

/** `UrlOpenResult`: what `UrlOpener.open` returns. */
export interface UrlOpenResult {
  success: boolean
  errorCode: UrlOpenErrorCode
  errorMessage: string
}

/** The app the system hands a URL to, as the desktop draws it. */
export type Handler = 'browser' | 'mail' | 'phone' | 'files'

/** One `open` the example made, newest first in the sidebar. */
export interface OpenAttempt {
  number: number
  url: string
  result: UrlOpenResult
  handler: Handler | null
}

export type Tab = 'open' | 'errors'
