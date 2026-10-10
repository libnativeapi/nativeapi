import { useCallback, useEffect, useMemo, useRef, useState } from 'react'

import { animationLabel, checklistOf, DEFAULT_TOOLTIP, ICON_POINTS, SCENES } from './data'
import { progressAt } from './icon-paint'
import { createSign, headingOf, templateOf } from './sign-data'
import { signLayoutOf } from './sign-layout'
import type { SignContent, SignStyle } from './sign-types'
import type {
  Capabilities,
  CheckItem,
  CheckStatus,
  IconAnimation,
  IconColor,
  MenuBackend,
  Scene,
  StillIcon,
  TrayEntry,
  Trigger,
  WindowPlacement,
} from './types'

/**
 * The prototype's stand-in for the example's `TrayController`: every
 * `TrayIcon` / `TrayManager` call it would make, simulated, with the log line
 * and the checklist tick the real one produces.
 */
export interface TrayState {
  entries: TrayEntry[]
  selected: number | null
  nextNumber: number
  nextId: number
  /** Popup mode: a click on the icon shows the window, losing focus hides it. */
  popupMode: boolean
  windowVisible: boolean
  placement: WindowPlacement
  /** The icon whose context menu is open. */
  menuOpenFor: number | null
  previewSigns: number[]
  backend: MenuBackend
  /** The menu's checkbox item. */
  notifications: boolean
  lastEvent: string
  /** Newest first. */
  log: string[]
  checklist: CheckItem[]
  /** Partial progress of the multi-part checklist items. */
  parts: Record<string, string[]>
}

/** What an icon created on start shows. */
export type InitialIcon = Partial<
  Pick<TrayEntry, 'animation' | 'still' | 'scene' | 'title' | 'tooltip' | 'visible' | 'trigger' | 'color'>
> & { signStyle?: SignStyle }

export interface TrayOptions {
  /** Icons created on start; one plain icon when left out. */
  initial?: InitialIcon[]
  popupMode?: boolean
}

export const nowSeconds = () => performance.now() / 1000

/** The animation time an icon is at, quantised to its frame rate. */
export function timeOf(entry: TrayEntry, now = nowSeconds()) {
  const t = entry.paused ? entry.pausedAt : now - entry.startedAt
  return Math.floor(t * entry.fps) / entry.fps
}

/** The frame count an icon has handed to the tray. */
export const framesOf = (entry: TrayEntry, now = nowSeconds()) =>
  entry.animation ? Math.floor((entry.paused ? entry.pausedAt : now - entry.startedAt) * entry.fps) : 0

/** What `getTitle` returns: a scene writes its title every frame. */
export function titleOf(entry: TrayEntry, now = nowSeconds()): string | null {
  if (entry.contentMode === 'sign') return null
  if (entry.scene === 'download') return `${Math.round(progressAt(timeOf(entry, now)) * 100)}%`
  if (entry.scene === 'recording') {
    const s = Math.floor(timeOf(entry, now))
    return `${String(Math.floor(s / 60)).padStart(2, '0')}:${String(s % 60).padStart(2, '0')}`
  }
  return entry.title
}

/** What `getBounds` returns: where the icon sits in the bar, or an empty rectangle once hidden. */
export function boundsOf(entry: TrayEntry, entries: TrayEntry[], caps: Capabilities) {
  const visible = entries.filter(e => e.visible)
  const index = visible.indexOf(entry)
  if (index < 0) return '0,0 0×0'
  if (caps.os === 'windows') return `${1630 - index * 32},1032 32×40`
  const widthOf = (item: TrayEntry) => item.contentMode === 'sign' ? signLayoutOf(item.sign).width * 0.24 + 8 : 30
  const offset = visible.slice(0, index).reduce((total, item) => total + widthOf(item), 0)
  const round = (value: number) => Math.round(value * 100) / 100
  return `${round(1180 - offset)},0 ${round(widthOf(entry))}×24`
}

/** The pixel size of an icon's frames. */
export const pixelSizeOf = (entry: TrayEntry) => ICON_POINTS * entry.scale

const quote = (s: string | null) => (s === null ? 'null' : `"${s.replaceAll('\n', '\\n')}"`)

const LOG_LIMIT = 60

function createState(caps: Capabilities, options: TrayOptions): TrayState {
  const state: TrayState = {
    entries: [],
    selected: null,
    nextNumber: 1,
    nextId: 1,
    popupMode: options.popupMode ?? false,
    windowVisible: !options.popupMode,
    placement: 'center',
    menuOpenFor: null,
    previewSigns: [],
    backend: 'native',
    notifications: true,
    lastEvent: 'No events yet',
    log: [],
    checklist: checklistOf(caps),
    parts: {},
  }
  pass(state, 'supported', 'true')
  for (const initial of options.initial ?? [{}]) {
    addEntry(state, initial)
  }
  return state
}

// Mutators over a draft. Every action copies the state, applies one of these
// and hands the copy back, so the hook stays a plain `useState`.

function log(d: TrayState, message: string) {
  d.log = [message, ...d.log].slice(0, LOG_LIMIT)
}

function setCheck(d: TrayState, id: string, status: CheckStatus, detail: string) {
  const item = d.checklist.find(i => i.id === id)
  if (!item) return
  item.status = status
  item.detail = detail
}

function pass(d: TrayState, id: string, detail = '') {
  setCheck(d, id, 'pass', detail)
}

/** Records `part` of a multi-part item; passes once `total` are in. */
function part(d: TrayState, id: string, name: string, total: number) {
  const parts = (d.parts[id] = [...new Set([...(d.parts[id] ?? []), name])])
  const item = d.checklist.find(i => i.id === id)
  if (!item || item.status === 'pass') return
  if (parts.length >= total) pass(d, id, `${total}/${total}`)
  else item.detail = `${parts.length}/${total}`
}

/** Counts an occurrence of an event item. */
function count(d: TrayState, id: string) {
  const n = (d.parts[id]?.length ?? 0) + 1
  d.parts[id] = Array.from({ length: n }, (_, i) => String(i))
  pass(d, id, `×${n}`)
}

function event(d: TrayState, what: string, entry: TrayEntry) {
  d.lastEvent = `${what} · #${entry.number}`
  log(d, `${what.toLowerCase()} #${entry.number}`)
}

function addEntry(d: TrayState, initial: InitialIcon) {
  const scene = initial.scene ?? null
  const sceneData = SCENES.find(s => s.value === scene)
  const animation = sceneData?.animation ?? initial.animation ?? null
  const entry: TrayEntry = {
    number: d.nextNumber++,
    id: d.nextId++,
    contentMode: initial.signStyle ? 'sign' : 'icon',
    sign: createSign(d.nextNumber - 1, initial.signStyle),
    animation,
    still: animation ? null : (initial.still ?? 'asset'),
    scene,
    title: scene === 'syncing' ? 'Syncing…' : (initial.title ?? null),
    tooltip: sceneData?.tooltip ?? (initial.tooltip === undefined ? DEFAULT_TOOLTIP : initial.tooltip),
    visible: initial.visible ?? true,
    trigger: initial.trigger ?? 'rightClicked',
    fps: 30,
    scale: 2,
    color: initial.color ?? 'auto',
    paused: false,
    startedAt: nowSeconds(),
    pausedAt: 0,
    clicks: 0,
    rightClicks: 0,
    doubleClicks: 0,
  }
  if (initial.signStyle) {
    entry.animation = null
    entry.scene = null
    entry.title = null
    entry.tooltip = headingOf(entry.sign)
  }
  d.entries.push(entry)
  d.selected = entry.number
  log(d, `create #${entry.number} → id ${entry.id}`)
  pass(d, 'create', `id ${entry.id}`)
  pass(d, 'managed', `getAll ${d.entries.length}`)
  return entry
}

function startAnimation(entry: TrayEntry, animation: IconAnimation | null) {
  entry.contentMode = 'icon'
  entry.animation = animation
  entry.still = animation ? null : 'asset'
  entry.paused = false
  entry.pausedAt = 0
  entry.startedAt = nowSeconds()
}

/** Gives back the title and tooltip a scene took. */
function leaveScene(entry: TrayEntry) {
  if (!entry.scene) return
  entry.scene = null
  entry.title = null
  entry.tooltip = DEFAULT_TOOLTIP
}

export function useTray(caps: Capabilities, options: TrayOptions = {}) {
  const [state, setState] = useState(() => createState(caps, options))
  const timers = useRef<number[]>([])
  // The state the last render saw, for actions that schedule work after it.
  const latest = useRef(state)
  latest.current = state

  useEffect(() => () => timers.current.forEach(clearTimeout), [])

  const later = useCallback((ms: number, fn: () => void) => {
    timers.current.push(window.setTimeout(fn, ms))
  }, [])

  /** Applies `fn` to a copy of the state; `number` picks the icon, the selected one by default. */
  const update = useCallback(
    (fn: (d: TrayState, entry: TrayEntry | undefined) => void, number?: number) =>
      setState(prev => {
        const d = structuredClone(prev)
        fn(d, d.entries.find(e => e.number === (number ?? d.selected)))
        return d
      }),
    [],
  )

  /** Ticks "100 frames at the target rate" once an animation has played that long. */
  const watchFrames = useCallback(
    (number: number, fps: number) =>
      later((100 / fps) * 1000 + 50, () =>
        update((d, entry) => {
          if (entry?.animation && !entry.paused) pass(d, 'frames', `${entry.fps} fps`)
        }, number),
      ),
    [later, update],
  )

  const actions = useMemo(() => {
    const onSelected = (fn: (d: TrayState, entry: TrayEntry) => void, number?: number) =>
      update((d, entry) => entry && fn(d, entry), number)

    const openMenu = (d: TrayState, entry: TrayEntry) => {
      if (d.menuOpenFor === entry.number) return
      d.menuOpenFor = entry.number
      d.lastEvent = `Menu opened · #${entry.number}`
      log(d, `menu opened #${entry.number}`)
      part(d, 'menuOpenClose', 'open', 2)
    }

    const closeMenu = (d: TrayState) => {
      if (d.menuOpenFor === null) return
      log(d, `menu closed #${d.menuOpenFor}`)
      d.lastEvent = `Menu closed · #${d.menuOpenFor}`
      d.menuOpenFor = null
      part(d, 'menuOpenClose', 'close', 2)
    }

    return {
      select: (number: number) => update(d => void (d.selected = number)),

      addIcon: () => update(d => void addEntry(d, {})),

      addScene: (scene: Scene) => update(d => void addEntry(d, { scene })),

      addThreeIcons: () => update(d => {
        for (const animation of ['spinner', 'pulse', 'wave'] as const) addEntry(d, { animation })
      }),

      addSign: (style: SignStyle = 'missing') => update(d => {
        if (!caps.contentView) return
        addEntry(d, { signStyle: style })
        d.windowVisible = true
      }),

      setSignStyle: (style: SignStyle) => onSelected((d, entry) => {
        entry.sign.style = style
        entry.sign.english = entry.sign.right = true
        if (caps.contentView) {
          leaveScene(entry)
          startAnimation(entry, null)
          entry.contentMode = 'sign'
          entry.title = null
          entry.tooltip = headingOf(entry.sign)
        }
        log(d, `contentView #${entry.number} ← ${templateOf(style).label}`)
      }),

      setSignContent: (content: SignContent) => onSelected((d, entry) => {
        entry.sign.contents[entry.sign.style] = content
        if (caps.contentView && entry.contentMode !== 'sign') {
          leaveScene(entry)
          startAnimation(entry, null)
          entry.contentMode = 'sign'
          entry.title = null
        }
        if (entry.contentMode === 'sign') entry.tooltip = headingOf(entry.sign)
        log(d, `sign #${entry.number} ← ${headingOf(entry.sign)}`)
      }),

      restoreSign: () => onSelected((d, entry) => {
        const sign = entry.sign
        sign.contents[sign.style] = { ...templateOf(sign.style).defaults }
        sign.english = sign.right = true
        if (sign.style === 'missing') sign.green = false
        if (entry.contentMode === 'sign') entry.tooltip = headingOf(sign)
        log(d, `restore sign #${entry.number}`)
      }),

      setSignOption: (option: 'green' | 'english' | 'right', value: boolean) => onSelected((d, entry) => {
        entry.sign[option] = value
        log(d, `sign #${entry.number} ${option} ← ${value}`)
      }),

      previewSign: (number?: number) => onSelected((d, entry) => {
        if (!d.previewSigns.includes(entry.number)) d.previewSigns.push(entry.number)
        log(d, `preview sign #${entry.number}`)
      }, number),

      closeSignPreview: (number: number) => update(d => {
        d.previewSigns = d.previewSigns.filter(preview => preview !== number)
      }),

      removeIcon: (number?: number) =>
        onSelected((d, entry) => {
          if (d.menuOpenFor === entry.number) d.menuOpenFor = null
          d.previewSigns = d.previewSigns.filter(number => number !== entry.number)
          d.entries = d.entries.filter(e => e !== entry)
          d.selected = d.entries.at(-1)?.number ?? null
          log(d, `dispose #${entry.number}`)
        }, number),

      play: (animation: IconAnimation | null, number?: number) => {
        onSelected((d, entry) => {
          if (entry.contentMode !== 'icon') return
          leaveScene(entry)
          startAnimation(entry, animation)
          log(d, animation ? `play #${entry.number} ← ${animationLabel(animation)}` : `stop #${entry.number}`)
        }, number)
        const target = latest.current.entries.find(e => e.number === (number ?? latest.current.selected))
        if (animation && target) watchFrames(target.number, target.fps)
      },

      setStill: (still: StillIcon) =>
        onSelected((d, entry) => {
          if (entry.contentMode !== 'icon') return
          leaveScene(entry)
          startAnimation(entry, null)
          entry.still = still
          log(d, `icon #${entry.number} ← ${still}`)
        }),

      setFps: (fps: number) =>
        onSelected((d, entry) => {
          const t = timeOf(entry)
          entry.fps = fps
          entry.startedAt = nowSeconds() - t
          log(d, `rate #${entry.number} ← ${fps} fps`)
        }),

      setScale: (scale: number) =>
        onSelected((d, entry) => {
          entry.scale = scale
          log(d, `resolution #${entry.number} ← ${ICON_POINTS * scale} px`)
        }),

      setColor: (color: IconColor) =>
        onSelected((d, entry) => {
          entry.color = color
          log(d, `colour #${entry.number} ← ${color}${caps.os === 'macos' && color === 'auto' ? ' (template)' : ''}`)
        }),

      togglePaused: () =>
        onSelected((d, entry) => {
          if (entry.paused) {
            entry.startedAt = nowSeconds() - entry.pausedAt
            entry.paused = false
          } else {
            entry.pausedAt = nowSeconds() - entry.startedAt
            entry.paused = true
          }
          log(d, `${entry.paused ? 'pause' : 'resume'} #${entry.number}`)
        }),

      step: () =>
        onSelected((d, entry) => {
          if (!entry.paused) entry.pausedAt = nowSeconds() - entry.startedAt
          entry.paused = true
          entry.pausedAt += 1 / entry.fps
          log(d, `step #${entry.number} → frame ${framesOf(entry)}`)
        }),

      playScene: (scene: Scene) =>
        onSelected((d, entry) => {
          if (entry.contentMode !== 'icon') return
          const data = SCENES.find(s => s.value === scene)!
          leaveScene(entry)
          startAnimation(entry, data.animation)
          entry.scene = scene
          entry.tooltip = data.tooltip
          entry.title = scene === 'syncing' ? 'Syncing…' : null
          log(d, `scene #${entry.number} ← ${scene}`)
        }),

      resetScene: () =>
        onSelected((d, entry) => {
          if (entry.contentMode !== 'icon') return
          leaveScene(entry)
          startAnimation(entry, null)
          log(d, `scene #${entry.number} ← default`)
        }),

      playThreeAtOnce: () =>
        update(d => {
          const icons = d.entries.filter(entry => entry.contentMode === 'icon')
          while (icons.length < 3) icons.push(addEntry(d, {}))
          const three: IconAnimation[] = ['spinner', 'pulse', 'wave']
          icons.slice(0, 3).forEach((entry, i) => {
            leaveScene(entry)
            startAnimation(entry, three[i]!)
            entry.visible = true
          })
          d.selected = icons[0]!.number
          log(d, 'scene ← three icons')
        }),

      setTitle: (title: string | null) =>
        onSelected((d, entry) => {
          if (!caps.title || entry.contentMode === 'sign') return
          leaveScene(entry)
          entry.title = title
          log(d, `setTitle(${quote(title)}) #${entry.number}`)
          pass(d, 'readBack', `title ${quote(title)}`)
        }),

      setTooltip: (tooltip: string | null) =>
        onSelected((d, entry) => {
          entry.tooltip = tooltip
          log(d, `setTooltip(${quote(tooltip)}) #${entry.number}`)
          if (!caps.title) pass(d, 'readBack', `tooltip ${quote(tooltip)}`)
        }),

      setVisible: (visible: boolean) =>
        onSelected((d, entry) => {
          entry.visible = visible
          if (!visible && d.menuOpenFor === entry.number) closeMenu(d)
          log(d, `setVisible(${visible}) #${entry.number} → true`)
          part(d, 'visible', String(visible), 2)
        }),

      setTrigger: (trigger: Trigger) =>
        onSelected((d, entry) => {
          entry.trigger = trigger
          log(d, `setContextMenuTrigger(${trigger}) #${entry.number}`)
        }),

      setPopupMode: (on: boolean) =>
        update(d => {
          d.popupMode = on
          d.windowVisible = !on
          d.placement = on ? 'icon' : 'center'
          log(d, `popup mode ← ${on ? 'on' : 'off'}`)
        }),

      setBackend: (backend: MenuBackend) =>
        update(d => {
          d.backend = backend
          log(d, `menu backend ← ${backend}`)
        }),

      /** `openContextMenu()`, and `closeContextMenu()` after `closeAfter` ms. */
      openMenu: (closeAfter?: number) => {
        onSelected((d, entry) => {
          if (!caps.openMenu || !entry.visible) return
          log(d, `openContextMenu() #${entry.number} → true`)
          pass(d, 'openMenu', 'true')
          if (entry.trigger === 'none') part(d, 'triggers', 'none', caps.os === 'linux' ? 3 : 4)
          openMenu(d, entry)
        })
        if (closeAfter)
          later(closeAfter, () =>
            update(d => {
              if (d.menuOpenFor === null) return
              log(d, `closeContextMenu() #${d.menuOpenFor} → true`)
              closeMenu(d)
              pass(d, 'closeMenu', 'closed')
            }),
          )
      },

      closeMenu: () => update(d => closeMenu(d)),

      moveWindowToIcon: () =>
        onSelected((d, entry) => {
          if (!caps.bounds) return
          const bounds = boundsOf(entry, d.entries, caps)
          d.placement = 'icon'
          d.windowVisible = true
          log(d, `window to icon #${entry.number}: getBounds ${bounds}`)
          pass(d, 'bounds', bounds)
        }),

      refresh: () => update(d => log(d, 'refresh')),

      resetCounters: () =>
        update(d => {
          for (const entry of d.entries) entry.clicks = entry.rightClicks = entry.doubleClicks = 0
          log(d, 'reset counts')
        }),

      /** A click on the icon in the tray, as the platform reports it. */
      trayClick: (number: number, kind: Exclude<Trigger, 'none'>) =>
        onSelected((d, entry) => {
          if (kind === 'clicked') entry.clicks++
          if (kind === 'rightClicked') entry.rightClicks++
          if (kind === 'doubleClicked') entry.doubleClicks++
          event(d, { clicked: 'Clicked', rightClicked: 'Right clicked', doubleClicked: 'Double clicked' }[kind], entry)
          count(d, kind)
          if (entry.contentMode === 'sign' && kind === 'clicked') {
            d.selected = entry.number
            d.windowVisible = true
          }
          if (entry.trigger === kind) {
            openMenu(d, entry)
            part(d, 'triggers', kind, caps.os === 'linux' ? 3 : 4)
          } else if (d.popupMode && kind === 'clicked' && entry.contentMode !== 'sign') {
            d.windowVisible = !d.windowVisible
            d.placement = 'icon'
            d.selected = entry.number
            log(d, `popup ${d.windowVisible ? 'shown' : 'hidden'}`)
          }
        }, number),

      /** A press anywhere else on the desktop: the menu closes, a popup loses focus. */
      desktopPress: () =>
        update(d => {
          closeMenu(d)
          if (d.popupMode && d.windowVisible) {
            d.windowVisible = false
            log(d, 'popup hidden (lost focus)')
          }
        }),

      menuItem: (label: string) =>
        update(d => {
          const number = d.menuOpenFor
          closeMenu(d)
          d.lastEvent = `${label} · menu`
          log(d, `menu item "${label}"`)
          part(d, 'menuItems', 'item', 3)
          if (label === 'Show window') {
            d.windowVisible = true
            if (number !== null) d.selected = number
          }
          if (label === 'Quit') {
            for (const entry of d.entries) log(d, `dispose #${entry.number}`)
            d.entries = []
            d.selected = null
            d.previewSigns = []
          }
        }),

      menuCheckbox: () =>
        update(d => {
          d.notifications = !d.notifications
          closeMenu(d)
          d.lastEvent = `Notifications ${d.notifications ? 'on' : 'off'} · menu`
          log(d, `menu checkbox "Notifications" → ${d.notifications}`)
          part(d, 'menuItems', 'checkbox', 3)
        }),

      menuSubmenuOpened: () =>
        update(d => {
          log(d, 'submenu opened "Animate"')
          part(d, 'menuItems', 'submenu', 3)
        }),

      menuAnimation: (animation: IconAnimation | null) =>
        update(d => {
          const entry = d.entries.find(e => e.number === d.menuOpenFor)
          closeMenu(d)
          if (!entry || entry.contentMode !== 'icon') return
          leaveScene(entry)
          startAnimation(entry, animation)
          d.lastEvent = `${animation ? animationLabel(animation) : 'Stop'} · menu`
          log(d, `menu → ${animation ?? 'stop'} #${entry.number}`)
          part(d, 'menuItems', 'item', 3)
        }),

      hideWindow: () =>
        update(d => {
          d.windowVisible = false
          log(d, 'window closed')
        }),

      mark: (id: string, status: CheckStatus) =>
        update(d => {
          const item = d.checklist.find(i => i.id === id)
          if (item) setCheck(d, id, item.status === status ? 'open' : status, '')
        }),

      resetChecklist: () =>
        update(d => {
          d.checklist = checklistOf(caps)
          d.parts = {}
          pass(d, 'supported', 'true')
          log(d, 'reset checklist')
        }),

      clearLog: () =>
        update(d => {
          d.log = []
          d.lastEvent = 'No events yet'
        }),
    }
  }, [caps, later, update, watchFrames])

  const selected = state.entries.find(e => e.number === state.selected) ?? null
  return { state, selected, actions }
}

export type TrayActions = ReturnType<typeof useTray>['actions']

/** Plain-text report for pasting into an issue or a PR. */
export function checklistReport(checklist: CheckItem[], caps: Capabilities) {
  const passed = checklist.filter(i => i.status === 'pass').length
  const failed = checklist.filter(i => i.status === 'fail').length
  const lines = [
    'tray_icon_example checklist',
    caps.platform,
    `${passed} pass · ${failed} fail · ${checklist.length - passed - failed} open`,
    '',
    ...checklist.map(
      i =>
        `[${i.status === 'pass' ? 'PASS' : i.status === 'fail' ? 'FAIL' : 'open'}] ${i.label}  <${i.manual ? 'manual' : 'auto'}>${i.detail ? `  (${i.detail})` : ''}`,
    ),
  ]
  return lines.join('\n')
}
