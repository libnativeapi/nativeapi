import type { WindowKey } from './types'

/** The windows, in the order the example creates them: `ViewCollection([Tertiary, Secondary, Primary])`. */
export const WINDOWS: Record<WindowKey, { id: number; title: string; slot: string; description: string }> = {
  tertiary: {
    id: 1,
    title: 'Tertiary Window',
    slot: 'Bottom right',
    description: 'The right half of the bottom row, under the primary window.',
  },
  secondary: {
    id: 2,
    title: 'Secondary Window',
    slot: 'Bottom left',
    description: 'The left half of the bottom row, under the primary window.',
  },
  primary: {
    id: 3,
    title: 'Primary Window',
    slot: 'Top row',
    description:
      'The will-show hook gave this window the top half of a block 60% of the work area wide and tall, centred on the primary display.',
  },
}

export const CREATION_ORDER: readonly WindowKey[] = ['tertiary', 'secondary', 'primary']

/** The size every `RegularWindowController` asks for. */
export const DEFAULT_SIZE = { width: 800, height: 600 }

/** The size the primary window's button asks for, which the hook takes back. */
export const RESIZE_TO = { width: 1000, height: 1000 }

/** How far apart the platform cascades new windows it places itself. */
export const CASCADE_STEP = 28

/** The share of the work area the hook's block covers. */
export const BLOCK_SHARE = 0.6
