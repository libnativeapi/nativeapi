import type { PointerEvent as ReactPointerEvent } from 'react'

import { nowSeconds } from '../../components/use-now'
import { SCENES, WINDOW_SIZE } from './data'
import type { Os } from '../../components/platform'
import { clampLeft, indexForLeft, layoutFor, STRIP, type TabLayout, tabExtent } from './tab-layout'
import type { BrowserTab, BrowserWindow, DragMode, Point, Rect, Scene } from './types'

/** What the HUD shows of the session while a gesture runs. */
export interface SessionHud {
  mode: DragMode
  /** The cursor, as the last `WindowDragMovedEvent` reported it. */
  cursor: Point | null
  /** The window the session moves, or null while it only follows the pointer. */
  sessionWindow: number | null
  /** The last `getWindowAtPoint` answer: a window id, null for none, undefined when not asked. */
  hit: number | null | undefined
  /** `WindowDragMovedEvent`s in this gesture. */
  moves: number
}

interface Press {
  kind: 'tab' | 'strip'
  windowId: number
  tabId: number | null
  /** Where the tab was grabbed, relative to its top-left corner. */
  grab: Point
  /** Desktop position of the press. */
  press: Point
}

interface Drag extends Press {
  mode: Exclude<DragMode, 'idle'>
  /** False on Wayland, where `WindowDragSession.start` returns false and the window's own pointer events drive the strip. */
  session: boolean
  /** The dragged tab's left edge, relative to the strip, while `inStrip`. */
  left: number
  /** The point of the moving window's frame that stays under the cursor. */
  anchor: Point
}

const LOG_LIMIT = 200
const contains = (r: Rect, p: Point) => p.x >= r.x && p.x < r.x + r.width && p.y >= r.y && p.y < r.y + r.height
const xy = (p: Point) => `${Math.round(p.x)}, ${Math.round(p.y)}`

/**
 * Chrome-style tab dragging on top of `WindowDragSession`, simulated as the
 * example's `TabsController` runs it: a press on a tab starts a pointer-only
 * session, and every step after it is driven by the session's cursor — the
 * page's own pointer events stop mattering once a tab has moved to another
 * window, which never saw the press. Here the "session" is a listener on the
 * whole page for the same reason: the tab under the pointer remounts in
 * another window, and pointer capture would go with it.
 */
export class TabsSimulation {
  windows: BrowserWindow[] = []
  hud: SessionHud = { mode: 'idle', cursor: null, sessionWindow: null, hit: undefined, moves: 0 }
  /** Calls and events, newest first. */
  log: string[] = []
  lastEvent = 'No drag yet'
  /** The last window closed, and the app with it. */
  quit = false

  private pressed: Press | null = null
  private drag: Drag | null = null
  private desktop: HTMLElement | null = null
  private nextTabId = 1
  private nextWindowId = 1
  private nextInstance = 1
  private topZ = 1

  readonly layout: TabLayout

  constructor(
    readonly os: Os,
    /** Hyprland: no global cursor, so no session. */
    readonly wayland: boolean,
    scene: Scene,
    private readonly onChange: () => void,
  ) {
    this.layout = layoutFor(os)
    for (const spec of SCENES[scene]) {
      const tabs = Array.from({ length: spec.tabs }, () => this.createTab())
      if (spec.moved) for (const tab of tabs) tab.page.moves = 1
      this.openWindow(tabs, { x: spec.x, y: spec.y }, { width: spec.width ?? WINDOW_SIZE.width, height: spec.height ?? WINDOW_SIZE.height })
    }
    // The strip takes the title bar's place: under a transparent one on macOS,
    // instead of a hidden one elsewhere.
    const chrome = os === 'macos' ? 'setContentUnderTitleBar(true) → true' : 'setTitleBarStyle(hidden)'
    this.log = [
      'WindowDragSession.create() → session',
      ...this.windows.map(w => `Window #${w.id}: ${chrome} · ${w.tabs.length} tabs`),
    ].reverse()
  }

  dispose() {
    this.detachListeners()
  }

  // ---------------------------------------------------------------------------
  // Queries for the strips

  window(id: number) {
    return this.windows.find(w => w.id === id)
  }

  get frontId() {
    return this.windows.reduce<BrowserWindow | null>((top, w) => (!top || w.z > top.z ? w : top), null)?.id ?? null
  }

  /** The dragged tab's left edge while it slides along `windowId`'s strip, or null if it sits in its slot. */
  draggedLeft(windowId: number, tabId: number) {
    const drag = this.drag
    if (!drag || drag.tabId !== tabId || drag.windowId !== windowId || drag.mode !== 'inStrip') return null
    return drag.left
  }

  isDragging(tabId: number) {
    return this.drag?.tabId === tabId && this.drag.mode !== 'pending'
  }

  // ---------------------------------------------------------------------------
  // Tabs and windows

  private createTab(): BrowserTab {
    const id = this.nextTabId++
    return {
      id,
      title: `Tab ${id}`,
      hue: id - 1,
      page: {
        instance: this.nextInstance++,
        openedAt: nowSeconds(),
        address: `https://example.com/tab-${id}`,
        likes: 0,
        scroll: 0,
        moves: 0,
      },
    }
  }

  private openWindow(tabs: BrowserTab[], at: Point, size: { width: number; height: number }) {
    const window: BrowserWindow = {
      id: this.nextWindowId++,
      x: Math.round(at.x),
      y: Math.round(at.y),
      ...size,
      z: ++this.topZ,
      tabs,
      activeTabId: tabs[0]!.id,
    }
    this.windows.push(window)
    return window
  }

  focus(windowId: number) {
    const window = this.window(windowId)
    if (!window || window.id === this.frontId) return
    window.z = ++this.topZ
    this.emit()
  }

  activate(windowId: number, tabId: number) {
    const window = this.window(windowId)
    if (!window || window.activeTabId === tabId) return
    window.activeTabId = tabId
    this.emit()
  }

  addTab(windowId: number) {
    const window = this.window(windowId)
    if (!window) return
    const tab = this.createTab()
    window.tabs.push(tab)
    window.activeTabId = tab.id
    this.event(`Opened ${tab.title} in window #${window.id}`)
  }

  closeTab(windowId: number, tabId: number) {
    const window = this.window(windowId)
    if (!window || this.drag?.tabId === tabId) return
    const tab = window.tabs.find(t => t.id === tabId)
    this.removeTab(window, tabId)
    if (window.tabs.length === 0) {
      this.closeWindow(window.id, `Closed ${tab?.title ?? 'the tab'}, the last of window #${window.id}`)
    } else {
      this.event(`Closed ${tab?.title ?? 'a tab'}`)
    }
  }

  closeWindow(windowId: number, why = `Closed window #${windowId}`) {
    const index = this.windows.findIndex(w => w.id === windowId)
    if (index < 0) return
    if (this.drag?.windowId === windowId) {
      this.call('WindowDragSession.cancel()')
      this.endDrag()
    }
    this.windows.splice(index, 1)
    this.call(`hide() #${windowId} · destroy()`)
    if (this.windows.length === 0) {
      this.quit = true
      this.event('The last window closed, so the app quit')
    } else {
      this.event(why)
    }
  }

  private removeTab(window: BrowserWindow, tabId: number) {
    const index = window.tabs.findIndex(t => t.id === tabId)
    if (index < 0) return
    window.tabs.splice(index, 1)
    if (window.activeTabId === tabId && window.tabs.length > 0) {
      window.activeTabId = window.tabs[Math.min(index, window.tabs.length - 1)]!.id
    }
  }

  /** A page's state changed (address, likes): redraw. */
  touch() {
    this.emit()
  }

  clearLog() {
    this.log = []
    this.lastEvent = 'No drag yet'
    this.emit()
  }

  // ---------------------------------------------------------------------------
  // Pointer: the press, then the "session"

  pressTab(event: ReactPointerEvent, windowId: number, tabId: number) {
    if (event.button !== 0 || this.drag) return
    const tab = (event.currentTarget as HTMLElement).getBoundingClientRect()
    this.press(event, { kind: 'tab', windowId, tabId, grab: { x: event.clientX - tab.left, y: event.clientY - tab.top } })
    this.window(windowId)!.activeTabId = tabId
    this.focus(windowId)
    this.emit()
  }

  /** A press on the empty strip: it moves the window, like a title bar. */
  pressStrip(event: ReactPointerEvent, windowId: number) {
    if (event.button !== 0 || this.drag || event.target !== event.currentTarget) return
    this.press(event, { kind: 'strip', windowId, tabId: null, grab: { x: 0, y: 0 } })
    this.focus(windowId)
  }

  private press(event: ReactPointerEvent, press: Omit<Press, 'press'>) {
    event.preventDefault()
    this.desktop = (event.currentTarget as HTMLElement).closest('.desktop-stage__desktop')
    this.pressed = { ...press, press: this.toDesktop(event.clientX, event.clientY) }
    window.addEventListener('pointermove', this.onPointerMove)
    window.addEventListener('pointerup', this.onPointerUp)
    window.addEventListener('pointercancel', this.onPointerUp)
    window.addEventListener('keydown', this.onKeyDown)
  }

  private detachListeners() {
    window.removeEventListener('pointermove', this.onPointerMove)
    window.removeEventListener('pointerup', this.onPointerUp)
    window.removeEventListener('pointercancel', this.onPointerUp)
    window.removeEventListener('keydown', this.onKeyDown)
  }

  private onPointerMove = (event: PointerEvent) => {
    const cursor = this.toDesktop(event.clientX, event.clientY)
    if (!this.drag) {
      const pressed = this.pressed
      // A click is not a drag: the session starts once the pointer moves.
      if (!pressed || Math.hypot(cursor.x - pressed.press.x, cursor.y - pressed.press.y) < 3) return
      this.begin(pressed)
    }
    this.move(cursor)
  }

  private onPointerUp = (event: PointerEvent) => {
    this.detachListeners()
    this.pressed = null
    const drag = this.drag
    if (!drag) return
    const cursor = this.toDesktop(event.clientX, event.clientY)
    if (drag.session) this.call(`WindowDragEndedEvent → ${xy(cursor)}`)
    const window = this.window(drag.windowId)
    const tab = window?.tabs.find(t => t.id === drag.tabId)
    if (drag.mode === 'window' && window) {
      this.call(`focus() #${window.id}`)
      this.lastEvent = `${tab?.title ?? 'The tab'} stays in window #${window.id}`
    } else if (drag.mode === 'inStrip' && window && tab) {
      this.lastEvent = `${tab.title} dropped at position ${window.tabs.indexOf(tab) + 1} of window #${window.id}`
    } else if (drag.mode === 'moveWindow' && window) {
      this.lastEvent = `Window #${window.id} moved to ${window.x}, ${window.y}`
    }
    this.endDrag()
    this.emit()
  }

  private onKeyDown = (event: KeyboardEvent) => {
    if (event.key !== 'Escape' || !this.drag) return
    this.detachListeners()
    this.pressed = null
    if (this.drag.session) {
      this.call('WindowDragSession.cancel()')
      this.call('WindowDragCancelledEvent')
    }
    this.lastEvent = 'Drag cancelled: everything stays where it is'
    this.endDrag()
    this.emit()
  }

  private endDrag() {
    this.drag = null
    this.hud = { ...this.hud, mode: 'idle', sessionWindow: null, hit: undefined }
  }

  private begin(pressed: Press) {
    const window = this.window(pressed.windowId)
    if (!window) return
    if (pressed.kind === 'strip') {
      const anchor = { x: pressed.press.x - window.x, y: pressed.press.y - window.y }
      if (this.wayland) {
        // No session on Wayland, but the compositor moves a window for us.
        this.call(`WindowDragSession.start(window #${window.id}) → false · Wayland`)
        this.call(`startDragging() #${window.id} → true · the compositor moves it`)
      } else {
        this.call(`WindowDragSession.start(window #${window.id}, anchor ${xy(anchor)}) → true`)
      }
      this.drag = { ...pressed, mode: 'moveWindow', session: !this.wayland, left: 0, anchor }
      this.hud = { mode: 'moveWindow', cursor: pressed.press, sessionWindow: window.id, hit: undefined, moves: 0 }
      return
    }
    if (this.wayland) {
      this.call('WindowDragSession.start(null, 0, 0) → false · Wayland: reorder only')
    } else {
      this.call('WindowDragSession.start(null, 0, 0) → true · following the pointer')
    }
    this.drag = { ...pressed, mode: 'pending', session: !this.wayland, left: 0, anchor: { x: 0, y: 0 } }
    this.hud = { mode: 'pending', cursor: pressed.press, sessionWindow: null, hit: undefined, moves: 0 }
  }

  // ---------------------------------------------------------------------------
  // The state machine, driven by the cursor

  private move(cursor: Point) {
    const drag = this.drag
    if (!drag) return
    this.hud.cursor = cursor
    if (drag.session) {
      this.hud.moves++
      // One line for the run of moves, so the log stays readable.
      const line = `WindowDragMovedEvent ×${this.hud.moves} → ${xy(cursor)}`
      if (this.log[0]?.startsWith('WindowDragMovedEvent')) this.log[0] = line
      else this.call(line)
    }
    switch (drag.mode) {
      case 'moveWindow':
        this.moveWindowTo(drag.windowId, cursor, drag.anchor)
        break
      case 'pending':
        if (Math.hypot(cursor.x - drag.press.x, cursor.y - drag.press.y) < STRIP.popOutDistance) break
        drag.mode = 'inStrip'
        this.hud.mode = 'inStrip'
        this.moveInStrip(drag, cursor)
        break
      case 'inStrip':
        this.moveInStrip(drag, cursor)
        break
      case 'window': {
        this.moveWindowTo(drag.windowId, cursor, drag.anchor)
        const target = this.windowWithStripAt(cursor, drag.windowId)
        if (target) this.mergeInto(drag, target, cursor)
        break
      }
    }
    this.emit()
  }

  private moveWindowTo(windowId: number, cursor: Point, anchor: Point) {
    const window = this.window(windowId)
    if (!window) return
    window.x = Math.round(cursor.x - anchor.x)
    window.y = Math.round(cursor.y - anchor.y)
  }

  private moveInStrip(drag: Drag, cursor: Point) {
    const strip = this.stripRect(drag.windowId)
    if (!strip) return
    const outside = cursor.y < strip.y - STRIP.detachMargin || cursor.y > strip.y + strip.height + STRIP.detachMargin
    // Wayland cannot place a window under the cursor: the tab stays in its strip.
    if (outside && drag.session) {
      this.tearOff(drag, cursor, strip)
      return
    }
    this.placeInStrip(drag, this.window(drag.windowId)!, strip, cursor)
  }

  /** Puts the dragged tab under the cursor in `window`'s strip, at the index it now covers. */
  private placeInStrip(drag: Drag, window: BrowserWindow, strip: Rect, cursor: Point) {
    const count = window.tabs.length
    const extent = tabExtent(this.layout, strip.width, count)
    drag.left = clampLeft(this.layout, cursor.x - strip.x - drag.grab.x, extent, count)
    const index = indexForLeft(this.layout, drag.left, extent, count)
    const current = window.tabs.findIndex(t => t.id === drag.tabId)
    if (current >= 0 && index !== current) {
      const [tab] = window.tabs.splice(current, 1)
      window.tabs.splice(index, 0, tab!)
    }
  }

  private tearOff(drag: Drag, cursor: Point, strip: Rect) {
    const source = this.window(drag.windowId)!
    const tab = source.tabs.find(t => t.id === drag.tabId)!
    // Keep the grabbed point of the tab, now the window's first, under the cursor.
    const anchor = {
      x: strip.x - source.x + this.layout.leadingInset + drag.grab.x,
      y: strip.y - source.y + STRIP.tabTop + drag.grab.y,
    }
    drag.anchor = anchor
    drag.mode = 'window'
    this.hud.mode = 'window'

    if (source.tabs.length === 1) {
      // Nothing would be left behind: carry the whole window instead.
      this.call(`WindowDragSession.start(window #${source.id}, anchor ${xy(anchor)}) → true · its only tab`)
      this.hud.sessionWindow = source.id
      this.moveWindowTo(source.id, cursor, anchor)
      this.lastEvent = `${tab.title} is the only tab: window #${source.id} follows the cursor`
      return
    }

    this.removeTab(source, tab.id)
    tab.page.moves++
    const torn = this.openWindow([tab], { x: cursor.x - anchor.x, y: cursor.y - anchor.y }, {
      width: source.width,
      height: source.height,
    })
    drag.windowId = torn.id
    this.hud.sessionWindow = torn.id
    this.call(`Window #${torn.id} created · contentSize ${source.width} × ${source.height}`)
    this.call(`WindowDragSession.start(window #${torn.id}, anchor ${xy(anchor)}) → true · retargeted`)
    this.call(`focus() #${torn.id}`)
    this.lastEvent = `Tore ${tab.title} off into window #${torn.id}`
  }

  private mergeInto(drag: Drag, target: BrowserWindow, cursor: Point) {
    const strip = this.stripRect(target.id)
    const dragged = this.window(drag.windowId)
    if (!strip || !dragged) return
    const tab = dragged.tabs.find(t => t.id === drag.tabId)!

    // Stop moving the dragged window before it goes away; the gesture goes on
    // as a drag along the target's strip.
    this.call('WindowDragSession.start(null, 0, 0) → true · following the pointer again')
    this.removeTab(dragged, tab.id)
    this.windows.splice(this.windows.indexOf(dragged), 1)
    this.call(`hide() #${dragged.id} · destroy()`)

    target.tabs.push(tab)
    target.activeTabId = tab.id
    tab.page.moves++
    drag.windowId = target.id
    drag.mode = 'inStrip'
    this.hud.mode = 'inStrip'
    this.hud.sessionWindow = null
    this.placeInStrip(drag, target, strip, cursor)
    target.z = ++this.topZ
    this.call(`focus() #${target.id}`)
    this.lastEvent = `Merged ${tab.title} into window #${target.id}`
  }

  // ---------------------------------------------------------------------------
  // Hit testing

  /**
   * `WindowManager.getWindowAtPoint(cursor, excluding)`: the frontmost window
   * under the cursor, looking through the dragged one — and its strip, if the
   * cursor is on it.
   */
  private windowWithStripAt(cursor: Point, excluding: number) {
    const hit =
      [...this.windows]
        .sort((a, b) => b.z - a.z)
        .find(w => w.id !== excluding && contains(this.frameRect(w), cursor)) ?? null
    const id = hit?.id ?? null
    if (id !== this.hud.hit) {
      this.call(`getWindowAtPoint(${xy(cursor)}, excluding #${excluding}) → ${id === null ? 'null' : `#${id}`}`)
      this.hud.hit = id
    }
    if (!hit) return null
    const strip = this.stripRect(hit.id)
    return strip && contains(strip, cursor) ? hit : null
  }

  private toDesktop(clientX: number, clientY: number): Point {
    const origin = this.desktop?.getBoundingClientRect()
    return { x: clientX - (origin?.left ?? 0), y: clientY - (origin?.top ?? 0) }
  }

  private rectOf(selector: string): Rect | null {
    const element = this.desktop?.querySelector(selector)
    const origin = this.desktop?.getBoundingClientRect()
    if (!element || !origin) return null
    const r = element.getBoundingClientRect()
    return { x: r.left - origin.left, y: r.top - origin.top, width: r.width, height: r.height }
  }

  /** A window's tab strip, in desktop coordinates. */
  private stripRect(windowId: number) {
    return this.rectOf(`[data-browser-strip="${windowId}"]`)
  }

  private frameRect(window: BrowserWindow): Rect {
    return this.rectOf(`[data-browser-window="${window.id}"]`) ?? window
  }

  // ---------------------------------------------------------------------------

  private call(line: string) {
    this.log = [line, ...this.log].slice(0, LOG_LIMIT)
  }

  private event(label: string) {
    this.lastEvent = label
    this.emit()
  }

  private emit() {
    this.onChange()
  }
}
