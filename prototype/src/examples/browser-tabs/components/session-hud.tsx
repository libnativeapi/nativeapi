import { ArrowReset20Regular } from '@fluentui/react-icons'

import { Badge, Button, Callout, Icon, type Tint } from '@dazzlabs/dazzui'

import { EventBar } from '../../../components/event-bar'
import { ReadBack } from '../../../components/read-back'
import type { TabsSimulation } from '../simulate-tab-drag'
import type { DragMode } from '../types'
import './session-hud.css'

const MODES: Record<DragMode, { tint: Tint; label: string; detail: string }> = {
  idle: { tint: 'neutral', label: 'idle', detail: 'No gesture' },
  pending: { tint: 'warning', label: 'pending', detail: 'Pressed, not 8px yet' },
  inStrip: { tint: 'info', label: 'inStrip', detail: 'Tab slides along a strip' },
  window: { tint: 'primary', label: 'window', detail: 'A window follows the cursor' },
  moveWindow: { tint: 'primary', label: 'moveWindow', detail: 'The strip moves its window' },
}

export interface SessionHudProps {
  tabs: TabsSimulation
  wayland: boolean
  onRestart: () => void
}

/**
 * Laid over the desktop, under the first window: what the example's
 * `WindowDragSession` is doing — its state, the window it moves, the cursor
 * and the last `getWindowAtPoint` — and the calls and events so far.
 */
export function SessionHud({ tabs, wayland, onRestart }: SessionHudProps) {
  const { hud } = tabs
  const mode = MODES[hud.mode]
  const recent = tabs.log.slice(0, 5)
  return (
    <div className="session-hud">
      <ReadBack
        label="WindowDragSession"
        action={
          <Badge size="small" variant="tinted" tint={mode.tint}>
            {mode.label}
          </Badge>
        }
        columns={1}
        keyWidth="8.5rem"
        rows={[
          ['state', mode.detail],
          [
            'moves',
            hud.mode === 'idle' ? '—' : hud.sessionWindow === null ? 'nothing · pointer only' : `window #${hud.sessionWindow}`,
          ],
          ['cursor', hud.mode !== 'idle' && hud.cursor ? `${Math.round(hud.cursor.x)}, ${Math.round(hud.cursor.y)}` : '—'],
          ['getWindowAtPoint', hud.hit === undefined ? '—' : hud.hit === null ? 'null' : `#${hud.hit}`],
        ]}
      />
      {wayland && (
        <Callout size="small" tint="warning" title="Wayland: no dragging between windows">
          WindowDragSession.start returns false: tabs reorder within their strip only, and the empty strip moves its
          window with startDragging().
        </Callout>
      )}
      <ol className="session-hud__log" aria-label="Recent calls and events">
        {recent.length === 0 ? <li>No calls yet</li> : recent.map((line, i) => <li key={`${tabs.log.length - i}`}>{line}</li>)}
      </ol>
      {tabs.quit ? (
        <Button size="small" variant="filled" onClick={onRestart}>
          <Icon icon={ArrowReset20Regular} />
          Run the example again
        </Button>
      ) : (
        <EventBar lastEvent={tabs.lastEvent} log={tabs.log} onClear={() => tabs.clearLog()} />
      )}
    </div>
  )
}
