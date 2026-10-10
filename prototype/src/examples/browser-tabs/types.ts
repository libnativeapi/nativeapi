/** A point or an offset on the desktop, in pixels from its top-left. */
export interface Point {
  x: number
  y: number
}

export interface Rect extends Point {
  width: number
  height: number
}

/** One tab. Its page state lives here, not in the page, so it survives every move. */
export interface BrowserTab {
  id: number
  title: string
  /** Which of the theme's ramps marks the tab, so the dot follows the theme. */
  hue: number
  page: PageState
}

/**
 * What a page would lose if it were rebuilt: the edited address, the likes,
 * the scroll position and the time it has been open. `instance` is the page
 * object's number; it never changes for a tab.
 */
export interface PageState {
  instance: number
  openedAt: number
  address: string
  likes: number
  scroll: number
  /** How many times the tab has moved to another window. */
  moves: number
}

/** A browser window: a frame on the desktop, a tab strip and its tabs. */
export interface BrowserWindow {
  /** The native window id (`Window.id`). */
  id: number
  x: number
  y: number
  width: number
  height: number
  z: number
  tabs: BrowserTab[]
  activeTabId: number
}

/**
 * The gesture's state, as the example's controller keeps it on top of
 * `WindowDragSession`: pressed but not moved far enough, a tab sliding along a
 * strip, a torn-off window following the cursor, or a window moved by its strip.
 */
export type DragMode = 'idle' | 'pending' | 'inStrip' | 'window' | 'moveWindow'

/** The windows the example opens with. */
export type Scene = 'twoWindows' | 'manyTabs' | 'tornOff'
