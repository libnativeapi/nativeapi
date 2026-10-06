import type { WindowFramePlatform } from '@dazzlabs/dazzui'

/**
 * The themes the prototype is drawn in, chosen on two axes: the style — a
 * DazzUI family, a desktop, or an Omarchy theme — and its light or dark
 * appearance. Every style comes in both. The values are the `data-theme`
 * blocks of DazzUI's token sheet (`@dazzlabs/dazzui/style.css`);
 * Studio Light is what `:root` holds, so it clears the attribute instead.
 */
export const APPEARANCES = [
  { value: 'light', title: 'Light' },
  { value: 'dark', title: 'Dark' },
] as const

export type Appearance = (typeof APPEARANCES)[number]['value']

export interface ThemeStyle {
  value: string
  title: string
}

const DAZZUI_STYLES: readonly ThemeStyle[] = [
  { value: 'studio', title: 'Studio' },
  { value: 'bright', title: 'Bright' },
  { value: 'frost', title: 'Frost' },
  { value: 'graphite', title: 'Graphite' },
  { value: 'ember', title: 'Ember' },
  { value: 'nocturne', title: 'Nocturne' },
]

const DESKTOP_STYLES: readonly ThemeStyle[] = [
  { value: 'macos27', title: 'macOS 27 Golden Gate' },
  { value: 'macos15', title: 'macOS 15 Sequoia' },
  { value: 'windows11', title: 'Windows 11' },
  { value: 'ubuntu', title: 'Ubuntu' },
  { value: 'debian', title: 'Debian (GNOME)' },
  { value: 'fedora', title: 'Fedora (GNOME)' },
  { value: 'kde', title: 'KDE Plasma' },
]

const OMARCHY_STYLES: readonly ThemeStyle[] = [
  ['tokyo-night', 'Tokyo Night'],
  ['catppuccin', 'Catppuccin'],
  ['catppuccin-latte', 'Catppuccin Latte'],
  ['ethereal', 'Ethereal'],
  ['everforest', 'Everforest'],
  ['flexoki-light', 'Flexoki Light'],
  ['gruvbox', 'Gruvbox'],
  ['hackerman', 'Hackerman'],
  ['kanagawa', 'Kanagawa'],
  ['last-horizon', 'Last Horizon'],
  ['lumon', 'Lumon'],
  ['lupine', 'Lupine'],
  ['matte-black', 'Matte Black'],
  ['miasma', 'Miasma'],
  ['nord', 'Nord'],
  ['osaka-jade', 'Osaka Jade'],
  ['retro-82', 'Retro 82'],
  ['ristretto', 'Ristretto'],
  ['rose-pine', 'Rosé Pine'],
  ['solitude', 'Solitude'],
  ['vantablack', 'Vantablack'],
  ['white', 'White'],
].map(([value, title]) => ({ value: `omarchy-${value}`, title: `Omarchy · ${title}` }))

/** The Style toolbar: DazzUI's families, then the desktops, then Omarchy's themes. */
export const STYLES: readonly ThemeStyle[] = [...DAZZUI_STYLES, ...DESKTOP_STYLES, ...OMARCHY_STYLES]

export const DEFAULT_STYLE = 'studio'
export const DEFAULT_APPEARANCE: Appearance = 'light'

/** The theme `:root` holds, which is selected by clearing `data-theme`. */
const ROOT_THEME = 'studio-light'

/**
 * The window chrome a style stands for: what to pass as DazzUI's
 * `WindowFrame` `platform`. DazzUI's own families have none of their own and
 * are drawn as on a Mac.
 */
export function windowPlatformOf(style: string): WindowFramePlatform {
  if (style.startsWith('omarchy-')) return 'omarchy'
  if (style === 'windows11') return 'windows'
  if (style === 'ubuntu' || style === 'kde') return style
  if (style === 'debian' || style === 'fedora') return 'gnome'
  if (style === 'macos15') return 'macos15'
  return 'macos'
}

/** Points `root` at a style's colours in an appearance, the way the token sheet expects. */
function applyPalette(root: HTMLElement, style: string, appearance: Appearance) {
  const theme = `${style}-${appearance}`
  if (theme === ROOT_THEME) root.removeAttribute('data-theme')
  else root.setAttribute('data-theme', theme)
  root.style.colorScheme = appearance
}

/** The Style and Appearance toolbar: a style's colours in an appearance. */
export function applyTheme(root: HTMLElement, styleValue: string, appearanceValue: string) {
  const style = STYLES.some(entry => entry.value === styleValue) ? styleValue : DEFAULT_STYLE
  const appearance: Appearance = appearanceValue === 'dark' ? 'dark' : 'light'
  applyPalette(root, style, appearance)
}
