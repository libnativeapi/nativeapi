/** `ShortcutScope` (`shortcut.h`). */
export type Scope = 'global' | 'application'

/** What the example's callback does when a shortcut fires. */
export type CallbackKind = 'counter' | 'print'

/** One `Shortcut` the manager holds, with what the example counted. */
export interface ShortcutEntry {
  id: number
  accelerator: string
  description: string
  scope: Scope
  enabled: boolean
  callback: CallbackKind
  /** ShortcutActivatedEvents for it. */
  activations: number
  /** Times its callback ran: activations and `invoke()`s. */
  calls: number
}

/** The options the Register tab fills in. */
export interface ShortcutDraft {
  accelerator: string
  description: string
  scope: Scope
  enabled: boolean
}

/** A `register()` that came back null, and the ShortcutRegistrationFailedEvent with it. */
export interface RegistrationFailure {
  id: number
  accelerator: string
  error: string
}

export type Tab = 'shortcut' | 'register' | 'manager'

/** A shortcut that just fired, for the flash on the desktop. */
export interface Firing {
  key: number
  accelerator: string
  description: string
}
