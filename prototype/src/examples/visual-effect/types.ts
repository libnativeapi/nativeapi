/** `VisualEffect` (`window.h`), by the names the bindings give it. */
export type VisualEffect = 'none' | 'blur' | 'acrylic' | 'mica' | 'micaAlt' | 'hud' | 'popover' | 'menu'

/** Where a window stands on the free desktop. */
export interface Point {
  x: number
  y: number
}

/** The plain red window behind the example, when the Backdrop switch is on. */
export interface Backdrop {
  /** The id `Window.create()` handed back. */
  id: number
  at: Point
}
