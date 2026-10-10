/** The three `KeyboardEvent`s a `KeyboardMonitor` emits. */
export type KeyEventType = 'pressed' | 'released' | 'modifiers'

/** One event as the monitor delivered it. */
export interface KeyEventRecord {
  /** `#1`, `#2`… across the run. */
  number: number
  type: KeyEventType
  /** `GetKeycode()`: the platform's raw code; 0 for ModifierKeysChangedEvent. */
  keycode: number
  /** The key's name, from the page's `KeyboardEvent.code`, for the reader. */
  key: string
  /** `GetModifierKeys()` for ModifierKeysChangedEvent. */
  modifiers: number
  repeat: boolean
  /** `performance.now()` when it arrived. */
  at: number
}

/** Why `start()` left `isMonitoring` false. */
export type Blocked = 'permission' | null

export type Tab = 'live' | 'stream' | 'keycodes'

/** Which window has the keyboard focus on the desktop. */
export type Focus = 'example' | 'notes'
