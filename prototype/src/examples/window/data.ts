import type { WindowFramePlatform } from '@dazzlabs/dazzui'

import type { Os } from '../../components/platform'
import type { BackgroundName, BehaviourFlag, SimWindow, Size, VisualEffect } from './types'

/** The presets of the Geometry tab, as the real example sends them. */
export const SIZE_PRESETS: readonly Size[] = [
  { width: 800, height: 600 },
  { width: 1024, height: 768 },
]
export const CONTENT_SIZE: Size = { width: 760, height: 540 }
export const POSITION_PRESETS: readonly { x: number; y: number }[] = [
  { x: 100, y: 100 },
  { x: 400, y: 300 },
]
export const MINIMUM_SIZE: Size = { width: 400, height: 300 }
export const MAXIMUM_SIZE: Size = { width: 1200, height: 900 }

export const TITLE_PRESETS = ['Hello Window', 'My App', 'nativeapi'] as const

/**
 * The values `setBackgroundColor` is sent: colours of the window, not of this
 * UI, so each names the `0xRRGGBBAA` it stands for and the token that draws it.
 */
export interface Background {
  value: BackgroundName
  label: string
  rgba: string
  token: string
  /** The text a window of this colour wants; `theme` follows light and dark. */
  tone: 'light' | 'dark' | 'theme'
}

export const BACKGROUNDS: readonly Background[] = [
  { value: 'white', label: 'White', rgba: '#FFFFFFFF', token: 'var(--base-color-white)', tone: 'light' },
  { value: 'lightGrey', label: 'Light grey', rgba: '#EEEEEEFF', token: 'var(--base-color-graphite-100)', tone: 'light' },
  { value: 'dark', label: 'Dark', rgba: '#212121FF', token: 'var(--base-color-graphite-800)', tone: 'dark' },
  { value: 'blue', label: 'Blue', rgba: '#90CAF9FF', token: 'var(--base-color-sky-200)', tone: 'light' },
  { value: 'transparent', label: 'Transparent', rgba: '#00000000', token: 'transparent', tone: 'theme' },
]

/** What a window has before `setBackgroundColor`: the system's window colour, light or dark. */
const DEFAULT_BACKGROUND: Background = {
  value: 'default',
  label: 'System window colour',
  rgba: 'windowBackgroundColor',
  token: 'var(--color-surface)',
  tone: 'theme',
}

export const backgroundOf = (name: BackgroundName) => BACKGROUNDS.find(b => b.value === name) ?? DEFAULT_BACKGROUND

/** Every `VisualEffect`, with what stands in for it where the exact material is missing. */
export const VISUAL_EFFECTS: readonly { value: VisualEffect; label: string; macos: string; windows: string }[] = [
  { value: 'none', label: 'None', macos: 'Background colour', windows: 'Background colour' },
  { value: 'blur', label: 'Blur', macos: 'Sidebar material', windows: 'Acrylic backdrop (blur on Windows 10)' },
  { value: 'acrylic', label: 'Acrylic', macos: 'Under-window background', windows: 'Acrylic, Windows 10 1803+' },
  { value: 'mica', label: 'Mica', macos: 'Window background', windows: 'Mica, Windows 11 22H2+' },
  { value: 'micaAlt', label: 'Mica Alt', macos: 'Titlebar material', windows: 'Mica Alt, Windows 11 22H2+' },
  { value: 'hud', label: 'HUD', macos: 'HUD window material', windows: 'Same as Acrylic' },
  { value: 'popover', label: 'Popover', macos: 'Popover material', windows: 'Same as Acrylic' },
  { value: 'menu', label: 'Menu', macos: 'Menu material', windows: 'Same as Acrylic' },
]

export const effectName = (effect: VisualEffect) => VISUAL_EFFECTS.find(e => e.value === effect)!.label

/**
 * How tall the native title bar is on each desktop: what `contentBounds`
 * gives up to it. Hyprland draws none.
 */
export const TITLE_BAR_HEIGHT: Record<WindowFramePlatform, number> = {
  macos: 28,
  macos15: 28,
  windows: 32,
  gnome: 46,
  ubuntu: 46,
  kde: 30,
  omarchy: 0,
}

/**
 * Each Behaviour switch: its label, the setter it calls and the getter it
 * reads, and the desktops where the call does nothing (the getter then keeps
 * its default, and the switch says why).
 */
export const BEHAVIOUR: readonly {
  group: 'Stacking' | 'Capabilities' | 'Platform specific'
  flag: BehaviourFlag
  label: string
  setter: string
  getter: string
  note?: string
  unsupported?: Partial<Record<Os, string>>
}[] = [
  { group: 'Stacking', flag: 'alwaysOnTop', label: 'Always on top', setter: 'setAlwaysOnTop', getter: 'isAlwaysOnTop', note: 'Above every normal window, focused or not' },
  { group: 'Stacking', flag: 'alwaysOnBottom', label: 'Always on bottom', setter: 'setAlwaysOnBottom', getter: 'isAlwaysOnBottom', note: 'Behind every other window; clears always on top' },
  { group: 'Capabilities', flag: 'resizable', label: 'Resizable', setter: 'setResizable', getter: 'isResizable', note: 'The frame and the corner grip resize it' },
  { group: 'Capabilities', flag: 'movable', label: 'Movable', setter: 'setMovable', getter: 'isMovable', note: 'The title bar moves it' },
  { group: 'Capabilities', flag: 'minimizable', label: 'Minimizable', setter: 'setMinimizable', getter: 'isMinimizable' },
  { group: 'Capabilities', flag: 'maximizable', label: 'Maximizable', setter: 'setMaximizable', getter: 'isMaximizable' },
  { group: 'Capabilities', flag: 'fullScreenable', label: 'Full-screenable', setter: 'setFullScreenable', getter: 'isFullScreenable' },
  { group: 'Capabilities', flag: 'closable', label: 'Closable', setter: 'setClosable', getter: 'isClosable' },
  {
    group: 'Platform specific',
    flag: 'controlButtonsVisible',
    label: 'Control buttons visible',
    setter: 'setWindowControlButtonsVisible',
    getter: 'isWindowControlButtonsVisible',
    unsupported: { windows: 'macOS only: always true here', linux: 'macOS only: always true here' },
  },
  {
    group: 'Platform specific',
    flag: 'visibleOnAllWorkspaces',
    label: 'Visible on all workspaces',
    setter: 'setVisibleOnAllWorkspaces',
    getter: 'isVisibleOnAllWorkspaces',
    note: 'Follows the user to every desktop',
  },
  {
    group: 'Platform specific',
    flag: 'visibleInTaskbar',
    label: 'Visible in taskbar',
    setter: 'setVisibleInTaskbar',
    getter: 'isVisibleInTaskbar',
    note: 'macOS: the Window menu only, the Dock lists apps',
  },
  {
    group: 'Platform specific',
    flag: 'ignoreMouseEvents',
    label: 'Ignore mouse events',
    setter: 'setIgnoreMouseEvents',
    getter: 'isIgnoreMouseEvents',
    note: 'Clicks fall through to what is behind',
  },
  {
    group: 'Platform specific',
    flag: 'focusable',
    label: 'Focusable',
    setter: 'setFocusable',
    getter: 'isFocusable',
    unsupported: { windows: 'macOS only: not implemented here', linux: 'macOS only: not implemented here' },
  },
]

const DEFAULTS = {
  visible: true,
  minimized: false,
  maximized: false,
  fullScreen: false,
  minimumSize: null,
  maximumSize: null,
  titleBarStyle: 'normal',
  contentUnderTitleBar: false,
  titleBarColors: false,
  hasShadow: true,
  opacity: 1,
  visualEffect: 'none',
  background: 'default',
  alwaysOnTop: false,
  alwaysOnBottom: false,
  resizable: true,
  movable: true,
  minimizable: true,
  maximizable: true,
  fullScreenable: true,
  closable: true,
  controlButtonsVisible: true,
  visibleOnAllWorkspaces: false,
  visibleInTaskbar: true,
  ignoreMouseEvents: false,
  focusable: true,
} as const satisfies Partial<SimWindow>

/** A window the example created with `Window.create()`, where it was put. */
export function plainWindow(id: number, x: number, y: number, width = 340, height = 240): SimWindow {
  return { ...DEFAULTS, id, kind: 'plain', title: `Window #${id}`, frame: { x, y, width, height }, z: id - 1 }
}

/**
 * The windows as the example starts: its own, and two it created so that
 * selection matters. The example draws its own chrome — a sidebar to the
 * top edge, the traffic lights over it on macOS, the caption buttons in its
 * band elsewhere — which is content under the title bar on macOS and a
 * hidden title bar everywhere else.
 */
export function initialWindows(os: Os): SimWindow[] {
  return [
    {
      ...DEFAULTS,
      id: 1,
      kind: 'example',
      title: 'Window Example',
      frame: { x: 24, y: 24, width: 840, height: 560 },
      titleBarStyle: os === 'macos' ? 'normal' : 'hidden',
      contentUnderTitleBar: os === 'macos',
      minimumSize: { width: 640, height: 420 },
      z: 3,
    },
    plainWindow(2, 900, 48),
    plainWindow(3, 950, 330, 320, 220),
  ]
}
