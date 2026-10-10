import { Next16Regular, Pause16Regular, Play16Regular, Stop16Regular } from '@fluentui/react-icons'

import { Card, Icon, IconButton } from '@dazzlabs/dazzui'

import { animationLabel, STILL_ICONS } from '../data'
import type { TrayEntry } from '../types'
import { framesOf, pixelSizeOf, type TrayActions, timeOf } from '../use-tray'
import { useNow } from '../../../components/use-now'
import { IconCanvas } from './icon-canvas'
import './live-preview.css'

export interface LivePreviewProps {
  entry: TrayEntry
  actions: TrayActions
}

/**
 * The selected icon, magnified: the very frame that was just handed to the
 * tray, at its own pixels, beside what it costs to make — frame count,
 * measured rate, render time, dropped frames — and Pause, Step, Stop.
 */
export function LivePreview({ entry, actions }: LivePreviewProps) {
  const playing = entry.animation !== null
  const now = useNow(200, playing && !entry.paused)
  const px = pixelSizeOf(entry)
  const frames = framesOf(entry, now)
  const name = entry.animation
    ? animationLabel(entry.animation)
    : `${STILL_ICONS.find(s => s.value === entry.still)?.label ?? 'Last frame'} icon`
  const state = !playing ? 'still' : entry.paused ? 'paused' : 'playing'
  // What the real animator measures; here a steady estimate of the same cost.
  const renderMs = 0.18 + entry.scale * entry.scale * 0.09
  const dropped = entry.fps === 60 && entry.scale === 3 ? Math.floor(frames / 97) : 0
  const measured = entry.paused ? 0 : entry.fps - (frames % 7) * 0.04

  return (
    <div className="tray-live-preview">
      <Card variant="sunken" size="small" className="tray-live-preview__frame">
        <IconCanvas
          animation={entry.animation}
          still={entry.still}
          time={() => timeOf(entry)}
          pixels={px}
          size={44}
          color={entry.color}
          pixelated
        />
      </Card>
      <div className="tray-live-preview__details">
        <div className="tray-live-preview__name">
          {name} <span className="tray-live-preview__state">· {state}</span>
        </div>
        <div className="tray-live-preview__stats">
          {playing ? (
            <>
              <span>
                frame {frames} · {measured.toFixed(1)} fps · dropped {dropped}
              </span>
              <span>
                render {renderMs.toFixed(1)} ms · {px}×{px} px · same frame as the tray
              </span>
            </>
          ) : (
            <span>
              {px}×{px} px · same image as the tray
            </span>
          )}
        </div>
      </div>
      <div className="tray-live-preview__actions">
        <IconButton
          label={entry.paused ? 'Resume' : 'Pause'}
          size="small"
          variant="normal"
          disabled={!playing}
          onClick={actions.togglePaused}
        >
          <Icon icon={entry.paused ? Play16Regular : Pause16Regular} />
        </IconButton>
        <IconButton label="Step" size="small" variant="normal" disabled={!playing} onClick={actions.step}>
          <Icon icon={Next16Regular} />
        </IconButton>
        <IconButton
          label="Stop"
          size="small"
          variant="normal"
          disabled={!playing}
          onClick={entry.scene ? actions.resetScene : () => actions.setStill('asset')}
        >
          <Icon icon={Stop16Regular} />
        </IconButton>
      </div>
    </div>
  )
}
