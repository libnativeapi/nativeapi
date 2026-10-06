/** `DisplayOrientation` (`display.h`). */
export type Orientation = 'portrait' | 'landscape' | 'portraitFlipped' | 'landscapeFlipped'

export interface Point {
  x: number
  y: number
}

export interface Rect {
  x: number
  y: number
  width: number
  height: number
}

/** What the system's bars take off each edge of a display, in logical pixels. */
export interface Margins {
  top: number
  right: number
  bottom: number
  left: number
}

/** Where a display stands next to the primary one in the system's arrangement. */
export type Side = 'left' | 'right'

/**
 * One connected display as the platform describes it. Its position is not
 * stored: the arrangement lays the displays out from the primary one, so a
 * rotation or a new scale keeps them edge to edge, as the system does.
 */
export interface DisplayInfo {
  /** `getId()`: stable while connected, fresh after a reconnect. */
  id: number
  name: string
  primary: boolean
  /** The panel's own pixels in landscape; the logical size divides them by the scale. */
  pixelWidth: number
  pixelHeight: number
  scale: number
  refreshRate: number
  bitDepth: number
  orientation: Orientation
  side: Side
  /** The top edge's offset from the primary display's, in logical pixels. */
  offsetY: number
  margins: Margins
}

/** A display with its geometry worked out: what the getters return. */
export interface PlacedDisplay extends DisplayInfo {
  /** `getPosition()`. */
  x: number
  y: number
  /** `getSize()`. */
  width: number
  height: number
  /** `getWorkArea()`. */
  workArea: Rect
}

export type Tab = 'arrangement' | 'details' | 'table'

/** The display setups the stories start from. */
export type Scene = 'two' | 'single' | 'three' | 'rotated'
