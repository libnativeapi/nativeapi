import { useCallback, useMemo, useRef, useState } from 'react'

import type { WindowFramePlatform } from '@dazzlabs/dazzui'

import { type Os, osOf, PLATFORM_NAMES } from '../../components/platform'
import { backgroundOf, BEHAVIOUR, effectName, initialWindows, plainWindow, TITLE_BAR_HEIGHT } from './data'
import type {
  Area,
  BackgroundName,
  BehaviourFlag,
  Rect,
  ResizeEdge,
  SimWindow,
  Size,
  Tab,
  TitleBarStyle,
  View,
  VisualEffect,
} from './types'

/** A log line; lines of one gesture share a tag and replace each other. */
interface LogEntry {
  text: string
  tag?: string
}

/**
 * The prototype's stand-in for `WindowManager` and the windows it tracks:
 * every call the example makes, simulated against the desktop the stage
 * draws, and the `WindowEvent`s the platform would send back.
 */
export interface WindowsState {
  windows: SimWindow[]
  focusedId: number | null
  selectedId: number
  view: View
  tab: Tab
  nextId: number
  nextZ: number
  area: Area
  lastEvent: string
  /** Events sent so far: an action that sent one keeps its label as the last event. */
  seq: number
  log: LogEntry[]
}

export interface WindowsOptions {
  /** Changes to the starting windows, by id: a maximized one, a translucent one. */
  patch?: Record<number, Partial<SimWindow>>
  selected?: number
  view?: View
  tab?: Tab
}

const LOG_LIMIT = 200

/** A desktop until the stage has been measured. */
const UNMEASURED: Area = { width: 1320, height: 720, bar: 28, barTop: true, left: 0, top: 0 }

/** The frame a window stands in now: its own, or the work area, or the whole display. */
export function frameOf(w: SimWindow, area: Area): Rect {
  if (w.fullScreen) {
    return { x: 0, y: area.barTop ? -area.bar : 0, width: area.width, height: area.height + area.bar }
  }
  if (w.maximized) return { x: 0, y: 0, width: area.width, height: area.height }
  return w.frame
}

/** The native title bar's height while it is there, for `contentBounds`. */
export function titleBarOf(w: SimWindow, platform: WindowFramePlatform) {
  if (w.titleBarStyle === 'hidden' || w.contentUnderTitleBar || w.fullScreen) return 0
  return TITLE_BAR_HEIGHT[platform]
}

/** Screen coordinates: the display's top-left is the origin, the bar included. */
export const toScreen = (r: Rect, area: Area): Rect => ({ ...r, y: r.y + (area.barTop ? area.bar : 0) })

export const sizeText = (s: Size) => `${Math.round(s.width)}×${Math.round(s.height)}`
export const rectText = (r: Rect) => `${Math.round(r.x)}, ${Math.round(r.y)} ${sizeText(r)}`

/** `getBounds` and `getContentBounds`, in screen coordinates. */
export function boundsOf(w: SimWindow, area: Area, platform: WindowFramePlatform) {
  const bounds = toScreen(frameOf(w, area), area)
  const bar = titleBarOf(w, platform)
  return { bounds, content: { ...bounds, y: bounds.y + bar, height: bounds.height - bar } }
}

/** Where a flag's setter does nothing here, and why. */
export const unsupportedOf = (flag: BehaviourFlag, os: Os) => BEHAVIOUR.find(b => b.flag === flag)?.unsupported?.[os]

/** What `isWindowControlButtonsVisible` returns: true wherever it is not implemented. */
export const buttonsVisibleOf = (w: SimWindow, os: Os) => (os === 'macos' ? w.controlButtonsVisible : true)

function createState(platform: WindowFramePlatform, options: WindowsOptions): WindowsState {
  const os = osOf(platform)
  const windows = initialWindows(os).map(w => {
    const patched = { ...w, ...options.patch?.[w.id] }
    // Linux has no visual effects: a story asking for one gets none.
    if (os === 'linux') patched.visualEffect = 'none'
    return patched
  })
  return {
    windows,
    focusedId: 1,
    selectedId: options.selected ?? 1,
    view: options.view ?? 'window',
    tab: options.tab ?? 'state',
    nextId: 4,
    nextZ: 10,
    area: UNMEASURED,
    lastEvent: 'No events yet',
    seq: 0,
    log: [
      { text: `getAll() → [${windows.map(w => w.id).join(', ')}]` },
      { text: 'getCurrent() → #1' },
      { text: 'addListener(WindowEvent) → 1' },
    ].reverse(),
  }
}

// Mutators over a draft. Every action copies the state, applies some of
// these and hands the copy back, so the hook stays a plain `useState`.

function push(d: WindowsState, text: string, tag?: string) {
  if (tag) {
    // The newest run of tagged lines is one gesture: a line of the same
    // kind replaces its predecessor rather than piling up.
    for (let i = 0; i < d.log.length && d.log[i]!.tag; i++) {
      if (d.log[i]!.tag === tag) {
        d.log = d.log.slice()
        d.log[i] = { text, tag }
        return
      }
    }
  }
  d.log = [{ text, tag }, ...d.log].slice(0, LOG_LIMIT)
}

function call(d: WindowsState, text: string) {
  push(d, text)
}

function emit(d: WindowsState, label: string, line: string, tag?: string, quiet = false) {
  if (!quiet) {
    d.lastEvent = label
    d.seq++
  }
  push(d, line, tag)
}

const find = (d: WindowsState, id: number) => d.windows.find(w => w.id === id)

function raise(d: WindowsState, w: SimWindow) {
  w.z = ++d.nextZ
}

function blurWindow(d: WindowsState, w: SimWindow) {
  if (d.focusedId !== w.id) return
  d.focusedId = null
  emit(d, `Window #${w.id} blurred`, `WindowBlurredEvent #${w.id}`)
}

/** Activates a window: the one before it blurs. A window that cannot become key only comes forward. */
function focusWindow(d: WindowsState, w: SimWindow, os: Os) {
  if (!w.visible || w.minimized) return
  raise(d, w)
  if (os === 'macos' && !w.focusable) return
  if (d.focusedId === w.id) return
  const before = d.focusedId === null ? null : find(d, d.focusedId)
  if (before) blurWindow(d, before)
  d.focusedId = w.id
  emit(d, `Window #${w.id} focused`, `WindowFocusedEvent #${w.id}`)
}

/** The size a window may have: its own limits, then the work area. */
function constrain(w: SimWindow, size: Size, area: Area): Size {
  let { width, height } = size
  if (w.minimumSize) {
    width = Math.max(width, w.minimumSize.width)
    height = Math.max(height, w.minimumSize.height)
  }
  if (w.maximumSize) {
    width = Math.min(width, w.maximumSize.width)
    height = Math.min(height, w.maximumSize.height)
  }
  return { width: Math.round(Math.min(width, area.width)), height: Math.round(Math.min(height, area.height)) }
}

/** Keeps a window's title bar on the desktop, as every window manager does. */
function place(w: SimWindow, x: number, y: number, area: Area) {
  w.frame.x = Math.round(Math.min(Math.max(x, 80 - w.frame.width), area.width - 80))
  w.frame.y = Math.round(Math.min(Math.max(y, 0), area.height - 40))
}

/**
 * Sends `WindowMovedEvent` and `WindowResizedEvent` for every window whose
 * frame an action changed, in screen coordinates. They come with the event
 * that caused them (maximized, restored…), which keeps the last-event label.
 */
function diffFrames(prev: WindowsState, d: WindowsState) {
  const quiet = d.seq !== prev.seq
  for (const w of d.windows) {
    const before = prev.windows.find(p => p.id === w.id)
    if (!before) continue
    const a = toScreen(frameOf(before, prev.area), prev.area)
    const b = toScreen(frameOf(w, d.area), d.area)
    if (a.x !== b.x || a.y !== b.y) {
      emit(d, `Window #${w.id} moved to ${b.x}, ${b.y}`, `WindowMovedEvent #${w.id} → ${b.x}, ${b.y}`, `moved-${w.id}`, quiet)
    }
    if (a.width !== b.width || a.height !== b.height) {
      emit(
        d,
        `Window #${w.id} resized to ${sizeText(b)}`,
        `WindowResizedEvent #${w.id} → ${sizeText(b)}`,
        `resized-${w.id}`,
        quiet,
      )
    }
  }
}

const flagMeta = (flag: BehaviourFlag) => BEHAVIOUR.find(b => b.flag === flag)!

export function useWindows(platform: WindowFramePlatform, options: WindowsOptions = {}) {
  const os = osOf(platform)
  const [state, setState] = useState(() => createState(platform, options))
  const ref = useRef(state)

  /** Copies the state, lets `fn` change the copy, sends the frame events. */
  const act = useCallback((fn: (d: WindowsState) => void) => {
    const prev = ref.current
    const d: WindowsState = { ...prev, windows: prev.windows.map(w => ({ ...w, frame: { ...w.frame } })) }
    fn(d)
    diffFrames(prev, d)
    ref.current = d
    setState(d)
  }, [])

  /** Runs `fn` on one window of the draft, if it still exists. */
  const on = useCallback(
    (id: number, fn: (w: SimWindow, d: WindowsState) => void) =>
      act(d => {
        const w = find(d, id)
        if (w) fn(w, d)
      }),
    [act],
  )

  const actions = useMemo(() => {
    const restoreFrom = (d: WindowsState, w: SimWindow) => {
      if (w.minimized) {
        w.minimized = false
        emit(d, `Window #${w.id} restored`, `WindowRestoredEvent #${w.id}`)
        focusWindow(d, w, os)
        return true
      }
      if (w.maximized) {
        w.maximized = false
        emit(d, `Window #${w.id} restored`, `WindowRestoredEvent #${w.id}`)
        return true
      }
      return false
    }

    const minimizeIn = (d: WindowsState, w: SimWindow) => {
      if (w.minimized || !w.visible) return
      w.minimized = true
      blurWindow(d, w)
      emit(d, `Window #${w.id} minimized`, `WindowMinimizedEvent #${w.id}`)
    }

    const maximizeIn = (d: WindowsState, w: SimWindow) => {
      if (w.minimized) {
        w.minimized = false
        focusWindow(d, w, os)
      }
      if (w.maximized) return
      w.maximized = true
      emit(d, `Window #${w.id} maximized`, `WindowMaximizedEvent #${w.id}`)
    }

    const fullScreenIn = (d: WindowsState, w: SimWindow, on: boolean) => {
      if (w.fullScreen === on) return
      w.fullScreen = on
      if (on) {
        focusWindow(d, w, os)
        emit(d, `Window #${w.id} entered full screen`, `WindowEnteredFullScreenEvent #${w.id}`)
      } else {
        emit(d, `Window #${w.id} exited full screen`, `WindowExitedFullScreenEvent #${w.id}`)
      }
    }

    const showIn = (d: WindowsState, w: SimWindow, activate: boolean) => {
      w.visible = true
      if (activate) focusWindow(d, w, os)
      else raise(d, w)
    }

    const hideIn = (d: WindowsState, w: SimWindow) => {
      if (!w.visible) return
      w.visible = false
      blurWindow(d, w)
    }

    /** Programmatic size: the window leaves maximized and full screen for its own frame. */
    const resizeIn = (d: WindowsState, w: SimWindow, size: Size) => {
      w.maximized = false
      w.fullScreen = false
      const s = constrain(w, size, d.area)
      w.frame.width = s.width
      w.frame.height = s.height
      return s
    }

    const fits = (asked: Size, got: Size) =>
      asked.width === got.width && asked.height === got.height ? '' : ` → ${sizeText(got)}, limited by the work area or the size limits`

    return {
      setArea: (area: Area) => {
        const next = { ...ref.current, area }
        ref.current = next
        setState(next)
      },
      select: (id: number) =>
        act(d => {
          d.selectedId = id
          d.view = 'window'
        }),
      /** Selects a window and stays on the map. */
      pick: (id: number) =>
        act(d => {
          d.selectedId = id
        }),
      setView: (view: View) =>
        act(d => {
          d.view = view
        }),
      setTab: (tab: Tab) =>
        act(d => {
          d.tab = tab
        }),
      /** A press on a window: the window manager activates it and brings it forward. */
      press: (id: number) =>
        on(id, (w, d) => {
          focusWindow(d, w, os)
          if (w.kind === 'plain') d.selectedId = id
        }),
      /** A press on the bare desktop: another app is active, ours blur. */
      desktopPress: () =>
        act(d => {
          const w = d.focusedId === null ? null : find(d, d.focusedId)
          if (w) blurWindow(d, w)
        }),

      // State
      show: (id: number) => on(id, (w, d) => (call(d, `show() #${id}`), showIn(d, w, true))),
      showInactive: (id: number) => on(id, (w, d) => (call(d, `showInactive() #${id}`), showIn(d, w, false))),
      hide: (id: number) => on(id, (w, d) => (call(d, `hide() #${id}`), hideIn(d, w))),
      maximize: (id: number) =>
        on(id, (w, d) => {
          if (!w.maximizable) return call(d, `maximize() #${id} · ignored, isMaximizable → false`)
          call(d, `maximize() #${id}`)
          maximizeIn(d, w)
        }),
      unmaximize: (id: number) =>
        on(id, (w, d) => {
          call(d, `unmaximize() #${id}`)
          if (w.maximized) {
            w.maximized = false
            emit(d, `Window #${id} restored`, `WindowRestoredEvent #${id}`)
          }
        }),
      minimize: (id: number) =>
        on(id, (w, d) => {
          if (!w.minimizable) return call(d, `minimize() #${id} · ignored, isMinimizable → false`)
          call(d, `minimize() #${id}`)
          minimizeIn(d, w)
        }),
      restore: (id: number) => on(id, (w, d) => (call(d, `restore() #${id}`), restoreFrom(d, w))),
      setFullScreen: (id: number, value: boolean) =>
        on(id, (w, d) => {
          if (value && !w.fullScreenable) return call(d, `setFullScreen(true) #${id} · ignored, isFullScreenable → false`)
          call(d, `setFullScreen(${value}) #${id}`)
          fullScreenIn(d, w, value)
        }),
      focus: (id: number) => on(id, (w, d) => (call(d, `focus() #${id}`), focusWindow(d, w, os))),
      blur: (id: number) => on(id, (w, d) => (call(d, `blur() #${id}`), blurWindow(d, w))),

      /** The window's own controls and the dock: user actions, events without a call. */
      userMinimize: (id: number) => on(id, (w, d) => minimizeIn(d, w)),
      userToggleMaximize: (id: number) =>
        on(id, (w, d) => {
          if (!w.maximizable) return
          if (w.maximized) restoreFrom(d, w)
          else maximizeIn(d, w)
        }),
      /** A taskbar button: restores a minimized window, minimizes the active one, else activates it. */
      userTaskbar: (id: number) =>
        on(id, (w, d) => {
          if (w.minimized) restoreFrom(d, w)
          else if (d.focusedId === id && w.minimizable) minimizeIn(d, w)
          else focusWindow(d, w, os)
        }),

      // Geometry
      setSize: (id: number, size: Size) =>
        on(id, (w, d) => {
          const got = resizeIn(d, w, size)
          call(d, `setSize(${sizeText(size)}, animate: false) #${id}${fits(size, got)}`)
        }),
      setContentSize: (id: number, size: Size) =>
        on(id, (w, d) => {
          const bar = titleBarOf({ ...w, fullScreen: false }, platform)
          const got = resizeIn(d, w, { width: size.width, height: size.height + bar })
          call(d, `setContentSize(${sizeText(size)}) #${id}${fits({ ...size, height: size.height + bar }, got)}`)
        }),
      setPosition: (id: number, x: number, y: number) =>
        on(id, (w, d) => {
          call(d, `setPosition(${x}, ${y}) #${id}`)
          w.maximized = false
          w.fullScreen = false
          place(w, x, y - (d.area.barTop ? d.area.bar : 0), d.area)
        }),
      center: (id: number) =>
        on(id, (w, d) => {
          call(d, `center() #${id}`)
          w.maximized = false
          w.fullScreen = false
          place(w, (d.area.width - w.frame.width) / 2, (d.area.height - w.frame.height) / 2, d.area)
        }),
      setMinimumSize: (id: number, size: Size | null) =>
        on(id, (w, d) => {
          call(d, `setMinimumSize(${size ? sizeText(size) : '0×0'}) #${id}`)
          w.minimumSize = size
          if (!w.maximized && !w.fullScreen) Object.assign(w.frame, constrain(w, w.frame, d.area))
        }),
      setMaximumSize: (id: number, size: Size | null) =>
        on(id, (w, d) => {
          call(d, `setMaximumSize(${size ? sizeText(size) : '0×0'}) #${id}`)
          w.maximumSize = size
          if (!w.maximized && !w.fullScreen) Object.assign(w.frame, constrain(w, w.frame, d.area))
        }),
      /** A move the window manager runs: by the title bar, or after `startDragging`. */
      moveTo: (id: number, x: number, y: number) => on(id, (w, d) => place(w, x, y, d.area)),
      /** A resize the window manager runs from `edge`, against the frame the gesture started from. */
      resizeFrom: (id: number, start: Rect, edge: ResizeEdge, dx: number, dy: number) =>
        on(id, (w, d) => {
          const west = edge === 'left' || edge === 'topLeft' || edge === 'bottomLeft'
          const north = edge === 'top' || edge === 'topLeft' || edge === 'topRight'
          const east = edge === 'right' || edge === 'topRight' || edge === 'bottomRight'
          const south = edge === 'bottom' || edge === 'bottomLeft' || edge === 'bottomRight'
          const s = constrain(
            w,
            {
              width: start.width + (east ? dx : west ? -dx : 0),
              height: start.height + (south ? dy : north ? -dy : 0),
            },
            d.area,
          )
          w.frame.width = s.width
          w.frame.height = s.height
          w.frame.x = west ? start.x + start.width - s.width : start.x
          w.frame.y = north ? start.y + start.height - s.height : start.y
        }),
      /** Dragging a maximized window by its title bar: it restores under the pointer. */
      unmaximizeUnder: (id: number, clientX: number) =>
        on(id, (w, d) => {
          w.maximized = false
          emit(d, `Window #${id} restored`, `WindowRestoredEvent #${id}`)
          place(w, clientX - d.area.left - w.frame.width / 2, 0, d.area)
        }),
      startDragging: (id: number) => on(id, (_w, d) => call(d, `startDragging() #${id}`)),
      startResizing: (id: number, edge: ResizeEdge) =>
        on(id, (_w, d) => call(d, `startResizing(ResizeEdge.${edge}) #${id}`)),

      // Appearance
      setTitle: (id: number, title: string) =>
        on(id, (w, d) => {
          call(d, `setTitle("${title}") #${id}`)
          w.title = title
        }),
      setTitleBarStyle: (id: number, style: TitleBarStyle) =>
        on(id, (w, d) => {
          call(d, `setTitleBarStyle(TitleBarStyle.${style}) #${id}`)
          w.titleBarStyle = style
          // A style puts the buttons back to what it implies.
          w.controlButtonsVisible = style === 'normal'
        }),
      setContentUnderTitleBar: (id: number, value: boolean) =>
        on(id, (w, d) => {
          if (os !== 'macos') {
            return call(d, `setContentUnderTitleBar(${value}) #${id} → false · not on ${PLATFORM_NAMES[platform]}`)
          }
          call(d, `setContentUnderTitleBar(${value}) #${id} → true`)
          w.contentUnderTitleBar = value
        }),
      setTitleBarColors: (id: number, on_: boolean) =>
        on(id, (w, d) => {
          const name = on_ ? 'setTitleBarColors(#3F51B5FF, #FFFFFFFF)' : 'resetTitleBarColors()'
          if (os !== 'windows') return call(d, `${name} #${id} → false · WinUI 3 only`)
          call(d, `${name} #${id} → true`)
          w.titleBarColors = on_
        }),
      setHasShadow: (id: number, value: boolean) =>
        on(id, (w, d) => {
          call(d, `setHasShadow(${value}) #${id}`)
          w.hasShadow = value
        }),
      /** The slider moves the window live and logs once, on release. */
      setOpacity: (id: number, value: number, final: boolean) =>
        on(id, (w, d) => {
          w.opacity = value
          if (final) call(d, `setOpacity(${value.toFixed(2)}) #${id}`)
        }),
      setVisualEffect: (id: number, effect: VisualEffect) =>
        on(id, (w, d) => {
          const name = `setVisualEffect(VisualEffect.${effect}) #${id}`
          if (os === 'linux' && effect !== 'none') {
            return emit(d, `Visual effect ${effectName(effect)} is not available here`, `${name} → false · ${PLATFORM_NAMES[platform]} has none`)
          }
          call(d, `${name} → true`)
          w.visualEffect = effect
        }),
      setBackgroundColor: (id: number, background: BackgroundName) =>
        on(id, (w, d) => {
          call(d, `setBackgroundColor(${backgroundOf(background).rgba}) #${id}`)
          w.background = background
        }),

      // Behaviour
      setFlag: (id: number, flag: BehaviourFlag, value: boolean) =>
        on(id, (w, d) => {
          const meta = flagMeta(flag)
          const unsupported = unsupportedOf(flag, os)
          if (unsupported) return call(d, `${meta.setter}(${value}) #${id} · not implemented on ${PLATFORM_NAMES[platform]}`)
          call(d, `${meta.setter}(${value}) #${id}`)
          w[flag] = value
          // The two stacking levels exclude each other.
          if (flag === 'alwaysOnTop' && value) w.alwaysOnBottom = false
          if (flag === 'alwaysOnBottom' && value) w.alwaysOnTop = false
          if (flag === 'focusable' && !value) blurWindow(d, w)
        }),

      // WindowManager
      forAll: (what: 'minimize' | 'restore' | 'show' | 'hide') =>
        act(d => {
          call(d, `getAll() → ${d.windows.length} windows · ${what}() each`)
          for (const w of d.windows) {
            if (what === 'minimize' && w.minimizable) minimizeIn(d, w)
            if (what === 'restore') restoreFrom(d, w)
            if (what === 'show') showIn(d, w, false)
            if (what === 'hide') hideIn(d, w)
          }
        }),
      create: () =>
        act(d => {
          const id = d.nextId++
          const offset = ((id - 4) % 5) * 28
          const w = plainWindow(id, 420 + offset, 120 + offset)
          d.windows = [...d.windows, w]
          call(d, `Window.create() → #${id}`)
          call(d, `show() #${id}`)
          emit(d, `Window #${id} created`, `WindowCreatedEvent #${id} (first shown)`)
          focusWindow(d, w, os)
          d.selectedId = id
          d.view = 'window'
        }),
      close: (id: number) =>
        act(d => {
          const w = find(d, id)
          if (!w || !w.closable || w.kind === 'example') return
          blurWindow(d, w)
          d.windows = d.windows.filter(x => x.id !== id)
          emit(d, `Window #${id} closed`, `WindowClosedEvent #${id}`)
          if (d.selectedId === id) d.selectedId = 1
        }),
      clearLog: () =>
        act(d => {
          d.log = []
          d.lastEvent = 'No events yet'
        }),
    }
  }, [act, on, os, platform])

  const selected = state.windows.find(w => w.id === state.selectedId) ?? state.windows[0]!
  return { state, selected, actions }
}

export type WindowActions = ReturnType<typeof useWindows>['actions']
