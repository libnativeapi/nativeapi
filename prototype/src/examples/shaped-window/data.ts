import type { DemoShape, Shadow, ShadowColor, ShadowPreset, ShapeLook } from './types'

export const SHAPES: readonly DemoShape[] = [
  'circle',
  'star',
  'bubble',
  'heart',
  'flower',
  'hexagon',
  'squircle',
  'blob',
  'burst',
  'droplet',
  'diamond',
  'shield',
]

const ramp = (hue: string, shade: number) => `var(--base-color-${hue}-${shade})`

/**
 * The art of each silhouette: a fixed gradient per shape, shared by its
 * gallery thumbnail, the preview window and the controls' banner. They are
 * the picture being shaped, not theme colours, so they come from DazzUI's
 * base ramps and stay the same in every theme and appearance.
 */
export const LOOKS: Record<DemoShape, ShapeLook> = {
  circle: { name: 'Aurora', colors: [ramp('brand', 700), ramp('nocturne', 500), ramp('red', 300)] },
  star: { name: 'Sunset', colors: [ramp('red', 600), ramp('ember', 400), ramp('amber', 300)] },
  bubble: { name: 'Ocean', colors: [ramp('sky', 800), ramp('sky', 500), ramp('frost', 200)] },
  heart: { name: 'Berry', colors: [ramp('nocturne', 900), ramp('red', 500), ramp('red', 200)] },
  flower: { name: 'Lime', colors: [ramp('green', 800), ramp('green', 500), ramp('acid', 400)] },
  hexagon: { name: 'Glacier', colors: [ramp('brand', 900), ramp('brand', 400), ramp('sky', 200)] },
  squircle: { name: 'Honey', colors: [ramp('ember', 600), ramp('amber', 600), ramp('amber', 300)] },
  blob: { name: 'Lagoon', colors: [ramp('frost', 800), ramp('frost', 500), ramp('green', 300)] },
  burst: { name: 'Coral', colors: [ramp('red', 700), ramp('red', 400), ramp('amber', 200)] },
  droplet: { name: 'Rain', colors: [ramp('brand', 800), ramp('sky', 600), ramp('sky', 300)] },
  diamond: { name: 'Peach', colors: [ramp('red', 500), ramp('ember', 300), ramp('amber', 200)] },
  shield: { name: 'Forest', colors: [ramp('green', 900), ramp('green', 700), ramp('green', 300)] },
}

/** Where the controls' banner starts, before the shape's own first colour. */
export const NIGHT = ramp('ink', 900)

/** The two sizes the Size toggle moves the preview between, and how long the morph takes. */
export const SIZES = [320, 400] as const
export const MORPH_MS = 450
export const SHADOW_MS = 220

export const SHADOW_COLORS: Record<ShadowColor, string> = {
  Black: 'var(--base-color-black)',
  Purple: ramp('brand', 500),
  Blue: ramp('sky', 600),
  Rose: ramp('red', 500),
  Green: ramp('green', 600),
}

/** The presets: opacity, blur and a straight-down offset; None hides the shadow. */
export const SHADOW_PRESETS: Record<Exclude<ShadowPreset, 'None'>, Omit<Shadow, 'enabled' | 'x'>> = {
  Soft: { opacity: 0.3, blur: 18, y: 6, color: 'Black' },
  Float: { opacity: 0.32, blur: 32, y: 14, color: 'Black' },
  Sharp: { opacity: 0.4, blur: 3, y: 5, color: 'Black' },
  Glow: { opacity: 0.55, blur: 28, y: 0, color: 'Purple' },
}

export const DEFAULT_SHADOW: Shadow = { enabled: true, color: 'Black', opacity: 0.3, blur: 18, x: 0, y: 6 }

/** The two windows' places on a ~1350 × 730 desktop, as the example puts them from the work area. */
export const CONTROLS_AT = { x: 60, y: 30 }
export const PREVIEW_AT = { x: 640, y: 120 }
export const CONTROLS_SIZE = { width: 480, height: 668 }
