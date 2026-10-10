/** `ResizeEdge` (`window.h`). */
export type ResizeEdge = 'top' | 'left' | 'right' | 'bottom' | 'topLeft' | 'topRight' | 'bottomLeft' | 'bottomRight'

/** The window the example moves and resizes by its own drag areas. */
export interface DragAreasWindow {
  x: number
  y: number
  width: number
  height: number
  maximized: boolean
}

export interface Size {
  width: number
  height: number
}
