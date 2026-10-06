/** The four stores the example opens, as the log names them. */
export type StoreId = 'preferences' | 'scoped_preferences' | 'secure_storage' | 'scoped_secure_storage'

/** `Preferences` (plain text, `preferences.h`) or `SecureStorage` (encrypted at rest, `secure_storage.h`). */
export type StoreKind = 'preferences' | 'secure'

export interface StoreInfo {
  id: StoreId
  kind: StoreKind
  /** The sidebar's name for it: Default, or the scope it was created with. */
  label: string
  /** What `getScope()` returns: `"default"` for the unscoped constructor. */
  scope: string
  /** Created with `createWithScope`, rather than `create`. */
  scoped: boolean
}

/** One store's entries, in the order they were set. */
export type Entries = Record<string, string>

export type Stores = Record<StoreId, Entries>

export type Tab = 'entries' | 'lookup' | 'tests' | 'backend'

export type TestId = 'basic' | 'bulk' | 'special' | 'defaults' | 'overwrite' | 'large' | 'empty' | 'scoped' | 'compare'

export type TestStatus = 'open' | 'pass' | 'fail'

export interface TestResult {
  status: TestStatus
  /** One line under the row: what the run found. */
  detail: string
  /** The calls the run made, for the log. */
  steps: string[]
}

/** What the Lookup tab last asked, and what came back. */
export interface Lookup {
  call: string
  result: string
  ok: boolean
}

/** A removed entry the Lookup tab can put back. */
export interface Removed {
  store: StoreId
  key: string
  value: string
}
