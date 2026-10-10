import { ArrowReset20Regular } from '@fluentui/react-icons'

import { Badge, Button, Callout, Icon, type Tint } from '@dazzlabs/dazzui'

import { EventBar } from '../../../components/event-bar'
import { ReadBack } from '../../../components/read-back'
import { PANELS } from '../data'
import type { DetachSimulation } from '../simulate-detach'
import type { DragMode } from '../types'
import './session-hud.css'

const MODES: Record<DragMode, { tint: Tint; detail: string }> = {
  idle: { tint: 'neutral', detail: 'No gesture' },
  pending: { tint: 'warning', detail: 'Pressed, not 8px yet' },
  floating: { tint: 'primary', detail: 'A window follows the cursor' },
}

export interface SessionHudProps {
  detach: DetachSimulation
  onRestart: () => void
}

/**
 * Laid over the desktop: what the example's `WindowDragSession` is doing —
 * its state, the window it moves, the cursor, what `getWindowAtPoint` sees
 * and the slot a release would dock into — and the calls and events so far.
 */
export function SessionHud({ detach, onRestart }: SessionHudProps) {
  const { hud } = detach
  const mode = MODES[hud.mode]
  return (
    <div className="detach-hud">
      <ReadBack
        label="WindowDragSession"
        action={
          <Badge size="small" variant="tinted" tint={mode.tint}>
            {hud.mode}
          </Badge>
        }
        columns={1}
        keyWidth="8.5rem"
        rows={[
          ['state', mode.detail],
          ['moves', hud.mode === 'idle' ? '—' : hud.sessionWindow === null ? 'nothing · pointer only' : `window #${hud.sessionWindow}`],
          ['cursor', hud.mode !== 'idle' && hud.cursor ? `${Math.round(hud.cursor.x)}, ${Math.round(hud.cursor.y)}` : '—'],
          ['getWindowAtPoint', hud.hit === undefined ? '—' : hud.hit === null ? 'null' : `#${hud.hit}`],
          ['drop target', detach.hovered ?? '—'],
          [PANELS.inspector.title, detach.describe('inspector')],
          [PANELS.stopwatch.title, detach.describe('stopwatch')],
        ]}
      />
      {detach.wayland && (
        <Callout size="small" tint="warning" title="Wayland: no tear-off by drag">
          WindowDragSession.start returns false, so a drag ends at once. Use Open in a window and Dock back in the
          panel headers instead.
        </Callout>
      )}
      {detach.quit ? (
        <Button size="small" variant="filled" onClick={onRestart}>
          <Icon icon={ArrowReset20Regular} />
          Run the example again
        </Button>
      ) : (
        <EventBar lastEvent={detach.lastEvent} log={detach.log} onClear={() => detach.clearLog()} />
      )}
    </div>
  )
}
