import { useEffect, useRef, useState } from 'react'

import { DEFAULTS, KEYS, SCOPE } from './data'
import type { Launch, RunState, Store } from './types'

export interface PreferencesAppOptions {
  /** What the store holds before the app first starts: earlier runs' values, or nothing. */
  initialStore: Store
  /** Press Clear right after the first start, as the After clear story shows. */
  clearAtStart?: boolean
}

/** The keys a call just wrote, for the store's own view to pick out. */
export interface Changed {
  keys: string[]
  n: number
}

/**
 * The start of a run, as the app makes it: open the scoped store, read the
 * launch count and bump it, then read every other key with its default.
 */
function simulateStartup(store: Store): { store: Store; launch: Omit<Launch, 'number'>; count: number } {
  const previous = store['launch-count'] ?? DEFAULTS['launch-count']
  const count = Number(previous) + 1
  const startup = [
    `Preferences.createWithScope("${SCOPE}")`,
    `getScope() → "${SCOPE}"`,
    `get("launch-count", "0") → "${previous}"`,
    `set("launch-count", "${count}") → true`,
    ...KEYS.filter(k => k !== 'launch-count').map(k => `get("${k}", "${DEFAULTS[k]}") → "${store[k] ?? DEFAULTS[k]}"`),
  ]
  const restored = KEYS.filter(k => k !== 'launch-count' && k in store).length
  return { store: { ...store, 'launch-count': String(count) }, launch: { startup, restored }, count }
}

/**
 * The example as a process over a store that outlives it: the store is the
 * disk (or the registry), the window is one run. Relaunch quits the run and
 * starts the next; what the new run shows came back from the store.
 */
export function usePreferencesApp({ initialStore, clearAtStart }: PreferencesAppOptions) {
  const [first] = useState(() => simulateStartup(initialStore))
  const [store, setStore] = useState<Store>(() => (clearAtStart ? {} : first.store))
  const [launches, setLaunches] = useState<Launch[]>(() => [
    {
      number: first.count,
      startup: clearAtStart ? [...first.launch.startup, 'clear() → true', 'getSize() → 0'] : first.launch.startup,
      restored: first.launch.restored,
    },
  ])
  const [run, setRun] = useState<RunState>('running')
  const [changed, setChanged] = useState<Changed | null>(null)
  const storeRef = useRef(store)
  storeRef.current = store
  const timers = useRef<number[]>([])
  useEffect(() => () => timers.current.forEach(clearTimeout), [])

  const current = launches[0]!

  const set = (key: string, value: string) => {
    setStore(s => ({ ...s, [key]: value }))
    setChanged(c => ({ keys: [key], n: (c?.n ?? 0) + 1 }))
  }

  const clear = () => {
    setStore({})
    setChanged(c => ({ keys: [], n: (c?.n ?? 0) + 1 }))
  }

  const start = () => {
    // A timer's callback: the store as it is now, not as it was when Relaunch was pressed.
    const next = simulateStartup(storeRef.current)
    setStore(next.store)
    setLaunches(l => [{ number: next.count, ...next.launch }, ...l].slice(0, 8))
    setChanged(c => ({ keys: ['launch-count'], n: (c?.n ?? 0) + 1 }))
    setRun('running')
  }

  /** Quit, and start again a moment later: the window goes, then comes back. */
  const relaunch = () => {
    setRun('quitting')
    timers.current.push(
      window.setTimeout(() => setRun('starting'), 700),
      window.setTimeout(start, 1300),
    )
  }

  const quit = () => setRun('stopped')

  return { store, launches, current, run, changed, actions: { set, clear, relaunch, quit, start } }
}

export type PreferencesApp = ReturnType<typeof usePreferencesApp>
