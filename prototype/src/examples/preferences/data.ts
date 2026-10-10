import type { Store, Theme } from './types'

/** The scope the example opens its store with: `Preferences.createWithScope("nativeapi-example")`. */
export const SCOPE = 'nativeapi-example'

/** The keys the form is bound to, with what `get` falls back to while one is missing. */
export const DEFAULTS = {
  greeting: 'Hello from C#!',
  'launch-count': '0',
  theme: 'system',
  'show-tips': 'true',
} as const satisfies Store

export type Key = keyof typeof DEFAULTS

export const KEYS = Object.keys(DEFAULTS) as Key[]

export const THEMES: readonly { value: Theme; label: string }[] = [
  { value: 'system', label: 'System' },
  { value: 'light', label: 'Light' },
  { value: 'dark', label: 'Dark' },
]

/** What earlier runs left: three launches, a greeting of the user's own, dark. */
export const PLAYGROUND_STORE: Store = {
  greeting: 'Hello again, Ada',
  'launch-count': '3',
  theme: 'dark',
  'show-tips': 'false',
}

/** After `clear()`: the store is there, empty. */
export const CLEARED_STORE: Store = {}
