import type { Scene } from './types'

/** A new window's frame: the example's 760 × 520, scaled to fit two on the stage. */
export const WINDOW_SIZE = { width: 600, height: 400 }

/** The theme ramps a tab's dot and paragraphs take, by `hue`. */
export const TAB_RAMPS = ['primary', 'success', 'warning', 'danger', 'info', 'neutral'] as const

export const tabColor = (hue: number) => `var(--color-${TAB_RAMPS[hue % TAB_RAMPS.length]}-500)`

/** How many placeholder paragraphs a page has, so it scrolls. */
export const PARAGRAPHS = 24

/** One window of a scene: its frame and how many tabs it opens with. */
export interface SceneWindow {
  x: number
  y: number
  width?: number
  height?: number
  tabs: number
  /** Count the tabs as already moved once: a window torn off before the story starts. */
  moved?: boolean
}

/**
 * The scenes: the example's two windows with four and two tabs, placed side
 * by side and stepped down as the example places them; one wide window with
 * many tabs; and the same two after a tab was torn off into a third.
 */
export const SCENES: Record<Scene, readonly SceneWindow[]> = {
  twoWindows: [
    { x: 40, y: 32, tabs: 4 },
    { x: 690, y: 92, tabs: 2 },
  ],
  manyTabs: [{ x: 120, y: 40, width: 980, height: 460, tabs: 10 }],
  tornOff: [
    { x: 40, y: 32, tabs: 3 },
    { x: 690, y: 92, tabs: 2 },
    { x: 420, y: 250, width: 520, height: 330, tabs: 1, moved: true },
  ],
}
