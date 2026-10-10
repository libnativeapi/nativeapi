import { useCallback, useMemo, useState } from 'react'

import { useEventLog } from '../../components/event-bar'
import type { Os } from '../../components/platform'
import { MACOS_GLOBAL_KEYS, SPECIAL_CASES, STORES, storeOf, TESTS } from './data'
import type { Entries, Lookup, Removed, StoreId, StoreInfo, Stores, TestId, TestResult } from './types'

/** How the log names each instance: the fields the Flutter example keeps them in. */
export const INSTANCE: Record<StoreId, string> = {
  preferences: 'preferences',
  scoped_preferences: 'scopedPreferences',
  secure_storage: 'secureStorage',
  scoped_secure_storage: 'scopedSecureStorage',
}

const quote = (s: string) => `"${s.length > 40 ? `${s.slice(0, 37)}…` : s}"`

export interface StorageOptions {
  os: Os
  initial: Stores
  initialStore?: StoreId
  /**
   * What `SecureStorage.isAvailable()` returns. While it is false the secure
   * stores behave as core's stub does: `set` returns false and nothing is kept.
   */
  secureAvailable?: boolean
  /** Run every test as the example starts. */
  runTestsAtStart?: boolean
}

/** The keys a call just touched, for the Backend tab to pick out. */
export interface Changed {
  store: StoreId
  keys: string[]
  /** Bumps on every call, so the same key changing twice still flashes. */
  n: number
}

/** `entries` without `key`. */
function without(entries: Entries, key: string): Entries {
  const copy = { ...entries }
  delete copy[key]
  return copy
}

const OPEN: TestResult = { status: 'open', detail: '', steps: [] }
const openTests = () => Object.fromEntries(TESTS.map(t => [t.id, OPEN])) as Record<TestId, TestResult>

/**
 * The store operations over a copy of every store, as core runs them: a
 * store whose backend is not available refuses every write.
 */
function opsOver(source: Stores, usable: (id: StoreId) => boolean) {
  const stores: Stores = { ...source }
  return {
    stores,
    set(id: StoreId, key: string, value: string) {
      if (!usable(id)) return false
      stores[id] = { ...stores[id], [key]: value }
      return true
    },
    get(id: StoreId, key: string, fallback: string) {
      const entries = stores[id]
      return usable(id) && key in entries ? entries[key]! : fallback
    },
    contains(id: StoreId, key: string) {
      return usable(id) && key in stores[id]
    },
    remove(id: StoreId, key: string) {
      if (!usable(id) || !(key in stores[id])) return false
      stores[id] = without(stores[id], key)
      return true
    },
    size(id: StoreId) {
      return usable(id) ? Object.keys(stores[id]).length : 0
    },
  }
}

type Ops = ReturnType<typeof opsOver>

/** One test case against `id` (or every store, for the `all` ones): the calls it made and whether they held. */
function runCase(test: TestId, id: StoreId, o: Ops): TestResult {
  const steps: string[] = []
  const name = INSTANCE[id]
  const log = (line: string) => steps.push(line)
  const result = (pass: boolean, detail: string): TestResult => ({ status: pass ? 'pass' : 'fail', detail, steps })

  switch (test) {
    case 'basic': {
      const set = o.set(id, 'test_key', 'test_value')
      log(`${name}.set("test_key", "test_value") → ${set}`)
      const got = o.get(id, 'test_key', '')
      log(`${name}.get("test_key") → ${quote(got)}`)
      const has = o.contains(id, 'test_key')
      log(`${name}.contains("test_key") → ${has}`)
      const removed = o.remove(id, 'test_key')
      log(`${name}.remove("test_key") → ${removed}`)
      const still = o.contains(id, 'test_key')
      log(`${name}.contains("test_key") → ${still}`)
      return result(
        set && got === 'test_value' && has && removed && !still,
        `get → ${quote(got)} · contains → ${has} · after remove → ${still}`,
      )
    }
    case 'bulk': {
      let ok = 0
      for (let i = 1; i <= 10; i++) if (o.set(id, `bulk_key_${i}`, `value_${i}`)) ok++
      log(`${name}.set("bulk_key_1…10") → ${ok}/10 true`)
      const keys = o.size(id)
      log(`${name}.getKeys() → ${keys} keys`)
      log(`${name}.getAll() → ${keys} entries`)
      return result(ok === 10 && keys >= 10, `10 set → ${ok} true · getKeys → ${keys} · getAll → ${keys}`)
    }
    case 'special': {
      let matched = 0
      for (const [key, value] of SPECIAL_CASES) {
        o.set(id, key, value)
        const back = o.get(id, key, '')
        if (back === value) matched++
        log(`${back === value ? '✓' : '✗'} ${name}.get("${key}") → ${quote(back)}`)
      }
      return result(matched === SPECIAL_CASES.length, `${matched}/${SPECIAL_CASES.length} came back unchanged`)
    }
    case 'defaults': {
      const a = o.get(id, 'non_existent_key', 'default_value')
      log(`${name}.get("non_existent_key", "default_value") → ${quote(a)}`)
      const b = o.get(id, 'another_non_existent_key', '')
      log(`${name}.get("another_non_existent_key", "") → ${quote(b)}`)
      return result(a === 'default_value' && b === '', `missing → ${quote(a)} · without default → ${quote(b)}`)
    }
    case 'overwrite': {
      o.set(id, 'overwrite_test', 'original_value')
      const a = o.get(id, 'overwrite_test', '')
      o.set(id, 'overwrite_test', 'updated_value')
      const b = o.get(id, 'overwrite_test', '')
      o.remove(id, 'overwrite_test')
      log(`${name}.get("overwrite_test") → ${quote(a)}, then ${quote(b)}`)
      return result(a === 'original_value' && b === 'updated_value', `${quote(a)} → ${quote(b)} · cleaned up`)
    }
    case 'large': {
      const lengths = [100, 1000, 10000].map(size => {
        const key = `large_data_${size}`
        const set = o.set(id, key, 'x'.repeat(size))
        const back = o.get(id, key, '').length
        o.remove(id, key)
        log(`${back === size ? '✓' : '✗'} ${size} chars: set → ${set}, read ${back}`)
        return back === size
      })
      return result(lengths.every(Boolean), `${lengths.filter(Boolean).length}/3 sizes read back whole`)
    }
    case 'empty': {
      o.set(id, 'empty_value', '')
      const back = o.get(id, 'empty_value', 'default')
      const has = o.contains(id, 'empty_value')
      o.remove(id, 'empty_value')
      log(`${name}.get("empty_value", "default") → ${quote(back)} · contains → ${has}`)
      return result(back === '' && has, `get → ${quote(back)} (length ${back.length}) · contains → ${has}`)
    }
    case 'scoped': {
      o.set('preferences', 'shared_key', 'regular_value')
      o.set('scoped_preferences', 'shared_key', 'scoped_value')
      const a = o.get('preferences', 'shared_key', '')
      const b = o.get('scoped_preferences', 'shared_key', '')
      o.remove('preferences', 'shared_key')
      o.remove('scoped_preferences', 'shared_key')
      log(`preferences.get("shared_key") → ${quote(a)}`)
      log(`scopedPreferences.get("shared_key") → ${quote(b)}`)
      return result(a === 'regular_value' && b === 'scoped_value', `Default → ${quote(a)} · user_settings → ${quote(b)}`)
    }
    case 'compare': {
      const sets = STORES.map(s => o.set(s.id, 'comparison_test', 'test_value_123'))
      const sizes = STORES.map(s => o.size(s.id))
      for (const s of STORES) o.remove(s.id, 'comparison_test')
      STORES.forEach((s, i) => log(`${INSTANCE[s.id]}.set("comparison_test") → ${sets[i]} · getSize() → ${sizes[i]}`))
      return result(
        sets.every(Boolean),
        `set → ${sets.filter(Boolean).length}/4 true · sizes ${sizes.join(' / ')}`,
      )
    }
  }
}

/**
 * The storage example's four stores, simulated: what each holds, the calls
 * the window makes on the selected one, the test cases, and the log.
 */
export function useStorage({ os, initial, initialStore = 'preferences', secureAvailable = true, runTestsAtStart }: StorageOptions) {
  const usable = useCallback((id: StoreId) => storeOf(id).kind === 'preferences' || secureAvailable, [secureAvailable])

  // Run the tests once up front, for the story that starts with them done.
  const start = useMemo(() => {
    if (!runTestsAtStart) return { stores: initial, tests: openTests() }
    const o = opsOver(initial, usable)
    const tests = Object.fromEntries(TESTS.map(t => [t.id, runCase(t.id, initialStore, o)])) as Record<TestId, TestResult>
    return { stores: o.stores, tests }
    // Once, as the example starts.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [])

  const log = useEventLog(
    [
      'Preferences.create()',
      'Preferences.createWithScope("user_settings")',
      'SecureStorage.create()',
      'SecureStorage.createWithScope("api_credentials")',
      `SecureStorage.isAvailable() → ${secureAvailable}`,
    ],
    'Storage opened',
  )
  const { event, call } = log
  const [stores, setStores] = useState<Stores>(start.stores)
  const [selected, setSelected] = useState<StoreId>(initialStore)
  const [tests, setTests] = useState(start.tests)
  const [lookup, setLookup] = useState<Lookup | null>(null)
  const [removed, setRemoved] = useState<Removed | null>(null)
  const [changed, setChanged] = useState<Changed | null>(null)

  const info = storeOf(selected)
  const name = INSTANCE[selected]
  const entries = stores[selected]

  const touch = (store: StoreId, keys: string[]) => setChanged(c => ({ store, keys, n: (c?.n ?? 0) + 1 }))

  /**
   * What `getKeys`, `getSize` and `getAll` see. On macOS a suite's
   * `dictionaryRepresentation` merges in NSGlobalDomain, so a Preferences
   * store reports keys it never set.
   */
  const mergedView = (store: StoreInfo, own: Entries): Entries =>
    os === 'macos' && store.kind === 'preferences' ? { ...own, ...Object.fromEntries(MACOS_GLOBAL_KEYS) } : own

  const select = (id: StoreId) => {
    if (id === selected) return
    setSelected(id)
    setLookup(null)
    call(`Switched to ${INSTANCE[id]}`)
  }

  const set = (key: string, value: string) => {
    const k = key.trim()
    if (!k) return false
    const ok = usable(selected)
    if (ok) {
      setStores(s => ({ ...s, [selected]: { ...s[selected], [k]: value } }))
      touch(selected, [k])
    }
    event(ok ? `Set "${k}"` : `Could not set "${k}"`, `${name}.set(${quote(k)}, ${quote(value)}) → ${ok}`)
    return ok
  }

  const get = (key: string, fallback: string) => {
    const value = usable(selected) && key in entries ? entries[key]! : fallback
    const line = `${name}.get(${quote(key)}${fallback ? `, ${quote(fallback)}` : ''}) → ${quote(value)}`
    setLookup({ call: `get(${quote(key)}${fallback ? `, ${quote(fallback)}` : ''})`, result: quote(value), ok: key in entries })
    event(`get("${key}") → ${quote(value)}`, line)
  }

  const contains = (key: string) => {
    const has = usable(selected) && key in entries
    setLookup({ call: `contains(${quote(key)})`, result: String(has), ok: has })
    event(`contains("${key}") → ${has}`, `${name}.contains(${quote(key)}) → ${has}`)
  }

  const remove = (key: string) => {
    const has = usable(selected) && key in entries
    if (has) {
      const previous = entries[key]!
      setStores(s => ({ ...s, [selected]: without(s[selected], key) }))
      setRemoved({ store: selected, key, value: previous })
    }
    setLookup({ call: `remove(${quote(key)})`, result: String(has), ok: has })
    event(has ? `Removed "${key}"` : `Nothing to remove at "${key}"`, `${name}.remove(${quote(key)}) → ${has}`)
  }

  const undoRemove = () => {
    if (!removed) return
    const { store, key, value } = removed
    setStores(s => ({ ...s, [store]: { ...s[store], [key]: value } }))
    touch(store, [key])
    setRemoved(null)
    event(`Put "${key}" back`, `${INSTANCE[store]}.set(${quote(key)}, ${quote(value)}) → true`)
  }

  const clear = () => {
    const count = Object.keys(entries).length
    const ok = usable(selected)
    if (ok) setStores(s => ({ ...s, [selected]: {} }))
    setRemoved(null)
    event(ok ? `Cleared ${count} entries` : 'Could not clear', `${name}.clear() → ${ok}`)
    if (ok && os === 'macos' && info.kind === 'preferences') {
      call(`  also tried ${MACOS_GLOBAL_KEYS.length} NSGlobalDomain keys, which belong to no suite`)
    }
  }

  const listKeys = () => {
    const keys = usable(selected) ? Object.keys(mergedView(info, entries)) : []
    setLookup({ call: 'getKeys()', result: `[${keys.map(k => `"${k}"`).join(', ')}]`, ok: true })
    event(`getKeys() → ${keys.length} keys`, `${name}.getKeys() → ${keys.length} keys`)
  }

  const getSize = () => {
    const size = usable(selected) ? Object.keys(mergedView(info, entries)).length : 0
    setLookup({ call: 'getSize()', result: String(size), ok: true })
    event(`getSize() → ${size}`, `${name}.getSize() → ${size}`)
  }

  const getAll = () => {
    const all = usable(selected) ? mergedView(info, entries) : {}
    const pairs = Object.entries(all)
    setLookup({ call: 'getAll()', result: `{${pairs.map(([k, v]) => `"${k}": ${quote(v)}`).join(', ')}}`, ok: true })
    event(`getAll() → ${pairs.length} entries`, `${name}.getAll() → ${pairs.length} entries`)
  }

  const runTest = (id: TestId) => {
    const o = opsOver(stores, usable)
    const before = stores
    const result = runCase(id, selected, o)
    setStores(o.stores)
    setTests(t => ({ ...t, [id]: result }))
    // The keys a test left behind, for the Backend tab.
    const left = Object.keys(o.stores[selected]).filter(k => !(k in before[selected]))
    if (left.length > 0) touch(selected, left)
    const test = TESTS.find(t => t.id === id)!
    for (const step of result.steps) call(step)
    event(`${test.name} ${result.status === 'pass' ? 'passed' : 'failed'}`)
  }

  const runAll = () => {
    const o = opsOver(stores, usable)
    const results = Object.fromEntries(TESTS.map(t => [t.id, runCase(t.id, selected, o)])) as Record<TestId, TestResult>
    setStores(o.stores)
    setTests(results)
    const passed = Object.values(results).filter(r => r.status === 'pass').length
    call(`Ran ${TESTS.length} tests on ${name}`)
    event(`${passed}/${TESTS.length} tests passed`)
  }

  const resetTests = () => setTests(openTests())

  return {
    os,
    secureAvailable,
    stores,
    selected,
    info,
    entries,
    tests,
    lookup,
    removed: removed?.store === selected ? removed : null,
    changed: changed?.store === selected ? changed : null,
    log,
    usable: usable(selected),
    mergedView,
    actions: { select, set, get, contains, remove, undoRemove, clear, listKeys, getSize, getAll, runTest, runAll, resetTests },
  }
}

export type StorageState = ReturnType<typeof useStorage>
export type StorageActions = StorageState['actions']
