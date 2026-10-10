import type { ResizeEdge, Size } from './types'

export const WINDOW_TITLE = 'nativeapi · Drag areas'

/** What the example asks for at start, with its title bar hidden. */
export const CONTENT_SIZE: Size = { width: 720, height: 480 }
export const MINIMUM_SIZE: Size = { width: 480, height: 320 }

/**
 * The handles' thickness and their distance from the window's edge: inset so
 * that they are clearly the page's, not the native frame's. The GUI tests
 * press in the middle of these bands.
 */
export const EDGE_SIZE = 12
export const EDGE_INSET = 16
/** The move bar's height. */
export const BAR_HEIGHT = 44

/** Every handle, with the cursor the platform shows over it. */
export const EDGES: readonly { edge: ResizeEdge; cursor: string }[] = [
  { edge: 'topLeft', cursor: 'nwse-resize' },
  { edge: 'top', cursor: 'ns-resize' },
  { edge: 'topRight', cursor: 'nesw-resize' },
  { edge: 'left', cursor: 'ew-resize' },
  { edge: 'right', cursor: 'ew-resize' },
  { edge: 'bottomLeft', cursor: 'nesw-resize' },
  { edge: 'bottom', cursor: 'ns-resize' },
  { edge: 'bottomRight', cursor: 'nwse-resize' },
]

/** `enableResizeEdges` with the button down: right and bottom only. */
export const LIMITED_EDGES: readonly ResizeEdge[] = ['right', 'bottom', 'bottomRight']
