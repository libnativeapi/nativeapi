import type { Os } from '../../components/platform'
import type { CallbackKind, Scope } from './types'

/** What the example registers as it starts: one with a bare callback, one with full options. */
export const SEEDS: readonly { accelerator: string; description: string; scope: Scope; callback: CallbackKind; withOptions: boolean }[] = [
  { accelerator: 'Ctrl+Shift+A', description: '', scope: 'global', callback: 'counter', withOptions: false },
  { accelerator: 'Ctrl+Shift+B', description: 'Second demo shortcut', scope: 'global', callback: 'print', withOptions: true },
]

/**
 * Combinations the system (or the desktop's shell) already holds: `isAvailable`
 * cannot see them — it only knows this app's own — so `register()` finds out,
 * and fails with "Platform registration failed".
 */
export const SYSTEM_TAKEN: Record<Os, Record<string, string>> = {
  macos: {
    'Cmd+Space': 'Spotlight',
    'Cmd+Tab': 'the app switcher',
    'Cmd+Shift+3': 'Screenshot',
    'Cmd+Shift+4': 'Screenshot of a selection',
    'Cmd+Shift+5': 'Screenshot and recording',
    'Ctrl+Up': 'Mission Control',
  },
  windows: {
    'Super+L': 'Lock',
    'Super+D': 'Show desktop',
    'Super+E': 'File Explorer',
    'Super+Shift+S': 'Snipping Tool',
    'Alt+Tab': 'Task switching',
    'Ctrl+Shift+Escape': 'Task Manager',
  },
  linux: {
    'Super+L': 'Lock screen',
    'Ctrl+Alt+T': 'Terminal',
    'Alt+Tab': 'Switch applications',
    'Alt+F2': 'Run a command',
    'Super+A': 'Show applications',
  },
}

/** A combination the system holds on each platform, for the presets and the Registration Failed story. */
export const TAKEN_SAMPLE: Record<Os, string> = { macos: 'Cmd+Space', windows: 'Super+L', linux: 'Super+L' }

/** The Register tab's quick fills. */
export function presetsOf(os: Os): readonly { accelerator: string; note: string }[] {
  return [
    { accelerator: 'Ctrl+Shift+C', note: 'Free' },
    { accelerator: os === 'macos' ? 'Cmd+Alt+K' : 'Ctrl+Alt+K', note: 'Free' },
    { accelerator: 'F8', note: 'A function key alone' },
    { accelerator: 'NotAKey', note: 'Invalid' },
    { accelerator: TAKEN_SAMPLE[os], note: 'Held by the system' },
    { accelerator: 'Ctrl+Shift+A', note: 'Already registered' },
  ]
}

export const SCOPES: readonly { value: Scope; label: string }[] = [
  { value: 'global', label: 'Global' },
  { value: 'application', label: 'Application' },
]

export const SCOPE_NAMES: Record<Scope, string> = { global: 'ShortcutScope::Global', application: 'ShortcutScope::Application' }
