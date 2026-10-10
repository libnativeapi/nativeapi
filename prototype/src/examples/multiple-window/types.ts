export interface Rect {
  x: number
  y: number
  width: number
  height: number
}

export type WindowKey = 'primary' | 'secondary' | 'tertiary'

/** One of the example's three windows, in screen coordinates. */
export interface HostWindow {
  key: WindowKey
  /** The native window id; the windows are created tertiary first. */
  id: number
  /** The frame the window has, whether or not it is on screen. */
  frame: Rect
  visible: boolean
  /** What `Window.bounds` returned at the last read, or null before the first. */
  readBack: Rect | null
  z: number
}

/**
 * What the will-hide hook does: none installed; log only, as the Flutter
 * example's hook does, which swallows the hide where the hook replaces it;
 * or log and call `callOriginalHide`, as the Deno and GPUI examples do.
 */
export type HideHook = 'none' | 'swallow' | 'pass'
