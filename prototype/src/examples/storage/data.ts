import type { Os } from '../../components/platform'
import type { Stores, StoreInfo, TestId } from './types'

export const STORES: readonly StoreInfo[] = [
  { id: 'preferences', kind: 'preferences', label: 'Default', scope: 'default', scoped: false },
  { id: 'scoped_preferences', kind: 'preferences', label: 'user_settings', scope: 'user_settings', scoped: true },
  { id: 'secure_storage', kind: 'secure', label: 'Default', scope: 'default', scoped: false },
  { id: 'scoped_secure_storage', kind: 'secure', label: 'api_credentials', scope: 'api_credentials', scoped: true },
]

export const storeOf = (id: string) => STORES.find(s => s.id === id) ?? STORES[0]!

/** How a call names the class it is made on. */
export const classOf = (info: StoreInfo) => (info.kind === 'secure' ? 'SecureStorage' : 'Preferences')

/** What each store holds when the example starts: a few runs' worth of settings and secrets. */
export const PLAYGROUND_ENTRIES: Stores = {
  preferences: {
    theme: 'dark',
    language: 'en-US',
    'window.width': '880',
    onboarding_done: 'true',
    last_opened: '2026-10-05T09:12:44Z',
  },
  scoped_preferences: {
    font_size: '14',
    sidebar: 'expanded',
    recent_files: '["notes.md","todo.txt"]',
  },
  secure_storage: {
    session_token: 'eyJhbGciOiJIUzI1NiJ9.e30.Kx9…',
  },
  scoped_secure_storage: {
    github_token: 'ghp_8f2Kq0vX7nLwR3aT1yZ5',
    openai_key: 'sk-proj-4Tq9Wm2…',
  },
}

export const EMPTY_ENTRIES: Stores = {
  preferences: {},
  scoped_preferences: {},
  secure_storage: {},
  scoped_secure_storage: {},
}

/** The test cases, in the order the Tests tab lists them. `all` ones act on every store, not the selected one. */
export const TESTS: readonly { id: TestId; name: string; does: string; all?: boolean }[] = [
  { id: 'basic', name: 'Basic operations', does: 'set, get, contains, remove, then contains again' },
  { id: 'bulk', name: 'Bulk operations', does: 'Sets bulk_key_1…10, then getKeys and getAll' },
  { id: 'special', name: 'Special characters', does: 'Emoji, CJK, symbols, accents and JSON round-trip' },
  { id: 'defaults', name: 'Default values', does: 'get on a missing key returns the default given' },
  { id: 'overwrite', name: 'Overwrite', does: 'A second set replaces the value' },
  { id: 'large', name: 'Large data', does: '100, 1 000 and 10 000 characters round-trip' },
  { id: 'empty', name: 'Empty values', does: 'An empty string is stored, and contains is true' },
  { id: 'scoped', name: 'Scoped storage isolation', does: 'Default and user_settings keep separate values', all: true },
  { id: 'compare', name: 'Compare types', does: 'One key in all four stores, sizes compared', all: true },
]

/** `special`'s cases: each must come back exactly as it went in. */
export const SPECIAL_CASES: readonly (readonly [string, string])[] = [
  ['emoji_key', '😀🎉🚀'],
  ['chinese_key', '你好世界'],
  ['special_chars', '@#$%^&*()'],
  ['unicode_key', 'Héllo Wörld'],
  ['json_like', '{"name":"value","array":[1,2,3]}'],
]

/**
 * What else a suite's `dictionaryRepresentation` holds on macOS: the
 * NSGlobalDomain and registration domain merged in. `getKeys()`, `getSize()`
 * and `getAll()` report them, and `clear()` tries to remove them.
 */
export const MACOS_GLOBAL_KEYS: readonly (readonly [string, string])[] = [
  ['AppleLanguages', '(en-US, zh-Hans-CN)'],
  ['AppleLocale', 'en_US'],
  ['AppleInterfaceStyle', 'Dark'],
  ['AppleKeyboardUIMode', '2'],
  ['NSInterfaceStyle', 'macintosh'],
  ['AKLastLocale', 'en_US'],
]

/** Where SecureStorage keeps its secrets on each desktop. */
export const SECURE_BACKENDS: Record<Os, string> = {
  macos: 'Keychain',
  windows: 'Credential Manager',
  linux: 'libsecret',
}

/** Where a store's entries live on disk or in the system, per desktop. */
export function locationOf(info: StoreInfo, os: Os): string {
  if (info.kind === 'secure') {
    if (os === 'macos') return `login keychain · service nativeapi.${info.scope}`
    if (os === 'windows') return `Credential Manager · nativeapi/${info.scope}/…`
    return `Login keyring · nativeapi ${info.scope}`
  }
  if (os === 'macos') return `~/Library/Preferences/com.nativeapi.preferences.${info.scope}.plist`
  if (os === 'windows') return `HKCU\\Software\\NativeAPI\\Preferences\\${info.scope}`
  return `~/.config/nativeapi/preferences_${info.scope}.conf`
}
