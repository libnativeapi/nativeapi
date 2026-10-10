/** `TitleBarStyle` (`window.h`). */
export type TitleBarStyle = 'normal' | 'hidden'

/** `VisualEffect` (`window.h`). */
export type VisualEffect = 'none' | 'blur' | 'acrylic' | 'mica' | 'micaAlt' | 'hud' | 'popover' | 'menu'

/** `ResizeEdge` (`window.h`). */
export type ResizeEdge = 'top' | 'left' | 'right' | 'bottom' | 'topLeft' | 'topRight' | 'bottomLeft' | 'bottomRight'

/**
 * The background presets the example sends with `setBackgroundColor`;
 * `default` is the platform's own window colour, what a new window has.
 */
export type BackgroundName = 'default' | 'white' | 'lightGrey' | 'dark' | 'blue' | 'transparent'

export interface Size {
  width: number
  height: number
}

/** A rectangle on the desktop the stage draws, in its pixels from the work area's top-left. */
export interface Rect {
  x: number
  y: number
  width: number
  height: number
}

/** The boolean setters of the Behaviour tab, by their getter's name. */
export type BehaviourFlag =
  | 'alwaysOnTop'
  | 'alwaysOnBottom'
  | 'resizable'
  | 'movable'
  | 'minimizable'
  | 'maximizable'
  | 'fullScreenable'
  | 'closable'
  | 'controlButtonsVisible'
  | 'visibleOnAllWorkspaces'
  | 'visibleInTaskbar'
  | 'ignoreMouseEvents'
  | 'focusable'

/**
 * One native window as the prototype keeps it: what its getters would
 * return. `frame` is the restored frame; maximized and full screen are
 * drawn from the desktop's size instead, and give it back when they end.
 */
export interface SimWindow extends Record<BehaviourFlag, boolean> {
  id: number
  /** The example's own window, or one it created with `Window.create()`. */
  kind: 'example' | 'plain'
  title: string
  frame: Rect
  visible: boolean
  minimized: boolean
  maximized: boolean
  fullScreen: boolean
  minimumSize: Size | null
  maximumSize: Size | null
  titleBarStyle: TitleBarStyle
  contentUnderTitleBar: boolean
  /** `setTitleBarColors` took (Windows, WinUI 3). */
  titleBarColors: boolean
  hasShadow: boolean
  opacity: number
  visualEffect: VisualEffect
  background: BackgroundName
  /** Stacking order among the normal-level windows; higher is in front. */
  z: number
}

export type Tab = 'state' | 'geometry' | 'appearance' | 'behaviour'

/** What the pane shows: the selected window's tabs, or the map of the display. */
export type View = 'window' | 'map'

/** The desktop's measured size: the work area, and the whole display around it. */
export interface Area {
  /** The work area: the desktop under (or over) the bar. */
  width: number
  height: number
  /** The bar's thickness, and whether it runs along the top. */
  bar: number
  barTop: boolean
  /** The work area's top-left in client coordinates, for drags. */
  left: number
  top: number
}
