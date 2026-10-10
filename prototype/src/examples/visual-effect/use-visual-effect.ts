import { useCallback, useState } from 'react'

import { useEventLog } from '../../components/event-bar'
import type { Os } from '../../components/platform'
import { BACKDROP_INSET, WINDOW_AT, WINDOW_SIZE } from './data'
import { simulateSetVisualEffect } from './set-visual-effect'
import type { Backdrop, Point, VisualEffect } from './types'

export interface VisualEffectOptions {
  /** An effect applied as the example starts. */
  initialEffect?: VisualEffect
  /** The backdrop already up as the example starts. */
  backdrop?: boolean
}

const backdropAround = (at: Point): Point => ({ x: at.x - BACKDROP_INSET.x, y: at.y - BACKDROP_INSET.y })

/**
 * The example's state: the effect the window's getter reports, the note the
 * last call left, the red backdrop window, and where both stand. Every call
 * the example makes goes to the event log as the bindings spell it.
 */
export function useVisualEffect(os: Os, { initialEffect = 'none', backdrop: backdropAtStart }: VisualEffectOptions) {
  const startApplied = initialEffect !== 'none' && simulateSetVisualEffect(initialEffect, os)
  const log = useEventLog(
    [
      `isContentUnderTitleBarSupported() → ${os === 'macos'}`,
      `setContentSize(${WINDOW_SIZE.width}×${WINDOW_SIZE.height})`,
      'center()',
      'setContentUnderTitleBar(true)',
      ...(backdropAtStart ? backdropCalls(2, WINDOW_AT) : []),
      ...(initialEffect !== 'none' ? [`setVisualEffect(${initialEffect}) → ${startApplied}`] : []),
    ],
    initialEffect !== 'none' ? `${startApplied ? 'Applied' : 'Refused'} ${initialEffect}` : 'Pick an effect',
  )
  const [effect, setEffect] = useState<VisualEffect>(startApplied ? initialEffect : 'none')
  const [note, setNote] = useState(log.lastEvent)
  const [at, setAt] = useState<Point>(WINDOW_AT)
  const [backdrop, setBackdrop] = useState<Backdrop | null>(
    backdropAtStart ? { id: 2, at: backdropAround(WINDOW_AT) } : null,
  )
  const [nextId, setNextId] = useState(backdropAtStart ? 3 : 2)
  const [active, setActive] = useState(true)

  const apply = useCallback(
    (value: VisualEffect) => {
      const ok = simulateSetVisualEffect(value, os)
      const message = `${ok ? 'Applied' : 'Refused'} ${value}`
      log.call(`setVisualEffect(${value}) → ${ok}`)
      if (ok) setEffect(value)
      setNote(message)
      log.event(message, `visualEffect → ${ok ? value : effect}`)
    },
    [os, effect, log],
  )

  const toggleBackdrop = useCallback(() => {
    if (backdrop) {
      log.call('setParentWindow(null) → true')
      log.call(`backdrop #${backdrop.id}.hide()`)
      log.event('Backdrop removed', `backdrop #${backdrop.id}.dispose()`)
      setBackdrop(null)
      return
    }
    for (const line of backdropCalls(nextId, at)) log.call(line)
    log.event('Backdrop shown behind the window', 'focus()')
    setBackdrop({ id: nextId, at: backdropAround(at) })
    setNextId(id => id + 1)
    setActive(true)
  }, [backdrop, nextId, at, log])

  /** The example's window was dragged by its title bar; a child does not take its parent along. */
  const moveWindow = useCallback((p: Point) => setAt(p), [])

  /**
   * The backdrop was dragged. On macOS a child window moves with its parent,
   * so the example comes along; elsewhere it stays where it was.
   */
  const moveBackdrop = useCallback(
    (p: Point, from: { backdrop: Point; window: Point }) => {
      setBackdrop(b => (b ? { ...b, at: p } : b))
      if (os === 'macos') setAt({ x: from.window.x + p.x - from.backdrop.x, y: from.window.y + p.y - from.backdrop.y })
    },
    [os],
  )

  return { log, effect, note, at, backdrop, active, setActive, apply, toggleBackdrop, moveWindow, moveBackdrop }
}

/** What turning the backdrop on calls, in order: a plain red window around this one, which becomes its parent. */
function backdropCalls(id: number, at: Point): string[] {
  const { x, y } = backdropAround(at)
  return [
    `Window.create() → #${id}`,
    `backdrop #${id}.setTitle("Backdrop")`,
    `backdrop #${id}.setBackgroundColor(#FF0000)`,
    `backdrop #${id}.setBounds(${x}, ${y}, ${WINDOW_SIZE.width + 2 * BACKDROP_INSET.x}×${WINDOW_SIZE.height + 2 * BACKDROP_INSET.y})`,
    `backdrop #${id}.show()`,
    `setParentWindow(#${id}) → true`,
  ]
}
