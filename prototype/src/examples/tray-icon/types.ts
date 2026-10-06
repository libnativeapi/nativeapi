import type { WindowFramePlatform } from '@dazzlabs/dazzui'

/** What `IconAnimator` can play on a tray icon (`icon_animations.dart`). */
export type IconAnimation = 'spinner' | 'pulse' | 'blink' | 'progress' | 'wave' | 'rotate' | 'clock' | 'widget'

/** The three ways the example hands a still image to `TrayIcon.setIcon`. */
export type StillIcon = 'asset' | 'drawn' | 'base64'

/** A canned use of a tray icon: animation, title and tooltip together. */
export type Scene = 'download' | 'recording' | 'syncing'

/** `ContextMenuTrigger`: which click opens the context menu by itself. */
export type Trigger = 'none' | 'clicked' | 'rightClicked' | 'doubleClicked'

/** `MenuBackend`; only Windows has a second one. */
export type MenuBackend = 'native' | 'winUi3'

/** The icon colours offered; `auto` is the bar's own ink (a template image on macOS). */
export type IconColor = 'auto' | 'blue' | 'amber' | 'red'

export type Os = 'macos' | 'windows' | 'linux'

/** What the platform under the prototype can and cannot do, as the example checks it. */
export interface Capabilities {
  os: Os
  platform: WindowFramePlatform
  /** `TrayIcon.setTitle` draws text beside the icon: not on Windows. */
  title: boolean
  /** `getBounds` locates the icon: not for a StatusNotifierItem. */
  bounds: boolean
  /** `openContextMenu` / `closeContextMenu`: only the shell opens it on Linux. */
  openMenu: boolean
  /** A second menu backend (WinUI 3) to choose. */
  backend: boolean
}

/** One `TrayIcon` the example created, with what the window shows about it. */
export interface TrayEntry {
  /** `#1`, `#2`… in the window. */
  number: number
  /** `TrayIcon.getId()`. */
  id: number
  animation: IconAnimation | null
  /** The still image shown when nothing plays. */
  still: StillIcon | null
  scene: Scene | null
  title: string | null
  tooltip: string | null
  visible: boolean
  trigger: Trigger
  fps: number
  scale: number
  color: IconColor
  paused: boolean
  /** `performance.now()` when the animation started, less the time spent paused. */
  startedAt: number
  /** The animation time a paused icon is frozen at, in seconds. */
  pausedAt: number
  clicks: number
  rightClicks: number
  doubleClicks: number
}

export type CheckStatus = 'open' | 'pass' | 'fail'

export interface CheckItem {
  id: string
  label: string
  /** Manual items need eyes on the tray; the rest tick themselves. */
  manual: boolean
  note?: string
  status: CheckStatus
  detail: string
}

export type Tab = 'animate' | 'properties' | 'checklist'

/** Where the window stands on the desktop. */
export type WindowPlacement = 'center' | 'icon'
