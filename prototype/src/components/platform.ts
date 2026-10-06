import type { WindowFramePlatform } from '@dazzlabs/dazzui'

export type Os = 'macos' | 'windows' | 'linux'

/** The operating system a desktop's chrome stands for. */
export function osOf(platform: WindowFramePlatform): Os {
  if (platform === 'macos' || platform === 'macos15') return 'macos'
  if (platform === 'windows') return 'windows'
  return 'linux'
}

/** How a log line names the platform: `macOS`, `Windows`, `GNOME`… */
export const PLATFORM_NAMES: Record<WindowFramePlatform, string> = {
  macos: 'macOS',
  macos15: 'macOS',
  windows: 'Windows',
  gnome: 'GNOME',
  ubuntu: 'Ubuntu',
  kde: 'KDE Plasma',
  omarchy: 'Hyprland',
}

/** Where each desktop keeps its bar: along the top, or a taskbar along the bottom. */
export const BAR_EDGE: Record<WindowFramePlatform, 'top' | 'bottom'> = {
  macos: 'top',
  macos15: 'top',
  windows: 'bottom',
  gnome: 'top',
  ubuntu: 'top',
  kde: 'bottom',
  omarchy: 'top',
}
