import { Button, type WindowFramePlatform } from '@dazzlabs/dazzui'

import { Panel } from '../../../components/panel'
import { PLATFORM_NAMES } from '../../../components/platform'
import { ReadBack } from '../../../components/read-back'
import type { Area, SimWindow } from '../types'
import { boundsOf, rectText, sizeText, type WindowActions } from '../use-windows'
import { StateBadges } from './state-badges'
import './map-panel.css'

export interface MapPanelProps {
  windows: SimWindow[]
  selected: SimWindow
  focusedId: number | null
  platform: WindowFramePlatform
  area: Area
  actions: WindowActions
}

/** The tallest the map is drawn, so the read-backs fit under it. */
const MAP_HEIGHT = 230

const pct = (value: number, of: number) => `${(value / of) * 100}%`

/**
 * The display and every window on it, to scale: the frame of each, its
 * content bounds inside, hidden ones dashed and minimized ones faint. A
 * click selects a window; under the map, what the selected one and the
 * manager read back, with the four most used calls.
 */
export function MapPanel({ windows, selected, focusedId, platform, area, actions }: MapPanelProps) {
  const width = area.width
  const height = area.height + area.bar
  const { bounds, content } = boundsOf(selected, area, platform)
  const id = selected.id

  return (
    <Panel>
      <div
        className="window-map"
        style={{ aspectRatio: `${width} / ${height}`, width: `min(100%, ${Math.round((MAP_HEIGHT * width) / height)}px)` }}
      >
        <div
          className="window-map__bar"
          style={{ top: area.barTop ? 0 : undefined, bottom: area.barTop ? undefined : 0, height: pct(area.bar, height) }}
        />
        {[...windows]
          .sort((a, b) => a.z - b.z)
          .map(w => {
            const b = boundsOf(w, area, platform)
            const bar = b.content.y - b.bounds.y
            return (
              <button
                key={w.id}
                type="button"
                className="window-map__window"
                data-selected={w.id === id ? '' : undefined}
                data-focused={w.id === focusedId ? '' : undefined}
                data-hidden={!w.visible ? '' : undefined}
                data-minimized={w.minimized ? '' : undefined}
                style={{
                  left: pct(b.bounds.x, width),
                  top: pct(b.bounds.y, height),
                  width: pct(b.bounds.width, width),
                  height: pct(b.bounds.height, height),
                }}
                onClick={() => actions.pick(w.id)}
              >
                <span className="window-map__content" style={{ top: pct(bar, b.bounds.height) }} />
                <span className="window-map__label">
                  #{w.id} {w.title}
                  <span>{w.minimized ? 'minimized' : !w.visible ? 'hidden' : sizeText(b.bounds)}</span>
                </span>
              </button>
            )
          })}
      </div>
      <p className="example-panel__note">
        {PLATFORM_NAMES[platform]} display, {sizeText({ width, height })} · click a window to select it · frames in
        outline, content bounds tinted
      </p>
      <div className="window-map__details">
        <ReadBack
          label={`#${id} ${selected.title}`}
          columns={1}
          keyWidth="8rem"
          rows={[
            ['getBounds', rectText(bounds)],
            ['getContentBounds', rectText(content)],
            ['isVisible', String(selected.visible)],
            ['isFocused', String(focusedId === id)],
          ]}
        />
        <ReadBack
          label="WindowManager"
          columns={1}
          keyWidth="8rem"
          rows={[
            ['getAll()', `[${windows.map(w => `#${w.id}`).join(', ')}]`],
            ['getCurrent()', '#1 · the example'],
            ['focused', focusedId === null ? 'none' : `#${focusedId}`],
            ['display', `0, 0 ${sizeText({ width, height })}`],
          ]}
        />
      </div>
      <StateBadges w={selected} focused={focusedId === id} />
      <div className="example-panel__actions">
        <Button size="small" variant="normal" onClick={() => actions.maximize(id)}>
          Maximize
        </Button>
        <Button size="small" variant="normal" onClick={() => actions.minimize(id)}>
          Minimize
        </Button>
        <Button size="small" variant="normal" onClick={() => actions.restore(id)}>
          Restore
        </Button>
        <Button size="small" variant="normal" onClick={() => (selected.visible ? actions.hide(id) : actions.show(id))}>
          {selected.visible ? 'Hide' : 'Show'}
        </Button>
        <Button size="small" variant="plain" onClick={() => actions.select(id)}>
          More actions
        </Button>
      </div>
    </Panel>
  )
}
