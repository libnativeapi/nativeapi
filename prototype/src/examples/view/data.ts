import type { Os } from '../../components/platform'
import type { AccentName, BoxSpec, ViewAlignmentName } from './types'

/**
 * The accents the header's button cycles through, in order. Each names the
 * DazzUI ramp it is drawn from (`view-view.css` binds it to `--view-accent`);
 * Pink has no ramp of its own and is mixed from red and brand.
 */
export const ACCENTS: readonly AccentName[] = ['Indigo', 'Teal', 'Orange', 'Pink', 'Green']

/** `View.getDefaultBackend()`: Native everywhere, which is a different toolkit per OS. */
export const BACKEND_NAMES: Record<Os, string> = {
  macos: 'AppKit',
  windows: 'Win32',
  linux: 'GTK3',
}

/** How `Platform.operatingSystem` names each OS, for the footer's status line. */
export const OS_NAMES: Record<Os, string> = {
  macos: 'macos',
  windows: 'windows',
  linux: 'linux',
}

export const DEFAULT_SUBTITLE = 'Labels, buttons, text fields and image views, laid out in rows and columns — no Flutter.'

export const MAX_TASKS = 5
export const NOTES_BUDGET = 140
/** The fake sign-in round trip, and how long a flashed view stays yellow. */
export const SIGN_IN_DELAY = 900
export const FLASH_DURATION = 900

/** The playground's four boxes: two fixed, two sharing what is left 1 : 2. */
export const BOXES: readonly BoxSpec[] = [
  { name: 'A', flex: 0, shade: 100 },
  { name: 'B', flex: 1, shade: 78 },
  { name: 'C', flex: 2, shade: 59 },
  { name: 'D', flex: 0, shade: 43 },
]

/** The size a box asks for: kept on the main axis when its flex is 0, and on the cross axis unless stretched. */
export const BOX_SIZE = { width: 64, height: 40 }
/**
 * The least the stage is given: it takes what its column leaves, so it grows
 * with the window and gives way to the taller controls of GTK and WinUI.
 */
export const STAGE_HEIGHT = 96

/** `ViewAlignment` in the order Align cycles it. */
export const ALIGNMENTS: readonly ViewAlignmentName[] = ['stretch', 'start', 'center', 'end']

/** The tasks of the "Signed in with tasks" scene. */
export const SAMPLE_TASKS = [
  { title: 'Write the view example', done: true },
  { title: 'Run it on macOS, Windows and Linux' },
  { title: 'Record a demo' },
] as const

export const FIND_HINT = 'Enter flashes the first match.'
