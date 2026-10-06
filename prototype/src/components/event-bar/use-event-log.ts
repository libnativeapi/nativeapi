import { useCallback, useState } from 'react'

const LIMIT = 200

export interface EventLog {
  /** The last event, in large type at the window's foot. */
  lastEvent: string
  /** Calls and events, newest first. */
  log: string[]
  /** An event arrived: it becomes the last event, and a log line. */
  event: (label: string, line?: string) => void
  /** A call was made: a log line only. */
  call: (line: string) => void
  clear: () => void
}

/** The calls an example makes and the events it receives, for its `EventBar`. */
export function useEventLog(initial: readonly string[] = [], idle = 'No events yet'): EventLog {
  const [lastEvent, setLastEvent] = useState(idle)
  const [log, setLog] = useState<string[]>(() => [...initial].reverse())
  const call = useCallback((line: string) => setLog(l => [line, ...l].slice(0, LIMIT)), [])
  const event = useCallback(
    (label: string, line?: string) => {
      setLastEvent(label)
      call(line ?? label)
    },
    [call],
  )
  const clear = useCallback(() => {
    setLog([])
    setLastEvent(idle)
  }, [idle])
  return { lastEvent, log, event, call, clear }
}
