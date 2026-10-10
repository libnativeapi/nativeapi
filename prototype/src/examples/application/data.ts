import type { WindowFramePlatform } from '@dazzlabs/dazzui'

import type { AppEventType, AppIconPreset, Brightness, DockSurface } from './types'

/** Every `ApplicationEvent`, in the order a run emits them. */
export const APP_EVENTS: readonly { type: AppEventType; name: string; label: string; when: string }[] = [
  { type: 'started', name: 'ApplicationStartedEvent', label: 'Started', when: 'run() entered the loop' },
  { type: 'activated', name: 'ApplicationActivatedEvent', label: 'Activated', when: 'Came to the front' },
  { type: 'deactivated', name: 'ApplicationDeactivatedEvent', label: 'Deactivated', when: 'Another app took the focus' },
  { type: 'quitRequested', name: 'ApplicationQuitRequestedEvent', label: 'Quit', when: 'quit(), Cmd+Q, logout' },
  { type: 'exiting', name: 'ApplicationExitingEvent', label: 'Exiting', when: 'The loop is about to stop' },
]

export const eventName = (type: AppEventType) => APP_EVENTS.find(e => e.type === type)!.name

/** What `setIcon` is handed for each preset. */
export const ICON_PRESETS: readonly { value: AppIconPreset; label: string; path: string }[] = [
  { value: 'default', label: 'Default', path: 'assets/icon.png' },
  { value: 'night', label: 'Night', path: 'assets/icon_night.png' },
  { value: 'amber', label: 'Amber', path: 'assets/icon_amber.png' },
  { value: 'beta', label: 'Beta', path: 'assets/icon_beta.png' },
  { value: 'missing', label: 'Missing', path: 'assets/missing.png' },
]

export const BADGE_PRESETS: readonly { label: string; value: string }[] = [
  { label: 'None', value: '' },
  { label: '3', value: '3' },
  { label: '99+', value: '99+' },
  { label: 'New', value: 'New' },
]

export const BRIGHTNESSES: readonly { value: Brightness; label: string }[] = [
  { value: 'system', label: 'System' },
  { value: 'light', label: 'Light' },
  { value: 'dark', label: 'Dark' },
]

export const BRIGHTNESS_NAMES: Record<Brightness, string> = {
  system: 'Brightness::System',
  light: 'Brightness::Light',
  dark: 'Brightness::Dark',
}

/** Where each desktop draws an app's icon with its badge and progress. */
export const DOCK_SURFACE: Record<WindowFramePlatform, DockSurface> = {
  macos: 'dock',
  macos15: 'dock',
  windows: 'taskbar',
  kde: 'taskbar',
  // Ubuntu Dock and GNOME's Dash to Dock read LauncherEntry.
  ubuntu: 'side-dock',
  gnome: 'side-dock',
  // Hyprland and Waybar have no launcher to draw it on.
  omarchy: 'none',
}

/** The window the app opens first, as the example creates it. */
export const MAIN_WINDOW_TITLE = 'Application Example'
