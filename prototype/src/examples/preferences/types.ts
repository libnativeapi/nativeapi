/** What the store holds: `Preferences` keeps strings only. */
export type Store = Record<string, string>

export type Theme = 'system' | 'light' | 'dark'

export type Tab = 'settings' | 'readback'

/** One run of the app, from start to quit. */
export interface Launch {
  number: number
  /** The calls the app made as it started, in order. */
  startup: string[]
  /** How many of the keys came back from the store rather than their defaults. */
  restored: number
}

/** Whether the app is up, on its way down, or on its way back. */
export type RunState = 'running' | 'quitting' | 'starting' | 'stopped'
