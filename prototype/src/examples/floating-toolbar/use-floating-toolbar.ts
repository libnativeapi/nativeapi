import { useCallback, useState } from 'react'

import { useEventLog } from '../../components/event-bar'
import { MAIN_AT, TOOLBAR_SIZE, toolbarAbove, WAYLAND_TOOLBAR_AT } from './data'
import type { Point, Swatch } from './types'

export interface FloatingToolbarOptions {
  /** The toolbar is the main window's child as the example starts. */
  attached?: boolean
  toolbarVisible?: boolean
  /** How far the main window has been moved from where it started, as a scene. */
  mainOffset?: Point
}

const at = (p: Point) => `${Math.round(p.x)}, ${Math.round(p.y)}`

/**
 * The two windows' shared model and the calls the example makes on them.
 * Where the app can place windows the toolbar is re-centred above the main
 * window on every WindowMovedEvent while attached; on Wayland it cannot be,
 * and the compositor's placement stands until the user drags the pill.
 */
export function useFloatingToolbar(canPlaceWindows: boolean, options: FloatingToolbarOptions = {}) {
  const { attached: attachedAtStart = true, toolbarVisible: visibleAtStart = true, mainOffset } = options
  const startMain = mainOffset ? { x: MAIN_AT.x + mainOffset.x, y: MAIN_AT.y + mainOffset.y } : MAIN_AT
  // A detached toolbar stayed where the main window was when it let go.
  const startToolbar = canPlaceWindows ? toolbarAbove(attachedAtStart ? startMain : MAIN_AT) : WAYLAND_TOOLBAR_AT

  const log = useEventLog(
    [
      ...(canPlaceWindows ? [`main.setPosition(${at(MAIN_AT)})`] : []),
      'toolbar.setTitleBarStyle(hidden)',
      'toolbar.setBackgroundColor(transparent)',
      'toolbar.setHasShadow(false)',
      'toolbar.setResizable(false)',
      'toolbar.setMovable(false)',
      'toolbar.setVisibleInTaskbar(false)',
      `toolbar.setContentSize(${TOOLBAR_SIZE.width}×${TOOLBAR_SIZE.height})`,
      'toolbar.setParentWindow(main) → true',
      ...(canPlaceWindows ? [`toolbar.setPosition(${at(startToolbar)})`] : []),
      'Toolbar attached to the main window',
      ...(attachedAtStart ? [] : ['toolbar.setParentWindow(null)', 'Toolbar detached: it no longer follows']),
      ...(mainOffset && canPlaceWindows ? [`WindowMovedEvent (main) → ${at(startMain)}`] : []),
      ...(visibleAtStart ? [] : ['toolbar.hide()', 'Toolbar hidden']),
    ],
    !visibleAtStart ? 'Toolbar hidden' : attachedAtStart ? 'Toolbar attached to the main window' : 'Toolbar detached: it no longer follows',
  )

  const [color, setColor] = useState<Swatch>('primary')
  const [stamps, setStamps] = useState(0)
  const [attached, setAttached] = useState(attachedAtStart)
  const [toolbarVisible, setToolbarVisible] = useState(visibleAtStart)
  const [mainAt, setMainAt] = useState<Point>(startMain)
  const [toolbarAt, setToolbarAt] = useState<Point>(startToolbar)
  const [minimized, setMinimized] = useState(false)
  const [closed, setClosed] = useState(false)

  const place = useCallback(
    (main: Point) => {
      if (canPlaceWindows) setToolbarAt(toolbarAbove(main))
    },
    [canPlaceWindows],
  )

  const attach = useCallback(() => {
    log.call('toolbar.setParentWindow(main) → true')
    if (canPlaceWindows) {
      place(mainAt)
      log.call(`toolbar.setPosition(${at(toolbarAbove(mainAt))})`)
    }
    setAttached(true)
    log.event('Toolbar attached to the main window', 'attached → true')
  }, [log, canPlaceWindows, place, mainAt])

  const detach = useCallback(() => {
    log.call('toolbar.setParentWindow(null)')
    setAttached(false)
    log.event('Toolbar detached: it no longer follows', 'parentWindow → null')
  }, [log])

  const showToolbar = useCallback(
    (visible: boolean) => {
      if (visible) {
        if (attached && canPlaceWindows) place(mainAt)
        log.call('toolbar.showInactive()')
      } else {
        log.call('toolbar.hide()')
      }
      setToolbarVisible(visible)
      log.event(visible ? 'Toolbar shown' : 'Toolbar hidden', `toolbar.isVisible → ${visible}`)
    },
    [attached, canPlaceWindows, place, mainAt, log],
  )

  /** A frame of the main window's drag: every WindowMovedEvent re-places an attached toolbar. */
  const moveMain = useCallback(
    (p: Point) => {
      setMainAt(p)
      if (attached) place(p)
    },
    [attached, place],
  )

  /** The drag is over: one line for the events it raised, rather than one per frame. */
  const endMainDrag = useCallback(
    (p: Point, moves: number) => {
      if (moves === 0) return
      if (!canPlaceWindows) {
        log.event('Moved by the compositor', 'No WindowMovedEvent: Wayland does not tell a client where it is')
        return
      }
      log.call(`WindowMovedEvent ×${moves} (main) → ${at(p)}`)
      if (attached) log.event('Toolbar followed the main window', `toolbar.setPosition(${at(toolbarAbove(p))}) ×${moves}`)
      else log.event('Main window moved; the toolbar stayed')
    },
    [canPlaceWindows, attached, log],
  )

  /** Wayland only: the pill starts a compositor drag of its own window. */
  const moveToolbar = useCallback((p: Point) => setToolbarAt(p), [])

  const minimize = useCallback(() => {
    log.call('main.minimize()')
    setMinimized(true)
    // A child is hidden while its parent is minimized; a detached toolbar stays.
    log.event('minimized: main', attached ? 'WindowMinimizedEvent (main) · toolbar hidden with it' : 'WindowMinimizedEvent (main)')
  }, [log, attached])

  const restore = useCallback(() => {
    log.call('main.restore()')
    setMinimized(false)
    if (attached) {
      place(mainAt)
      log.event('restored: main', canPlaceWindows ? `toolbar.setPosition(${at(toolbarAbove(mainAt))})` : 'WindowRestoredEvent (main)')
    } else {
      log.event('restored: main', 'WindowRestoredEvent (main)')
    }
  }, [log, attached, place, mainAt, canPlaceWindows])

  /** Children first, then the parent: what closing a parent does to its children differs between platforms. */
  const close = useCallback(() => {
    log.call('toolbar.setParentWindow(null)')
    log.call('toolbar.destroy()')
    log.event('closed: main', 'main.destroy() · exitApplication')
    setClosed(true)
  }, [log])

  const pick = useCallback((value: Swatch) => setColor(value), [])
  const stamp = useCallback(() => setStamps(n => n + 1), [])

  return {
    log,
    canPlaceWindows,
    color,
    stamps,
    attached,
    toolbarVisible,
    mainAt,
    toolbarAt,
    minimized,
    closed,
    pick,
    stamp,
    attach,
    detach,
    showToolbar,
    moveMain,
    endMainDrag,
    moveToolbar,
    minimize,
    restore,
    close,
  }
}

export type FloatingToolbar = ReturnType<typeof useFloatingToolbar>
