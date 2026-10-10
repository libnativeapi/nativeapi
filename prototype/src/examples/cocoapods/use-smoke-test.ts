import { useCallback, useEffect, useRef, useState } from 'react'

import { useEventLog } from '../../components/event-bar'
import { CHECKS, FAILURES } from './data'
import type { CheckId, CheckResult } from './types'

export interface SmokeTestOptions {
  /** Apple platforms only: elsewhere the example is never built, and nothing runs. */
  buildable: boolean
  /** Checks that throw, as when a module's symbols are missing from the build. */
  failing?: readonly CheckId[]
  /** Run as the example starts (it does), or leave the checks waiting. */
  runAtStart?: boolean
  /** Stop partway, for the story that shows a run in progress. */
  freezeAfter?: number
}

const STAGGER = 320
const DURATION = 420

const idle = () => Object.fromEntries(CHECKS.map(c => [c.id, { status: 'idle', detail: '' }])) as Record<CheckId, CheckResult>

/**
 * The smoke test: every check calls one module and reports what it got,
 * one after another — a check that throws is a failure with the error as its
 * detail. The example runs them as it starts, and again on Run checks.
 */
export function useSmokeTest({ buildable, failing = [], runAtStart = true, freezeAfter }: SmokeTestOptions) {
  const log = useEventLog([], buildable ? 'Starting checks' : 'Not built for this platform')
  const { event, call, clear } = log
  const [results, setResults] = useState(idle)
  const timers = useRef<number[]>([])

  const stop = () => {
    timers.current.forEach(clearTimeout)
    timers.current = []
  }
  useEffect(() => stop, [])

  const run = useCallback(() => {
    if (!buildable) return
    stop()
    clear()
    setResults(idle)
    let failed = 0
    CHECKS.forEach((check, i) => {
      if (freezeAfter !== undefined && i > freezeAfter) return
      const start = i * STAGGER
      timers.current.push(
        window.setTimeout(() => setResults(r => ({ ...r, [check.id]: { status: 'running', detail: '' } })), start),
      )
      if (freezeAfter !== undefined && i === freezeAfter) return
      timers.current.push(
        window.setTimeout(() => {
          const fails = failing.includes(check.id)
          if (fails) failed++
          setResults(r => ({ ...r, [check.id]: { status: fails ? 'fail' : 'pass', detail: fails ? FAILURES[check.id] : check.detail } }))
          if (fails) call(`${check.name} failed: ${FAILURES[check.id]}`)
          else check.calls.forEach(call)
          if (i === CHECKS.length - 1) {
            event(
              failed ? `${failed} failed` : `${CHECKS.length}/${CHECKS.length} passed`,
              `Ran ${CHECKS.length} checks · ${CHECKS.length - failed} passed, ${failed} failed`,
            )
          }
        }, start + DURATION),
      )
    })
    // `failing` and `freezeAfter` are story args: fixed for a run of the example.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [buildable])

  useEffect(() => {
    if (runAtStart) run()
    // Once, as the example starts.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [])

  const list = CHECKS.map(c => results[c.id])
  const running = list.some(r => r.status === 'running')
  const failed = list.filter(r => r.status === 'fail').length
  const done = list.every(r => r.status === 'pass' || r.status === 'fail')

  return { results, running, failed, done, log, run }
}
