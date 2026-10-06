import type { PointerEvent as ReactPointerEvent } from 'react'

import type { WindowFramePlatform } from '@dazzlabs/dazzui'

import { osOf } from '../../components/platform'
import { CREATION_ORDER, RESIZE_TO, WINDOWS } from './data'
import { defaultFrame, rectText, simulateHookPlacement, simulateHyprlandTiles } from './simulate-layout'
import type { HideHook, HostWindow, Rect, WindowKey } from './types'

export interface HooksOptions {
  /** `setWillShowHook` installed at start, as the example does before `runWidget`. */
  showHook?: boolean
  hideHook?: HideHook
  /** Press the primary window's "Resize to 1000 × 1000" once the windows are up. */
  resizeAtStart?: boolean
}

const LOG_LIMIT = 200

/**
 * The example's windows and hooks, simulated. `WindowManager.setWillShowHook`
 * replaces the platform's show: the hook lays the window out against the
 * primary display's work area and lets the show through with
 * `callOriginalShow`. The will-hide hook replaces the hide the same way, so
 * a hook that only logs — the Flutter example's — keeps a window on screen.
 *
 * Per platform: macOS swizzles `makeKeyAndOrderFront:` and Windows hooks
 * `ShowWindow`, so there the hooks replace the operation. GTK's emission
 * hooks only observe it: on Linux the show and the hide happen whatever the
 * hook does. On Wayland a client cannot place its windows at all: Hyprland
 * tiles them, and `bounds` reads x and y as 0.
 */
export class HooksSimulation {
  windows: Record<WindowKey, HostWindow>
  workArea: Rect | null = null
  showHook: boolean
  hideHook: HideHook
  /** What the hooks saw, newest first. */
  hookLog: string[] = []
  /** Calls and events, newest first. */
  log: string[] = []
  lastEvent = 'Starting'
  /** A window is being moved by its title bar: no easing. */
  moving: WindowKey | null = null

  readonly wayland: boolean
  /** Whether the hooks replace the show and hide (macOS, Windows) or only observe them (GTK). */
  readonly replaces: boolean
  private topZ = 3
  private timers: number[] = []
  private drag: { key: WindowKey; startX: number; startY: number; from: Rect } | null = null

  constructor(
    readonly platform: WindowFramePlatform,
    private readonly options: HooksOptions,
    private readonly onChange: () => void,
  ) {
    this.wayland = platform === 'omarchy'
    this.replaces = osOf(platform) !== 'linux'
    this.showHook = options.showHook ?? true
    this.hideHook = options.hideHook ?? 'swallow'
    const blank = { x: 0, y: 0, width: 0, height: 0 }
    this.windows = Object.fromEntries(
      CREATION_ORDER.map((key, i) => [key, { key, id: WINDOWS[key].id, frame: blank, visible: false, readBack: null, z: i + 1 }]),
    ) as Record<WindowKey, HostWindow>
  }

  dispose() {
    for (const timer of this.timers) clearTimeout(timer)
    this.stopListening()
  }

  private later(ms: number, run: () => void) {
    this.timers.push(window.setTimeout(run, ms))
  }

  /**
   * The primary display's work area, measured from the desktop. The first
   * one starts the example: the hooks are installed, and the three windows
   * are created at the platform's default frame and shown, through the hook.
   */
  setWorkArea(workArea: Rect) {
    const first = this.workArea === null
    this.workArea = workArea
    if (!first) {
      this.emit()
      return
    }
    for (const key of CREATION_ORDER) this.windows[key].frame = defaultFrame(key, workArea)
    this.call(`getPrimary().workArea → ${rectText(workArea)}`)
    if (this.showHook) this.call('setWillShowHook(…)')
    if (this.hideHook !== 'none') this.call('setWillHideHook(…)')
    CREATION_ORDER.forEach((key, i) => this.later(160 * (i + 1), () => this.show(key)))
    if (this.options.resizeAtStart) this.later(1100, () => this.resizePrimary())
    this.emit()
  }

  get frontKey() {
    return CREATION_ORDER.filter(k => this.windows[k].visible).reduce<WindowKey | null>(
      (top, k) => (top === null || this.windows[k].z > this.windows[top].z ? k : top),
      null,
    )
  }

  focus(key: WindowKey) {
    if (key === this.frontKey) return
    this.windows[key].z = ++this.topZ
    this.emit()
  }

  // ---------------------------------------------------------------------------
  // Hooks

  setShowHook(on: boolean) {
    this.showHook = on
    this.call(on ? 'setWillShowHook(…)' : 'setWillShowHook(null)')
    this.lastEvent = on ? 'Will-show hook installed' : 'Will-show hook removed: windows show where they are'
    this.emit()
  }

  setHideHook(mode: HideHook) {
    this.hideHook = mode
    this.call(mode === 'none' ? 'setWillHideHook(null)' : 'setWillHideHook(…)')
    this.lastEvent =
      mode === 'none'
        ? 'Will-hide hook removed'
        : mode === 'swallow'
          ? 'Will-hide hook logs only, as in the Flutter example'
          : 'Will-hide hook logs, then calls callOriginalHide'
    this.emit()
  }

  // ---------------------------------------------------------------------------
  // Window calls

  show(key: WindowKey) {
    const window = this.windows[key]
    const { title } = WINDOWS[key]
    const workArea = this.workArea
    if (!workArea) return
    this.call(`show() #${window.id}`)
    if (this.showHook) {
      const frame = simulateHookPlacement(key, workArea)
      if (this.wayland) {
        this.hook(`will show #${window.id} (${title}): asked for ${rectText(frame)} · Hyprland tiles it instead`)
      } else {
        this.hook(`will show #${window.id} (${title}): placed at ${rectText(frame)}`)
        this.call(`setSize(${frame.width} × ${frame.height}, false) #${window.id}`)
        this.call(`setPosition(${frame.x}, ${frame.y}) #${window.id}`)
        this.setFrame(key, frame)
      }
      this.call(`callOriginalShow(#${window.id}) → true`)
    }
    window.visible = true
    window.z = ++this.topZ
    if (window.readBack === null) window.readBack = this.bounds(window)
    this.retile()
    this.lastEvent = `${title} shown`
    this.emit()
  }

  hide(key: WindowKey) {
    const window = this.windows[key]
    const { title } = WINDOWS[key]
    this.call(`hide() #${window.id}`)
    let hidden = true
    if (this.hideHook !== 'none') {
      if (this.hideHook === 'pass') {
        this.hook(`will hide #${window.id} (${title}): passed on`)
        this.call(`callOriginalHide(#${window.id}) → true`)
      } else if (this.replaces) {
        // The hook replaced the hide and never called the original.
        hidden = false
        this.hook(`will hide #${window.id} (${title}): logged only · the window stays`)
      } else {
        this.hook(`will hide #${window.id} (${title}): logged · GTK hides it anyway`)
      }
    }
    window.visible = window.visible && !hidden
    this.retile()
    this.lastEvent = hidden ? `${title} hidden` : `The will-hide hook swallowed the hide: ${title} stays on screen`
    this.emit()
  }

  /**
   * The primary window's button: find the window by its title, make it
   * 1000 × 1000 and show it — and the will-show hook puts it straight back
   * into its slot.
   */
  resizePrimary() {
    const window = this.windows.primary
    this.call(`getAll() → 3 windows · "Primary Window" is #${window.id}`)
    if (this.wayland) {
      this.call(`setSize(${RESIZE_TO.width} × ${RESIZE_TO.height}, false) #${window.id} → ignored: the window is tiled`)
    } else {
      this.call(`setSize(${RESIZE_TO.width} × ${RESIZE_TO.height}, false) #${window.id}`)
      this.setFrame('primary', { ...window.frame, ...RESIZE_TO })
    }
    this.lastEvent = `Primary Window resized to ${RESIZE_TO.width} × ${RESIZE_TO.height}`
    this.emit()
    // A beat, so the resize is seen before the hook takes it back.
    this.later(550, () => this.show('primary'))
  }

  /** "Read frame": what `Window.bounds` returns now. */
  readFrame(key: WindowKey) {
    const window = this.windows[key]
    window.readBack = this.bounds(window)
    this.call(`bounds #${window.id} → ${rectText(window.readBack)}`)
    this.emit()
  }

  private bounds(window: HostWindow): Rect {
    // Wayland never tells a client where its window is: x and y stay 0.
    return this.wayland ? { ...window.frame, x: 0, y: 0 } : { ...window.frame }
  }

  /** Moves a window; the read-back follows a change of size, as the example's view rebuilds on a resize. */
  private setFrame(key: WindowKey, frame: Rect) {
    const window = this.windows[key]
    const resized = frame.width !== window.frame.width || frame.height !== window.frame.height
    window.frame = frame
    if (resized || window.readBack === null) window.readBack = this.bounds(window)
  }

  /** Hyprland lays out whatever is visible; nothing the client asks for changes that. */
  private retile() {
    if (!this.wayland || !this.workArea) return
    const order = CREATION_ORDER.filter(k => this.windows[k].visible).sort((a, b) => this.windows[a].z - this.windows[b].z)
    const tiles = simulateHyprlandTiles(order, this.workArea)
    for (const key of order) this.setFrame(key, tiles[key]!)
  }

  // ---------------------------------------------------------------------------
  // The window manager moving a window by its title bar

  pressWindow(event: ReactPointerEvent, key: WindowKey) {
    const element = event.target as HTMLElement
    if (event.button !== 0 || !element.closest('.dz-window-frame__titlebar') || element.closest('button')) return
    event.preventDefault()
    this.drag = { key, startX: event.clientX, startY: event.clientY, from: { ...this.windows[key].frame } }
    window.addEventListener('pointermove', this.onMove)
    window.addEventListener('pointerup', this.onUp)
    window.addEventListener('pointercancel', this.onUp)
  }

  private onMove = (event: PointerEvent) => {
    const drag = this.drag
    if (!drag) return
    this.moving = drag.key
    this.windows[drag.key].frame = {
      ...drag.from,
      x: Math.round(drag.from.x + event.clientX - drag.startX),
      y: Math.round(drag.from.y + event.clientY - drag.startY),
    }
    this.emit()
  }

  private onUp = () => {
    const drag = this.drag
    this.stopListening()
    this.drag = null
    this.moving = null
    if (drag) this.lastEvent = `${WINDOWS[drag.key].title} moved: Read frame to see it`
    this.emit()
  }

  private stopListening() {
    window.removeEventListener('pointermove', this.onMove)
    window.removeEventListener('pointerup', this.onUp)
    window.removeEventListener('pointercancel', this.onUp)
  }

  clearLog() {
    this.log = []
    this.hookLog = []
    this.emit()
  }

  // ---------------------------------------------------------------------------

  private call(line: string) {
    this.log = [line, ...this.log].slice(0, LOG_LIMIT)
  }

  private hook(line: string) {
    this.hookLog = [line, ...this.hookLog].slice(0, LOG_LIMIT)
    this.call(line)
  }

  private emit() {
    this.onChange()
  }
}
