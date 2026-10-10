import type { Layout, MainId, PanelId, SlotId } from './types'

/** A main window's frame: the example's 840 × 600, scaled so two stand side by side. */
export const MAIN_SIZE = { width: 600, height: 440 }

/** Where the example places its two main windows: side by side, stepped down. */
export const MAIN_PLACES: Record<MainId, { x: number; y: number }> = {
  A: { x: 24, y: 24 },
  B: { x: 648, y: 64 },
}

/**
 * The slots. The two windows lay them out differently on purpose: a panel
 * takes the size and shape of whatever slot it is docked in, and keeps that
 * size when it is torn off.
 */
export const SLOTS: Record<SlotId, { window: MainId; label: string }> = {
  'A-left': { window: 'A', label: 'Sidebar' },
  'A-bottom': { window: 'A', label: 'Bottom panel' },
  'B-top': { window: 'B', label: 'Top strip' },
  'B-right': { window: 'B', label: 'Wide sidebar' },
}

export const SLOT_IDS = Object.keys(SLOTS) as SlotId[]

export const PANELS: Record<PanelId, { title: string }> = {
  inspector: { title: 'Inspector' },
  stopwatch: { title: 'Stopwatch' },
}

/** Where the example docks its panels at start, and where Dock back takes them first. */
export const HOME_SLOTS: Record<PanelId, SlotId> = { inspector: 'A-left', stopwatch: 'A-bottom' }

export const LAYOUTS = {
  /** As the example starts. */
  start: HOME_SLOTS,
  /** The stopwatch opened in a window of its own, the size of the bottom panel. */
  floating: { inspector: 'A-left', stopwatch: { x: 380, y: 540, width: 400, height: 170 } },
  /** Both panels moved into Window B, leaving Window A's slots empty. */
  windowB: { inspector: 'B-right', stopwatch: 'B-top' },
} satisfies Record<string, Layout>

export type LayoutName = keyof typeof LAYOUTS

/** How many layers the Inspector lists, so the list scrolls. */
export const LAYER_COUNT = 40

/** How far a docked panel's header must be dragged before the panel becomes a window. */
export const POP_OUT_DISTANCE = 8
