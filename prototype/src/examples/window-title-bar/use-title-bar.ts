import { useCallback, useMemo, useRef, useState } from 'react'

import type { WindowFramePlatform } from '@dazzlabs/dazzui'

import { useEventLog } from '../../components/event-bar'
import { osOf, PLATFORM_NAMES } from '../../components/platform'
import { CONTENT_SIZE, NOTES, TITLE_BAR_HEIGHT, WINDOW_TITLE } from './data'
import type { Size, TitleBarState, TitleBarWindow } from './types'

export interface TitleBarOptions {
  /** The state the title bar starts in, as if its chip had been pressed. */
  state?: TitleBarState
  /** Hide the buttons after the state, as the Hide buttons chip does. */
  buttonsHidden?: boolean
}

/** The desktop the window stands on, measured by the view. */
export interface Desk {
  width: number
  height: number
}

/** The state the chips show as selected. */
export function stateOf(w: TitleBarWindow): TitleBarState {
  if (w.titleBarStyle === 'hidden') return 'hidden'
  return w.contentUnderTitleBar ? 'under' : 'normal'
}

/**
 * The prototype's stand-in for the example's one window: what
 * `setTitleBarStyle`, `setContentUnderTitleBar` and
 * `setWindowControlButtonsVisible` do to it on each desktop, and what its
 * getters return afterwards. The frame never changes with the state; the
 * content takes in or gives up the title bar's height.
 */
export function useTitleBar(platform: WindowFramePlatform, options: TitleBarOptions = {}) {
  const os = osOf(platform)
  const supported = os === 'macos'
  const bar = TITLE_BAR_HEIGHT[platform]
  const log = useEventLog(
    [
      `setTitle("${WINDOW_TITLE}")`,
      'setMinimumSize(520×420)',
      `setContentSize(${CONTENT_SIZE.width}×${CONTENT_SIZE.height})`,
      'center()',
    ],
    'Try the three states and watch the strip at the top',
  )
  const [desk, setDesk] = useState<Desk>({ width: 1320, height: 720 })
  const [running, setRunning] = useState(true)

  const [w, setW] = useState<TitleBarWindow>(() => {
    // The example sizes its content under a normal title bar; every state
    // after that keeps the frame this gives.
    const width = CONTENT_SIZE.width
    const height = CONTENT_SIZE.height + bar
    const start: TitleBarWindow = {
      titleBarStyle: 'normal',
      contentUnderTitleBar: false,
      buttonsVisible: true,
      maximized: false,
      x: -1,
      y: -1,
      width,
      height,
    }
    const state = options.state ?? 'normal'
    if (state === 'hidden') Object.assign(start, { titleBarStyle: 'hidden', buttonsVisible: false })
    if (state === 'under' && supported) start.contentUnderTitleBar = true
    if (options.buttonsHidden && supported) start.buttonsVisible = false
    return start
  })
  const ref = useRef(w)
  ref.current = w

  /** What `getContentSize` returns: the frame, less the title bar while it draws one. */
  const contentOf = useCallback(
    (window: TitleBarWindow, area: Desk = desk): Size => {
      const width = window.maximized ? area.width : window.width
      const height = window.maximized ? area.height : window.height
      const drawn = window.titleBarStyle === 'normal' && !window.contentUnderTitleBar
      return { width, height: height - (drawn ? bar : 0) }
    },
    [bar, desk],
  )

  const stateLine = useCallback(
    (window: TitleBarWindow) => {
      const size = contentOf(window)
      return `titleBarStyle=${window.titleBarStyle} under=${window.contentUnderTitleBar} buttons=${window.buttonsVisible} contentSize=${size.width}×${size.height}`
    },
    [contentOf],
  )

  /** One change: its calls logged, the window updated, the note and the getters' line after it. */
  const act = useCallback(
    (note: string, calls: string[], change: (draft: TitleBarWindow) => void) => {
      const draft = { ...ref.current }
      change(draft)
      ref.current = draft
      setW(draft)
      for (const line of calls) log.call(line)
      log.event(note, stateLine(draft))
    },
    [log, stateLine],
  )

  const actions = useMemo(
    () => ({
      setDesk: (next: Desk) => {
        setDesk(next)
        // Centred once the desktop is known, as `center()` did.
        if (ref.current.x < 0) {
          const centred = {
            ...ref.current,
            x: Math.round((next.width - ref.current.width) / 2),
            y: Math.max(0, Math.round((next.height - ref.current.height) / 2)),
          }
          ref.current = centred
          setW(centred)
        }
      },
      setState: (state: TitleBarState) => {
        if (state === 'normal') {
          act(NOTES.normal, ['setContentUnderTitleBar(false) → ' + supported, 'setTitleBarStyle(TitleBarStyle.normal)'], d => {
            d.contentUnderTitleBar = false
            d.titleBarStyle = 'normal'
            d.buttonsVisible = true
          })
        } else if (state === 'under') {
          if (!supported) {
            return log.call(`setContentUnderTitleBar(true) → false · not on ${PLATFORM_NAMES[platform]}`)
          }
          act(NOTES.under, ['setTitleBarStyle(TitleBarStyle.normal)', 'setContentUnderTitleBar(true) → true'], d => {
            d.titleBarStyle = 'normal'
            d.buttonsVisible = true
            d.contentUnderTitleBar = true
          })
        } else {
          act(NOTES.hidden, ['setTitleBarStyle(TitleBarStyle.hidden)'], d => {
            d.titleBarStyle = 'hidden'
            // The style takes the buttons with it, on every platform; the
            // getter only reports it on macOS.
            d.buttonsVisible = !supported
          })
        }
      },
      setButtons: (visible: boolean) =>
        act(visible ? 'Buttons shown' : 'Buttons hidden', [
          `setWindowControlButtonsVisible(${visible})${supported ? '' : ` · not implemented on ${PLATFORM_NAMES[platform]}`}`,
        ], d => {
          if (supported) d.buttonsVisible = visible
        }),
      /** The strip's press: `startDragging()`, and the window manager moves the window. */
      startDragging: () => log.call('startDragging()'),
      moveTo: (x: number, y: number) =>
        setW(current => {
          const next = {
            ...current,
            maximized: false,
            x: Math.round(Math.min(Math.max(x, 80 - current.width), desk.width - 80)),
            y: Math.round(Math.min(Math.max(y, 0), desk.height - 40)),
          }
          ref.current = next
          return next
        }),
      toggleMaximize: () => {
        const maximized = !ref.current.maximized
        act(maximized ? 'Maximized from the strip' : 'Restored from the strip', [maximized ? 'maximize()' : 'unmaximize()'], d => {
          d.maximized = maximized
        })
      },
      quit: () => {
        log.call('Application.quit(0)')
        log.event('The application quit')
        setRunning(false)
      },
      relaunch: () => {
        log.clear()
        setRunning(true)
      },
    }),
    [act, desk, log, platform, supported],
  )

  return { w, desk, running, supported, content: contentOf(w), log, actions }
}
