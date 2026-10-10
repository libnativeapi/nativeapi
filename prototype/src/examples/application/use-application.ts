import { useCallback, useEffect, useRef, useState } from 'react'

import { useEventLog } from '../../components/event-bar'
import type { Os } from '../../components/platform'
import { BRIGHTNESS_NAMES, eventName, ICON_PRESETS, MAIN_WINDOW_TITLE } from './data'
import type { AppEventType, AppIconPreset, AppWindow, Brightness, EventTally, Progress } from './types'

export interface ApplicationOptions {
  badge?: string
  progress?: Progress
  brightness?: Brightness
  /** Quit with this exit code as soon as the example has started. */
  quitAtStart?: number
}

export interface ApplicationState {
  running: boolean
  /** Between `quit()` and `ApplicationExitingEvent`. */
  quitting: boolean
  /** What the last `run()` returned; null while it has not. */
  exitCode: number | null
  /** The app is the active one: its window is key, its menus are in the bar. */
  active: boolean
  /** `hide()` took every window away (macOS). */
  hidden: boolean
  /** The badge the desktop shows, as it accepted it. */
  badge: string
  progress: Progress
  dockIconVisible: boolean
  icon: AppIconPreset
  brightness: Brightness
  menuBar: boolean
  primaryId: number | null
  windows: AppWindow[]
  /** The window in front, or null for the main one. */
  front: number | null
  tally: Record<AppEventType, EventTally>
  /** What the last call of each setter returned, for the read-back. */
  returns: Record<string, boolean>
}

const MAIN_ID = 1

const emptyTally = (): Record<AppEventType, EventTally> => ({
  started: { count: 0, at: 0 },
  activated: { count: 0, at: 0 },
  deactivated: { count: 0, at: 0 },
  quitRequested: { count: 0, at: 0 },
  exiting: { count: 0, at: 0 },
})

const launched = (os: Os, options: ApplicationOptions): ApplicationState => ({
  running: true,
  quitting: false,
  exitCode: null,
  active: true,
  hidden: false,
  badge: options.badge ?? '',
  progress: options.progress ?? -1,
  dockIconVisible: true,
  icon: 'default',
  brightness: options.brightness ?? 'system',
  // The example sets the primary window first, so Windows has a window to
  // attach the menu bar to; GTK 3's legacy menus cannot be an app menu bar.
  menuBar: os !== 'linux',
  primaryId: MAIN_ID,
  windows: [{ id: MAIN_ID, title: MAIN_WINDOW_TITLE, brightness: options.brightness ?? 'system' }],
  front: null,
  tally: emptyTally(),
  returns: {
    setMenuBar: os !== 'linux',
    ...(options.brightness ? { setBrightness: true } : {}),
    ...(options.badge ? { setBadgeLabel: true } : {}),
    ...(options.progress !== undefined ? { setProgressBar: true } : {}),
  },
})

/** `setBadgeLabel`: what the desktop accepts, and what it draws. */
export function simulateBadge(label: string, os: Os): { ok: boolean; shown: string } {
  // LauncherEntry carries a count, not text.
  if (os === 'linux') return { ok: label === '' || /^\d+$/.test(label), shown: label }
  // The taskbar overlay icon fits three characters.
  if (os === 'windows') return { ok: true, shown: label.slice(0, 3) }
  return { ok: true, shown: label }
}

const startupLog = (os: Os, options: ApplicationOptions = {}) => [
  `isSingleInstance → false`,
  `setPrimaryWindow(window #${MAIN_ID})`,
  os === 'linux' ? 'setMenuBar(menu) → false · GTK 3 legacy menus' : 'setMenuBar(menu) → true',
  `getAllWindows → 1`,
  ...(options.brightness ? [`setBrightness(${BRIGHTNESS_NAMES[options.brightness]}) → true`] : []),
  ...(options.badge ? [`setBadgeLabel("${options.badge}") → true`] : []),
  ...(options.progress !== undefined ? [`setProgressBar(${options.progress}) → true`] : []),
  `run(window #${MAIN_ID})`,
]

/**
 * The application example's run, simulated: the lifecycle events as the
 * desktop raises them, and what each `Application` call does to the dock,
 * the taskbar, the windows and the menu bar.
 */
export function useApplication(os: Os, options: ApplicationOptions = {}) {
  const { lastEvent, log, event, call, clear } = useEventLog(startupLog(os, options), 'Starting…')
  const [state, setState] = useState(() => launched(os, options))
  const ref = useRef(state)
  ref.current = state
  const timer = useRef<number | undefined>(undefined)

  const emit = useCallback(
    (type: AppEventType, label: string, detail = '') => {
      setState(s => {
        const tally = { ...s.tally, [type]: { count: s.tally[type].count + 1, at: performance.now() } }
        return { ...s, tally }
      })
      const count = ref.current.tally[type].count + 1
      event(label, `${eventName(type)} #${count}${detail}`)
    },
    [event],
  )

  const quit = useCallback(
    (code: number) => {
      if (!ref.current.running || ref.current.quitting) return
      call(`quit(${code})`)
      emit('quitRequested', 'Quit requested')
      setState(s => ({ ...s, quitting: true }))
      // Core carries the request to the main thread, then stops the loop.
      timer.current = window.setTimeout(() => {
        emit('exiting', `Exiting with ${code}`, ` → exitCode ${code}`)
        setState(s => ({ ...s, running: false, quitting: false, active: false, exitCode: code }))
        call(`run() returned ${code}`)
      }, 600)
    },
    [call, emit],
  )

  const started = useRef(false)
  const start = useCallback(() => {
    emit('started', 'Started')
    emit('activated', 'Activated')
  }, [emit])

  useEffect(() => {
    if (started.current) return
    started.current = true
    start()
    if (options.quitAtStart !== undefined) {
      const code = options.quitAtStart
      timer.current = window.setTimeout(() => quit(code), 500)
    }
    // Once, as the example starts.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [])
  useEffect(() => () => clearTimeout(timer.current), [])

  const set = (patch: Partial<ApplicationState>) => setState(s => ({ ...s, ...patch }))
  const returned = (name: string, ok: boolean) => setState(s => ({ ...s, returns: { ...s.returns, [name]: ok } }))

  const actions = {
    quit,
    relaunch: () => {
      setState(launched(os, {}))
      for (const line of startupLog(os)) call(line)
      start()
    },
    /** A press on the app's window, its dock tile or its taskbar button. */
    activate: (front: number | null = ref.current.front) => {
      const s = ref.current
      if (!s.running) return
      if (s.front !== front) set({ front })
      if (s.active && !s.hidden) return
      set({ active: true, hidden: false })
      emit('activated', 'Activated')
    },
    /** A press on the desktop: another app (the Finder, the shell) takes the focus. */
    deactivate: () => {
      const s = ref.current
      if (!s.running || !s.active) return
      set({ active: false })
      emit('deactivated', 'Deactivated')
    },
    hide: () => {
      if (os !== 'macos') {
        call('hide() → false · no application-level hiding')
        returned('hide', false)
        return
      }
      call('hide() → true')
      returned('hide', true)
      if (ref.current.hidden) return
      const wasActive = ref.current.active
      set({ hidden: true, active: false })
      if (wasActive) emit('deactivated', 'Hidden')
    },
    show: () => {
      if (os !== 'macos') {
        call('show() → false · no application-level hiding')
        returned('show', false)
        return
      }
      call('show() → true')
      returned('show', true)
      const s = ref.current
      if (s.active && !s.hidden) return
      set({ hidden: false, active: true })
      emit('activated', 'Shown')
    },
    setBadgeLabel: (label: string) => {
      const { ok, shown } = simulateBadge(label, os)
      call(`setBadgeLabel("${label}") → ${ok}${ok ? '' : ' · LauncherEntry takes a number'}`)
      returned('setBadgeLabel', ok)
      if (ok) set({ badge: shown })
    },
    /** `log` false while a slider drags: the call is logged once, when it settles. */
    setProgressBar: (progress: Progress, log = true) => {
      if (log) call(`setProgressBar(${progress === -1 ? '-1' : +progress.toFixed(2)}) → true`)
      returned('setProgressBar', true)
      set({ progress })
    },
    setDockIconVisible: (visible: boolean) => {
      call(`setDockIconVisible(${visible}) → true`)
      returned('setDockIconVisible', true)
      set({ dockIconVisible: visible })
    },
    setIcon: (icon: AppIconPreset) => {
      const preset = ICON_PRESETS.find(p => p.value === icon)!
      const ok = icon !== 'missing'
      call(`setIcon("${preset.path}") → ${ok}${ok ? '' : ' · no such file'}`)
      returned('setIcon', ok)
      if (ok) set({ icon })
    },
    setBrightness: (brightness: Brightness) => {
      call(`setBrightness(${BRIGHTNESS_NAMES[brightness]}) → true`)
      returned('setBrightness', true)
      // Every platform restyles the windows open now; only Windows leaves the
      // ones created later as the system has them.
      setState(s => ({ ...s, brightness, windows: s.windows.map(w => ({ ...w, brightness })) }))
    },
    setMenuBar: (on: boolean) => {
      if (!on) {
        // There is no unset: the example swaps in an empty menu.
        call('setMenuBar(emptyMenu) → ' + String(os !== 'linux'))
        set({ menuBar: false })
        return
      }
      const ok = os === 'linux' ? false : os === 'windows' ? ref.current.primaryId !== null : true
      call(`setMenuBar(menu) → ${ok}${os === 'linux' ? ' · GTK 3 legacy menus' : ok ? '' : ' · no primary window'}`)
      returned('setMenuBar', ok)
      if (ok) set({ menuBar: true })
    },
    setPrimaryWindow: (id: number) => {
      call(`setPrimaryWindow(window #${id})`)
      set({ primaryId: id })
    },
    openWindow: () => {
      const s = ref.current
      const id = Math.max(...s.windows.map(w => w.id)) + 1
      // A window made after setBrightness: Windows does not restyle it.
      const brightness: Brightness = os === 'windows' ? 'system' : s.brightness
      call(`Window() → window #${id} · getAllWindows → ${s.windows.length + 1}`)
      setState(prev => ({ ...prev, front: id, windows: [...prev.windows, { id, title: `Window #${id}`, brightness }] }))
    },
    closeWindow: (id: number) => {
      if (id === MAIN_ID) {
        // The example quits when its main window closes.
        quit(0)
        return
      }
      call(`window #${id}.close() · getAllWindows → ${ref.current.windows.length - 1}`)
      setState(s => ({
        ...s,
        windows: s.windows.filter(w => w.id !== id),
        front: s.front === id ? null : s.front,
        primaryId: s.primaryId === id ? null : s.primaryId,
      }))
    },
    menuItem: (label: 'About' | 'Quit') => {
      event(`Menu: ${label}`, `MenuItemClickedEvent "${label}"`)
      if (label === 'Quit') quit(0)
    },
    clearLog: clear,
    call,
  }

  return { state, actions, lastEvent, log }
}

export type ApplicationActions = ReturnType<typeof useApplication>['actions']
