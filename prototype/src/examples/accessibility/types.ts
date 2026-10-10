export type Tab = 'status' | 'platforms'

/** Which window is in front on the desktop. */
export type Front = 'example' | 'settings'

/** An app in System Settings ▸ Privacy & Security ▸ Accessibility. */
export interface TrustedApp {
  name: string
  /** The example itself: the row its `enable()` adds. */
  self?: boolean
  allowed: boolean
}

/** A feature that only works for a trusted process, and what it says right now. */
export interface Dependent {
  id: 'keyboard' | 'shortcuts'
  name: string
  api: string
  /** Whether it needs the permission on this platform at all. */
  needs: boolean
  why: string
}
