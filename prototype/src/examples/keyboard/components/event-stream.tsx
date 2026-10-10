import type { Os } from '../../../components/platform'
import { describeMask, formatKeycode } from '../data'
import type { KeyEventRecord } from '../types'
import './event-stream.css'

export interface EventStreamProps {
  os: Os
  /** Newest first. */
  events: readonly KeyEventRecord[]
  /** Show at most this many. */
  limit?: number
  empty?: string
}

/** The events as the example prints them: `[key] pressed 0`, `[key] modifiers 0x0003 → Shift + Ctrl`. */
export function EventStream({ os, events, limit, empty = 'No events yet.' }: EventStreamProps) {
  const shown = limit ? events.slice(0, limit) : events
  if (shown.length === 0) return <p className="event-stream__empty">{empty}</p>
  return (
    <ol className="event-stream">
      {shown.map(r => (
        <li key={r.number} data-type={r.type}>
          <span className="event-stream__number">#{r.number}</span>
          <span className="event-stream__type">
            [key] {r.type === 'modifiers' ? 'modifiers' : r.type === 'pressed' ? 'pressed ' : 'released'}
          </span>
          {r.type === 'modifiers' ? (
            <span className="event-stream__value">{describeMask(r.modifiers, os)}</span>
          ) : (
            <>
              <span className="event-stream__value">{formatKeycode(r.keycode, os)}</span>
              <span className="event-stream__key">
                {r.key}
                {r.repeat && ' · repeat'}
              </span>
            </>
          )}
        </li>
      ))}
    </ol>
  )
}
