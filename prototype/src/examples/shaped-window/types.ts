/** The silhouettes the example offers, in the gallery's order (`DemoShape`). */
export type DemoShape =
  | 'circle'
  | 'star'
  | 'bubble'
  | 'heart'
  | 'flower'
  | 'hexagon'
  | 'squircle'
  | 'blob'
  | 'burst'
  | 'droplet'
  | 'diamond'
  | 'shield'

/** A point in content-local logical pixels, as `WindowShape.addPoint` takes it. */
export interface Point {
  x: number
  y: number
}

/** A shape's art: its name and the three stops of its gradient. */
export interface ShapeLook {
  name: string
  colors: readonly [string, string, string]
}

export type ShadowPreset = 'None' | 'Soft' | 'Float' | 'Sharp' | 'Glow'

/** The colours the Adjust view offers for the shadow. */
export type ShadowColor = 'Black' | 'Purple' | 'Blue' | 'Rose' | 'Green'

/** What `WindowShadow` carries, plus whether `hasShadow` shows it. */
export interface Shadow {
  enabled: boolean
  color: ShadowColor
  /** 0…1, folded into the colour's alpha. */
  opacity: number
  /** 0…64 logical pixels. */
  blur: number
  /** −64…64 logical pixels; positive is right and down. */
  x: number
  y: number
}
