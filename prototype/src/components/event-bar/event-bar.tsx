import { Button, Popover } from '@dazzlabs/dazzui'

import './event-bar.css'

export interface EventBarProps {
  lastEvent: string
  /** Newest first. */
  log: string[]
  onClear: () => void
}

/** The window's foot: the last event in large type, the newest API call under it, the whole log a click away. */
export function EventBar({ lastEvent, log, onClear }: EventBarProps) {
  return (
    <div className="event-bar">
      <div className="event-bar__text">
        <span className="event-bar__event">{lastEvent}</span>
        <span className="event-bar__line">{log[0] ?? 'No calls yet'}</span>
      </div>
      <Popover
        title="Calls and events"
        side="top"
        align="end"
        width={320}
        trigger={
          <Button size="small" variant="normal" disabled={log.length === 0}>
            Log {log.length > 0 && `(${log.length})`}
          </Button>
        }
      >
        <ol className="event-bar__log">
          {log.map((line, i) => (
            <li key={`${log.length - i}`}>{line}</li>
          ))}
        </ol>
      </Popover>
      <Button size="small" variant="plain" onClick={onClear} disabled={log.length === 0}>
        Clear
      </Button>
    </div>
  )
}
