import type { Os } from '../../components/platform'
import type { VisualEffect } from './types'

/** Every `VisualEffect`, in the enum's order, with what each platform draws for it. */
export const EFFECTS: readonly {
  value: VisualEffect
  /** What the value is for, as `window.h` says it. */
  what: string
  macos: string
  windows: string
}[] = [
  { value: 'none', what: 'No material: the window shows its background colour.', macos: 'Background colour', windows: 'Background colour' },
  {
    value: 'blur',
    what: 'A plain blur of what is behind the window, the most see-through material.',
    macos: 'NSVisualEffectMaterialSidebar',
    windows: 'Acrylic system backdrop',
  },
  {
    value: 'acrylic',
    what: 'A heavier, tinted blur.',
    macos: 'NSVisualEffectMaterialUnderWindowBackground',
    windows: 'Acrylic system backdrop',
  },
  {
    value: 'mica',
    what: 'Nearly opaque, tinted by the desktop wallpaper rather than the windows behind.',
    macos: 'NSVisualEffectMaterialWindowBackground',
    windows: 'Mica system backdrop',
  },
  {
    value: 'micaAlt',
    what: 'Mica with a stronger tint, for windows with tabs in the title bar.',
    macos: 'NSVisualEffectMaterialTitlebar',
    windows: 'Mica Alt system backdrop',
  },
  { value: 'hud', what: 'The dark material of heads-up panels.', macos: 'NSVisualEffectMaterialHUDWindow', windows: 'Acrylic (no HUD material)' },
  { value: 'popover', what: 'The material of popovers.', macos: 'NSVisualEffectMaterialPopover', windows: 'Acrylic (no popover material)' },
  {
    value: 'menu',
    what: 'The material of menus, which suits a window shown from a tray icon.',
    macos: 'NSVisualEffectMaterialMenu',
    windows: 'Acrylic (no menu material)',
  },
]

export const effectOf = (value: VisualEffect) => EFFECTS.find(e => e.value === value)!

/** The window's content size, as the example sets it (`contentSize`). */
export const WINDOW_SIZE = { width: 600, height: 580 }

/** How far the backdrop reaches past the window on each side. */
export const BACKDROP_INSET = { x: 72, y: 44 }

/** Where the example centres itself on a ~1350 × 730 desktop. */
export const WINDOW_AT = { x: 375, y: 64 }

/**
 * Why a platform refuses, for the greyed-out chips' tooltip; null where every
 * effect is there.
 */
export function unsupportedReason(os: Os): string | null {
  return os === 'linux' ? 'Linux has no visual effects: blur behind a window is up to the compositor, and GNOME has none.' : null
}
