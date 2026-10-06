import type { PointerEvent as ReactPointerEvent } from 'react'

import { HOME_SLOTS, MAIN_PLACES, PANELS, POP_OUT_DISTANCE, SLOT_IDS, SLOTS } from './data'
import type {
  DragMode,
  FloatingWindow,
  InspectorState,
  Layout,
  MainId,
  MainWindow,
  PanelCommon,
  PanelId,
  PanelPlace,
  Point,
  Rect,
  SlotId,
  StopwatchState,
} from './types'

/** What the HUD shows of the session while a gesture runs. */
export interface SessionHud {
  mode: DragMode
  cursor: Point | null
  /** The window the session moves, or null while it only follows the pointer. */
  sessionWindow: number | null
  /** The last `getWindowAtPoint` answer: a window id, null for none, undefined when not asked. */
  hit: number | null | undefined
  moves: number
}

interface Press {
  panel: PanelId
  press: Point
  /** Where the panel was pressed, relative to its content's top-left. */
  pointerInItem: Point
  itemSize: { width: number; height: number }
}

interface Drag extends Press {
  mode: Exclude<DragMode, 'idle'>
  /** Where the panel was docked when the drag began, if it was. */
  restoreSlot: SlotId | null
}

/** The window manager moving a window by its title bar: not the example's doing. */
interface Move {
  target: { main: MainId } | { panel: PanelId }
  press: Point
  from: Point
}

const LOG_LIMIT = 200
const contains = (r: Rect, p: Point) => p.x >= r.x && p.x < r.x + r.width && p.y >= r.y && p.y < r.y + r.height
const xy = (p: Point) => `${Math.round(p.x)}, ${Math.round(p.y)}`
const two = (v: number) => String(v).padStart(2, '0')
const clock = (d = new Date()) => `${two(d.getHours())}:${two(d.getMinutes())}:${two(d.getSeconds())}`

/**
 * The example's `DetachController`, simulated: which panel sits in which
 * slot, which float in windows of their own, and the tear-off gesture on top
 * of `WindowDragSession`.
 *
 * 1. A press on a docked panel's header starts a pointer-only session.
 * 2. Once the cursor is 8px from the press, the panel is torn off: a window
 *    the size of its slot opens exactly where it was, and the session is
 *    retargeted so the window follows the cursor.
 * 3. While it moves, `getWindowAtPoint` (looking through the moving window)
 *    finds the window under the cursor; an empty slot there is the drop
 *    target, highlighted, and the moving window turns translucent.
 * 4. On release the panel docks into the target; anywhere else it stays a
 *    window of its own.
 *
 * The "session" listens on the whole page, as the native one does globally:
 * the header under the pointer moves to another window mid-gesture.
 */
export class DetachSimulation {
  mains: MainWindow[]
  places: Record<PanelId, PanelPlace>
  inspector: InspectorState
  stopwatch: StopwatchState
  hud: SessionHud = { mode: 'idle', cursor: null, sessionWindow: null, hit: undefined, moves: 0 }
  /** The empty slot the dragged window would dock into if released now. */
  hovered: SlotId | null = null
  /** What the controller did, newest first, for the workspaces' Activity. */
  activity: string[] = []
  /** Calls and events, newest first, for the HUD. */
  log: string[] = []
  lastEvent = 'No drag yet'
  quit = false
  /** Where a floating window's content starts inside its frame: the title bar's height. */
  inset: Point = { x: 0, y: 32 }

  private desktop: HTMLElement | null = null
  private pressed: Press | null = null
  private drag: Drag | null = null
  private moving: Move | null = null
  private nextNativeId = 3
  private topZ = 2

  constructor(
    /** Hyprland: no global cursor, so no session. */
    readonly wayland: boolean,
    layout: Layout,
    private readonly onChange: () => void,
  ) {
    this.mains = (['A', 'B'] as const).map((id, i) => ({ id, nativeId: i + 1, ...MAIN_PLACES[id], z: i + 1 }))
    const createdAt = clock()
    const common = (instance: number): PanelCommon => ({ instance, createdAt, moves: 0, shownIn: '' })
    this.inspector = { ...common(1), name: 'Untitled layer', clicks: 0, visible: true, opacity: 80, selected: 3, scroll: 0 }
    this.stopwatch = { ...common(2), elapsed: 0, startedAt: performance.now(), laps: [] }
    this.places = { inspector: this.placeFor(layout.inspector), stopwatch: this.placeFor(layout.stopwatch) }
    for (const id of ['inspector', 'stopwatch'] as const) this.panel(id).shownIn = this.windowKey(this.places[id])
    this.log = [
      'WindowDragSession.create() → session',
      `DetachableItem("inspector") → ${this.describe('inspector')}`,
      `DetachableItem("stopwatch") → ${this.describe('stopwatch')}`,
    ].reverse()
  }

  private placeFor(spec: Layout[PanelId]): PanelPlace {
    if (typeof spec === 'string') return { kind: 'docked', slot: spec }
    return { kind: 'floating', window: { nativeId: this.nextNativeId++, ...spec, z: ++this.topZ, opacity: 1 } }
  }

  /** The desktop the windows stand on, for hit testing in its coordinates. */
  attach(desktop: HTMLElement | null) {
    this.desktop = desktop
  }

  dispose() {
    this.detachListeners()
  }

  // ---------------------------------------------------------------------------
  // Queries

  panel(id: PanelId): PanelCommon {
    return id === 'inspector' ? this.inspector : this.stopwatch
  }

  itemInSlot(slot: SlotId): PanelId | null {
    for (const id of ['inspector', 'stopwatch'] as const) {
      const place = this.places[id]
      if (place.kind === 'docked' && place.slot === slot) return id
    }
    return null
  }

  floating(id: PanelId): FloatingWindow | null {
    const place = this.places[id]
    return place.kind === 'floating' ? place.window : null
  }

  get floatingPanels(): PanelId[] {
    return (['inspector', 'stopwatch'] as const).filter(id => this.places[id].kind === 'floating')
  }

  main(id: MainId) {
    return this.mains.find(m => m.id === id)
  }

  /** The native id of the frontmost window. */
  get frontId() {
    const all = [...this.mains.map(m => ({ id: m.nativeId, z: m.z })), ...this.floatingPanels.map(p => this.floating(p)!).map(w => ({ id: w.nativeId, z: w.z }))]
    return all.reduce<{ id: number; z: number } | null>((top, w) => (!top || w.z > top.z ? w : top), null)?.id ?? null
  }

  /** Stacking ranks for every window, so they all stay under the HUD. */
  rank(nativeId: number) {
    const all = [...this.mains.map(m => ({ id: m.nativeId, z: m.z })), ...this.floatingPanels.map(p => this.floating(p)!).map(w => ({ id: w.nativeId, z: w.z }))]
    return all.sort((a, b) => a.z - b.z).findIndex(w => w.id === nativeId) + 1
  }

  /** Whether a floating panel's window is being dragged, which lights every empty slot. */
  get isMovingWindow() {
    return this.drag?.mode === 'floating'
  }

  describe(id: PanelId) {
    const place = this.places[id]
    return place.kind === 'docked' ? `docked ${place.slot}` : `floating · window #${place.window.nativeId}`
  }

  private windowKey(place: PanelPlace) {
    return place.kind === 'docked' ? SLOTS[place.slot].window : `#${place.window.nativeId}`
  }

  // ---------------------------------------------------------------------------
  // Commands

  /** Moves a panel; a move to another window is one more for `moved between windows`. */
  private setPlace(id: PanelId, place: PanelPlace) {
    this.places[id] = place
    const state = this.panel(id)
    const key = this.windowKey(place)
    if (key !== state.shownIn) {
      state.moves++
      state.shownIn = key
    }
  }

  private tearOff(id: PanelId, at: Point, size: { width: number; height: number }) {
    const window: FloatingWindow = { nativeId: this.nextNativeId++, x: Math.round(at.x), y: Math.round(at.y), ...size, z: ++this.topZ, opacity: 1 }
    this.setPlace(id, { kind: 'floating', window })
    this.call(`Window #${window.nativeId} created · contentSize ${Math.round(size.width)} × ${Math.round(size.height)}`)
    return window
  }

  /** "Open in a window": the panel moves into its own window without a drag. */
  float(id: PanelId) {
    const existing = this.floating(id)
    if (existing) {
      this.focusWindow(existing.nativeId)
      this.call(`focus() #${existing.nativeId}`)
      this.emit()
      return
    }
    const place = this.places[id]
    const slot = place.kind === 'docked' ? this.slotRect(place.slot) : null
    const size = slot ? { width: slot.width, height: slot.height } : { width: 320, height: 420 }
    const window = this.tearOff(id, slot ? { x: slot.x + 32, y: slot.y + 32 } : { x: 120, y: 120 }, size)
    this.call(`setPosition(${xy({ x: window.x - this.inset.x, y: window.y - this.inset.y })}) #${window.nativeId}`)
    this.call(`focus() #${window.nativeId}`)
    this.note(`Opened "${PANELS[id].title}" in window #${window.nativeId}`)
  }

  /** Docks a panel into `slot`, closing its window if it was floating. */
  private dockSilently(id: PanelId, slot: SlotId) {
    if (!this.main(SLOTS[slot].window)) return false
    const occupant = this.itemInSlot(slot)
    if (occupant && occupant !== id) return false
    const window = this.floating(id)
    this.setPlace(id, { kind: 'docked', slot })
    if (window) this.call(`hide() #${window.nativeId} · destroy()`)
    return true
  }

  dock(id: PanelId, slot: SlotId) {
    if (this.dockSilently(id, slot)) this.note(`Docked "${PANELS[id].title}" into ${slot}`)
  }

  /** "Dock back": home first, then any empty slot. False when every slot is taken. */
  dockAnywhere(id: PanelId) {
    const home = HOME_SLOTS[id]
    for (const slot of [home, ...SLOT_IDS.filter(s => s !== home)]) {
      if (this.main(SLOTS[slot].window) && this.itemInSlot(slot) === null) {
        this.dock(id, slot)
        return true
      }
    }
    return false
  }

  /** Closing a floating window puts the panel back rather than discarding it. */
  closeFloating(id: PanelId) {
    this.call(`onWindowCloseRequested #${this.floating(id)?.nativeId}`)
    if (!this.dockAnywhere(id)) {
      this.lastEvent = 'Every slot is taken: the window stays open'
      this.emit()
    }
  }

  /** Closing a main window pops its panels out instead of losing them; the app ends with the last one. */
  closeMain(id: MainId) {
    const main = this.main(id)
    if (!main) return
    for (const slot of SLOT_IDS.filter(s => SLOTS[s].window === id)) {
      const panel = this.itemInSlot(slot)
      if (panel) this.float(panel)
    }
    this.mains = this.mains.filter(m => m.id !== id)
    this.call(`hide() #${main.nativeId} · destroy()`)
    if (this.mains.length === 0) {
      this.quit = true
      this.note('The last main window closed, so the app quit')
    } else {
      this.note(`Closed Window ${id}`)
    }
  }

  focusWindow(nativeId: number) {
    if (nativeId === this.frontId) return
    const main = this.mains.find(m => m.nativeId === nativeId)
    if (main) main.z = ++this.topZ
    for (const id of this.floatingPanels) {
      const window = this.floating(id)!
      if (window.nativeId === nativeId) window.z = ++this.topZ
    }
    this.emit()
  }

  /** The floating frame told us how tall its title bar is. */
  setInset(inset: Point) {
    if (inset.x === this.inset.x && inset.y === this.inset.y) return
    this.inset = inset
    this.emit()
  }

  /** A panel's own state changed: redraw. */
  touch() {
    this.emit()
  }

  clearLog() {
    this.log = []
    this.lastEvent = 'No drag yet'
    this.emit()
  }

  // ---------------------------------------------------------------------------
  // Pointer

  /** A press on a panel's header: the grip the example is about. */
  pressHeader(event: ReactPointerEvent, id: PanelId) {
    if (event.button !== 0 || this.drag || (event.target as HTMLElement).closest('button')) return
    event.preventDefault()
    event.stopPropagation()
    const content = (event.currentTarget as HTMLElement).closest('[data-panel]')!.getBoundingClientRect()
    this.pressed = {
      panel: id,
      press: this.toDesktop(event.clientX, event.clientY),
      pointerInItem: { x: event.clientX - content.left, y: event.clientY - content.top },
      itemSize: { width: Math.round(content.width), height: Math.round(content.height) },
    }
    const window = this.floating(id)
    if (window) this.focusWindow(window.nativeId)
    this.listen()
  }

  /** A press on a window's own title bar: the window manager moves the window. */
  pressWindow(event: ReactPointerEvent, target: Move['target']) {
    const element = event.target as HTMLElement
    if (event.button !== 0 || this.drag || !element.closest('.dz-window-frame__titlebar') || element.closest('button')) return
    event.preventDefault()
    const window = 'main' in target ? this.main(target.main) : this.floating(target.panel)
    if (!window) return
    this.moving = { target, press: this.toDesktop(event.clientX, event.clientY), from: { x: window.x, y: window.y } }
    this.listen()
  }

  private listen() {
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
    const moving = this.moving
    if (moving) {
      const window = 'main' in moving.target ? this.main(moving.target.main) : this.floating(moving.target.panel)
      if (window) {
        window.x = Math.round(moving.from.x + cursor.x - moving.press.x)
        window.y = Math.round(moving.from.y + cursor.y - moving.press.y)
        this.emit()
      }
      return
    }
    if (!this.drag) {
      const pressed = this.pressed
      if (!pressed || Math.hypot(cursor.x - pressed.press.x, cursor.y - pressed.press.y) < 3) return
      if (!this.begin(pressed)) return
    }
    this.move(cursor)
  }

  private onPointerUp = (event: PointerEvent) => {
    this.detachListeners()
    this.pressed = null
    if (this.moving) {
      const window = 'main' in this.moving.target ? this.main(this.moving.target.main) : this.floating(this.moving.target.panel)
      this.moving = null
      if (window) this.lastEvent = `Window moved to ${window.x}, ${window.y}`
      this.emit()
      return
    }
    const drag = this.drag
    if (!drag) return
    this.call(`WindowDragEndedEvent → ${xy(this.toDesktop(event.clientX, event.clientY))}`)
    this.drag = null
    this.hud = { ...this.hud, mode: 'idle', sessionWindow: null, hit: undefined }
    if (drag.mode === 'pending') {
      // Released before popping out: nothing changed.
      this.emit()
      return
    }
    // Exactly what the highlight promised: the last move already evaluated the release position.
    const target = this.hovered
    const title = PANELS[drag.panel].title
    this.setHovered(null)
    if (target === null) {
      const window = this.floating(drag.panel)
      if (window) this.call(`focus() #${window.nativeId}`)
      if (drag.restoreSlot) this.note(`Detached "${title}" into its own window`)
      else this.lastEvent = `"${title}" stays in window #${window?.nativeId}`
    } else if (target === drag.restoreSlot) {
      this.dockSilently(drag.panel, target)
      this.note(`"${title}" was dropped back onto ${target}`)
    } else {
      this.dock(drag.panel, target)
    }
    this.emit()
  }

  private onKeyDown = (event: KeyboardEvent) => {
    if (event.key !== 'Escape' || !this.drag) return
    this.detachListeners()
    this.pressed = null
    const drag = this.drag
    this.drag = null
    this.hud = { ...this.hud, mode: 'idle', sessionWindow: null, hit: undefined }
    this.call('WindowDragSession.cancel()')
    this.call('WindowDragCancelledEvent')
    this.setHovered(null)
    if (drag.mode === 'floating' && drag.restoreSlot) this.dockSilently(drag.panel, drag.restoreSlot)
    this.lastEvent = 'Drag cancelled'
    this.emit()
  }

  private begin(pressed: Press) {
    this.pressed = null
    const window = this.floating(pressed.panel)
    if (this.wayland) {
      // The session cannot follow the cursor: the drag ends before it starts.
      this.detachListeners()
      this.call(`WindowDragSession.start(${window ? `window #${window.nativeId}` : 'null'}) → false · Wayland`)
      this.lastEvent = 'No drag on Wayland: use Open in a window and Dock back'
      this.emit()
      return false
    }
    if (window) {
      const anchor = { x: this.inset.x + pressed.pointerInItem.x, y: this.inset.y + pressed.pointerInItem.y }
      this.call(`WindowDragSession.start(window #${window.nativeId}, anchor ${xy(anchor)}) → true`)
      this.drag = { ...pressed, mode: 'floating', restoreSlot: null }
      this.hud = { mode: 'floating', cursor: pressed.press, sessionWindow: window.nativeId, hit: undefined, moves: 0 }
    } else {
      const place = this.places[pressed.panel]
      this.call('WindowDragSession.start(null, 0, 0) → true · following the pointer')
      this.drag = { ...pressed, mode: 'pending', restoreSlot: place.kind === 'docked' ? place.slot : null }
      this.hud = { mode: 'pending', cursor: pressed.press, sessionWindow: null, hit: undefined, moves: 0 }
    }
    return true
  }

  private move(cursor: Point) {
    const drag = this.drag
    if (!drag) return
    this.hud.cursor = cursor
    this.hud.moves++
    const line = `WindowDragMovedEvent ×${this.hud.moves} → ${xy(cursor)}`
    if (this.log[0]?.startsWith('WindowDragMovedEvent')) this.log[0] = line
    else this.call(line)

    if (drag.mode === 'pending') {
      // Shorter drags change nothing, so there is no flicker.
      if (Math.hypot(cursor.x - drag.press.x, cursor.y - drag.press.y) < POP_OUT_DISTANCE) return this.emit()
      // A window exactly where the panel was, with the pressed point under the cursor.
      const window = this.tearOff(drag.panel, { x: cursor.x - drag.pointerInItem.x, y: cursor.y - drag.pointerInItem.y }, drag.itemSize)
      const anchor = { x: this.inset.x + drag.pointerInItem.x, y: this.inset.y + drag.pointerInItem.y }
      this.call(`WindowDragSession.start(window #${window.nativeId}, anchor ${xy(anchor)}) → true · retargeted`)
      this.call(`focus() #${window.nativeId}`)
      drag.mode = 'floating'
      this.hud.mode = 'floating'
      this.hud.sessionWindow = window.nativeId
      this.lastEvent = `"${PANELS[drag.panel].title}" popped out into window #${window.nativeId}`
    }
    const window = this.floating(drag.panel)
    if (window) {
      window.x = Math.round(cursor.x - drag.pointerInItem.x)
      window.y = Math.round(cursor.y - drag.pointerInItem.y)
      this.setHovered(this.dropTarget(cursor, window.nativeId))
    }
    this.emit()
  }

  /** Highlights the slot and makes the moving window translucent while it is over one, so the slot shows through. */
  private setHovered(slot: SlotId | null) {
    if (this.hovered === slot) return
    this.hovered = slot
    const id = this.drag?.panel ?? this.floatingPanels.find(p => this.floating(p)!.opacity !== 1)
    const window = id ? this.floating(id) : null
    if (window) {
      window.opacity = slot ? 0.6 : 1
      this.call(`setOpacity(${window.opacity}) #${window.nativeId}`)
    }
  }

  // ---------------------------------------------------------------------------
  // Hit testing

  /** The empty slot under the cursor in whichever window is frontmost there, looking through the moving one. */
  private dropTarget(cursor: Point, excluding: number): SlotId | null {
    const frames = [
      ...this.mains.map(m => ({ id: m.nativeId, z: m.z, main: m.id as MainId | null })),
      ...this.floatingPanels.map(p => this.floating(p)!).map(w => ({ id: w.nativeId, z: w.z, main: null })),
    ]
      .filter(w => w.id !== excluding)
      .sort((a, b) => b.z - a.z)
    const hit = frames.find(w => {
      const rect = this.rectOf(`[data-detach-window="${w.id}"]`)
      return rect && contains(rect, cursor)
    })
    const id = hit?.id ?? null
    if (id !== this.hud.hit) {
      this.call(`getWindowAtPoint(${xy(cursor)}, excluding #${excluding}) → ${id === null ? 'null' : `#${id}`}`)
      this.hud.hit = id
    }
    if (!hit?.main) return null
    for (const slot of SLOT_IDS) {
      if (SLOTS[slot].window !== hit.main || this.itemInSlot(slot) !== null) continue
      const rect = this.slotRect(slot)
      if (rect && contains(rect, cursor)) return slot
    }
    return null
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
    return { x: r.left - origin.left, y: r.top - origin.top, width: Math.round(r.width), height: Math.round(r.height) }
  }

  private slotRect(slot: SlotId) {
    return this.rectOf(`[data-slot="${slot}"]`)
  }

  // ---------------------------------------------------------------------------

  private call(line: string) {
    this.log = [line, ...this.log].slice(0, LOG_LIMIT)
  }

  /** The controller's own log line, as the example's `ActivityLog` stamps it. */
  private note(message: string) {
    const now = new Date()
    this.activity = [`${two(now.getHours())}:${two(now.getMinutes())}  ${message}`, ...this.activity].slice(0, 50)
    this.lastEvent = message
    this.emit()
  }

  private emit() {
    this.onChange()
  }
}
