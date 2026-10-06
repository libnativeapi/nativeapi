export interface Point {
  x: number
  y: number
}

export interface Rect extends Point {
  width: number
  height: number
}

export type PanelId = 'inspector' | 'stopwatch'
export type MainId = 'A' | 'B'
export type SlotId = 'A-left' | 'A-bottom' | 'B-top' | 'B-right'

/** One main window: a workspace with a panel slot on two of its sides. */
export interface MainWindow {
  id: MainId
  /** The native window id (`Window.id`). */
  nativeId: number
  x: number
  y: number
  z: number
}

/** A panel in a window of its own. Its frame is where its content stands; the title bar is above it. */
export interface FloatingWindow {
  nativeId: number
  /** The content's top-left on the desktop. */
  x: number
  y: number
  /** The content size: the size of the slot the panel was torn from. */
  width: number
  height: number
  z: number
  /** `Window.opacity`: 0.6 while the window is over a slot it would dock into. */
  opacity: number
}

export type PanelPlace = { kind: 'docked'; slot: SlotId } | { kind: 'floating'; window: FloatingWindow }

/**
 * What a panel would lose if it were rebuilt, kept on the panel so it
 * survives every move: `instance` and `createdAt` never change, `moves`
 * counts the windows it has been moved between.
 */
export interface PanelCommon {
  instance: number
  createdAt: string
  moves: number
  /** The window the panel was last shown in, to count the moves. */
  shownIn: string
}

export interface InspectorState extends PanelCommon {
  name: string
  clicks: number
  visible: boolean
  opacity: number
  selected: number
  scroll: number
}

export interface StopwatchState extends PanelCommon {
  /** Milliseconds run before the current start. */
  elapsed: number
  /** `performance.now()` of the current start, or null while paused. */
  startedAt: number | null
  laps: number[]
}

/** The gesture: pressed on a docked header but not 8px yet, or a panel's window following the cursor. */
export type DragMode = 'idle' | 'pending' | 'floating'

/** Where the panels start, for the stories. */
export interface Layout {
  inspector: SlotId | Rect
  stopwatch: SlotId | Rect
}
