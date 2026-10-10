import type { WindowFramePlatform } from '@dazzlabs/dazzui'

import type {
  Capabilities,
  CheckItem,
  IconAnimation,
  IconColor,
  MenuBackend,
  Scene,
  StillIcon,
  Trigger,
} from './types'

export const DEFAULT_TOOLTIP = 'nativeapi tray icon'

export const ANIMATIONS: readonly { value: IconAnimation; label: string }[] = [
  { value: 'spinner', label: 'Spinner' },
  { value: 'pulse', label: 'Pulse' },
  { value: 'blink', label: 'Blink' },
  { value: 'progress', label: 'Progress' },
  { value: 'wave', label: 'Wave' },
  { value: 'rotate', label: 'Rotate' },
  { value: 'clock', label: 'Clock' },
  { value: 'widget', label: 'Any widget' },
]

export const animationLabel = (animation: IconAnimation) =>
  ANIMATIONS.find(entry => entry.value === animation)?.label ?? animation

export const STILL_ICONS: readonly { value: StillIcon; label: string }[] = [
  { value: 'asset', label: 'Asset' },
  { value: 'drawn', label: 'Drawn' },
  { value: 'base64', label: 'Base64' },
]

export const RATES = [10, 24, 30, 60] as const
export const SCALES = [1, 2, 3] as const

/** Points per side of a tray icon; the pixel size is this times the resolution. */
export const ICON_POINTS = 16

/** The colour tokens behind each choice; `auto` follows the bar. */
export const COLORS: readonly { value: IconColor; label: string; token: string | null }[] = [
  { value: 'auto', label: 'Auto', token: null },
  { value: 'blue', label: 'Blue', token: 'var(--color-info-500)' },
  { value: 'amber', label: 'Amber', token: 'var(--color-warning-500)' },
  { value: 'red', label: 'Red', token: 'var(--color-danger-500)' },
]

export const SCENES: readonly { value: Scene; label: string; animation: IconAnimation; tooltip: string }[] = [
  { value: 'download', label: 'Download', animation: 'progress', tooltip: 'Downloading nativeapi.zip' },
  { value: 'recording', label: 'Recording', animation: 'blink', tooltip: 'Recording — click to stop' },
  { value: 'syncing', label: 'Syncing', animation: 'spinner', tooltip: 'Syncing 3 folders' },
]

export const TITLE_PRESETS: readonly { label: string; value: string | null }[] = [
  { label: 'None', value: null },
  { label: '42%', value: '42%' },
  { label: '00:12', value: '00:12' },
  { label: 'Chinese greeting', value: '你好' },
]

export const TOOLTIP_PRESETS: readonly { label: string; value: string | null }[] = [
  { label: 'None', value: null },
  { label: 'Short', value: DEFAULT_TOOLTIP },
  {
    label: 'Long',
    value:
      'A long tooltip that says rather more than a tooltip usually should, to see where the platform cuts it off',
  },
  { label: '2 lines', value: 'Line one\nLine two' },
]

export const TRIGGERS: readonly { value: Trigger; label: string }[] = [
  { value: 'none', label: 'Manual' },
  { value: 'clicked', label: 'Left' },
  { value: 'rightClicked', label: 'Right' },
  { value: 'doubleClicked', label: 'Double' },
]

export const BACKENDS: readonly { value: MenuBackend; label: string }[] = [
  { value: 'native', label: 'Native' },
  { value: 'winUi3', label: 'WinUI 3' },
]

/** What the example's `TrayController` checks before offering a control. */
export function capabilitiesOf(platform: WindowFramePlatform): Capabilities {
  const os = platform === 'windows' ? 'windows' : platform === 'macos' || platform === 'macos15' ? 'macos' : 'linux'
  return {
    os,
    platform,
    title: os !== 'windows',
    bounds: os !== 'linux',
    openMenu: os !== 'linux',
    backend: os === 'windows',
    contentView: os === 'macos',
  }
}

/** The acceptance checklist (`checklist.dart`), less what the platform cannot offer. */
export function checklistOf({ os }: Capabilities): CheckItem[] {
  const item = (id: string, label: string, extra: Partial<CheckItem> = {}): CheckItem => ({
    id,
    label,
    manual: false,
    status: 'open',
    detail: '',
    ...extra,
  })
  const manual = (id: string, label: string, note?: string) => item(id, label, { manual: true, note })
  return [
    item('supported', 'TrayManager.isSupported'),
    item('create', 'TrayIcon.create returns an icon'),
    item('managed', 'TrayManager get and getAll know it'),
    item('clicked', 'Clicked event'),
    item('rightClicked', 'Right clicked event'),
    item('doubleClicked', 'Double clicked event'),
    item('menuOpenClose', 'Menu opened and closed'),
    item('menuItems', 'Menu item, checkbox, submenu'),
    item(
      'triggers',
      os === 'linux' ? 'Left, right and double click open the menu' : 'All four triggers open the menu',
    ),
    ...(os === 'linux'
      ? []
      : [item('openMenu', 'openContextMenu returns true'), item('closeMenu', 'closeContextMenu closes it')]),
    item('visible', 'setVisible round trip'),
    item('readBack', os === 'windows' ? 'Tooltip reads back' : 'Title and tooltip read back'),
    ...(os === 'linux' ? [] : [item('bounds', 'getBounds is not empty')]),
    item('frames', '100 frames at the target rate'),
    manual('m.still', 'Icon swaps: asset, drawn, base64'),
    manual('m.animation', 'Tray animation matches preview'),
    manual('m.three', 'Three icons animate at once'),
    manual('m.popup', 'Popup: a click shows the window, losing focus hides it'),
    ...(os === 'windows' ? [] : [manual('m.title', 'Title next to the icon', os === 'macos' ? 'macOS' : undefined)]),
    manual('m.tooltip', 'Tooltip on hover'),
    manual('m.hidden', 'Hidden icon leaves the tray'),
    manual('m.removed', 'Removed icon leaves the tray'),
  ]
}
