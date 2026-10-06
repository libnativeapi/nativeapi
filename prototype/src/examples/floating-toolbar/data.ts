import type { Point, Swatch } from './types'

export const SWATCHES: readonly { value: Swatch; label: string; color: string }[] = [
  { value: 'primary', label: 'Primary', color: 'var(--color-primary-500)' },
  { value: 'green', label: 'Green', color: 'var(--color-success-500)' },
  { value: 'amber', label: 'Amber', color: 'var(--color-warning-500)' },
  { value: 'red', label: 'Red', color: 'var(--color-danger-500)' },
]

export const swatchOf = (value: Swatch) => SWATCHES.find(s => s.value === value)!

export const MAIN_SIZE = { width: 720, height: 520 }
export const TOOLBAR_SIZE = { width: 380, height: 64 }

/** How far the toolbar floats above the main window's top edge. */
export const TOOLBAR_GAP = 10

/** Where the main window starts on a ~1350 × 730 desktop: centred, with room above for the toolbar. */
export const MAIN_AT: Point = { x: 315, y: 110 }

/**
 * Where Hyprland puts the toolbar: a client cannot place its windows on
 * Wayland, and the compositor centres a new floating window on the screen.
 */
export const WAYLAND_TOOLBAR_AT: Point = { x: 485, y: 330 }

/** The toolbar centred above a main window at `main` (`_placeToolbar`). */
export function toolbarAbove(main: Point): Point {
  return {
    x: main.x + (MAIN_SIZE.width - TOOLBAR_SIZE.width) / 2,
    y: main.y - TOOLBAR_SIZE.height - TOOLBAR_GAP,
  }
}
