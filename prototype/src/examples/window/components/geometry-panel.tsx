import { useRef } from 'react'

import { Button, PreferenceRow, PreferenceSection } from '@dazzlabs/dazzui'

import { usePointerDrag } from '../../../components/desktop-stage'
import { Panel, PanelPreferences } from '../../../components/panel'
import { ReadBack } from '../../../components/read-back'
import { CONTENT_SIZE, MAXIMUM_SIZE, MINIMUM_SIZE, POSITION_PRESETS, SIZE_PRESETS } from '../data'
import type { Rect, Size } from '../types'
import { boundsOf, rectText, sizeText } from '../use-windows'
import type { WindowPanelProps } from './state-panel'
import './panels.css'

/** No limit reads back as 0×0 (minimum) or as the largest float (maximum). */
const limitText = (s: Size | null, max: boolean) => (s ? sizeText(s) : max ? 'none (FLT_MAX)' : 'none (0×0)')

/**
 * Where the window is and how big, the limits on its size, and the two
 * gestures handed to the window manager. Start dragging and Start resizing
 * are press-and-hold buttons, as the calls are made from a mouse-down: the
 * window follows the pointer until it is released.
 */
export function GeometryPanel({ w, platform, area, actions }: WindowPanelProps) {
  const { bounds, content } = boundsOf(w, area, platform)
  const id = w.id
  const start = useRef<Rect>(w.frame)

  const dragging = usePointerDrag({
    onStart: () => {
      start.current = { ...w.frame }
      actions.startDragging(id)
    },
    onMove: ({ dx, dy }) => actions.moveTo(id, start.current.x + dx, start.current.y + dy),
  })
  const resizing = usePointerDrag({
    onStart: () => {
      start.current = { ...w.frame }
      actions.startResizing(id, 'bottomRight')
    },
    onMove: ({ dx, dy }) => actions.resizeFrom(id, start.current, 'bottomRight', dx, dy),
  })
  const busy = w.maximized || w.fullScreen || w.minimized || !w.visible

  return (
    <Panel>
      <ReadBack
        keyWidth="8.5rem"
        rows={[
          ['getBounds', rectText(bounds)],
          ['getContentBounds', rectText(content)],
          ['getSize', sizeText(bounds)],
          ['getContentSize', sizeText(content)],
          ['getPosition', `${bounds.x}, ${bounds.y}`],
          ['getMinimumSize', limitText(w.minimumSize, false)],
          ['getMaximumSize', limitText(w.maximumSize, true)],
          ['work area', `0, ${area.barTop ? area.bar : 0} ${sizeText(area)}`],
        ]}
      />
      <PanelPreferences>
        <PreferenceSection label="Position and size">
          <PreferenceRow title="Size" subtitle="setSize(size, animate: false)">
            <div className="example-panel__actions">
              {SIZE_PRESETS.map(size => (
                <Button key={sizeText(size)} size="small" variant="normal" onClick={() => actions.setSize(id, size)}>
                  {sizeText(size)}
                </Button>
              ))}
            </div>
          </PreferenceRow>
          <PreferenceRow title="Content size" subtitle="The frame grows by the title bar">
            <Button size="small" variant="normal" onClick={() => actions.setContentSize(id, CONTENT_SIZE)}>
              Content {sizeText(CONTENT_SIZE)}
            </Button>
          </PreferenceRow>
          <PreferenceRow title="Position" subtitle="Screen coordinates, the bar included">
            <div className="example-panel__actions">
              <Button size="small" variant="normal" onClick={() => actions.center(id)}>
                Center
              </Button>
              {POSITION_PRESETS.map(p => (
                <Button key={p.x} size="small" variant="normal" onClick={() => actions.setPosition(id, p.x, p.y)}>
                  ({p.x}, {p.y})
                </Button>
              ))}
            </div>
          </PreferenceRow>
        </PreferenceSection>
        <PreferenceSection label="Size constraints" footer="Drag the window's corner to feel them.">
          <PreferenceRow title="Minimum size" subtitle={limitText(w.minimumSize, false)}>
            <Button size="small" variant="normal" onClick={() => actions.setMinimumSize(id, MINIMUM_SIZE)}>
              Min {sizeText(MINIMUM_SIZE)}
            </Button>
          </PreferenceRow>
          <PreferenceRow title="Maximum size" subtitle={limitText(w.maximumSize, true)}>
            <Button size="small" variant="normal" onClick={() => actions.setMaximumSize(id, MAXIMUM_SIZE)}>
              Max {sizeText(MAXIMUM_SIZE)}
            </Button>
          </PreferenceRow>
          <PreferenceRow title="Minimum and maximum">
            <Button
              size="small"
              variant="normal"
              disabled={!w.minimumSize && !w.maximumSize}
              onClick={() => {
                actions.setMinimumSize(id, null)
                actions.setMaximumSize(id, null)
              }}
            >
              Reset constraints
            </Button>
          </PreferenceRow>
        </PreferenceSection>
        <PreferenceSection
          label="Interactions"
          footer="Both hand the pointer to the window manager from a mouse-down: press, move the mouse, release to stop."
        >
          <PreferenceRow title="Move with the mouse" subtitle="startDragging()">
            <Button size="small" variant="normal" className="window-panel__hold" disabled={busy} onPointerDown={dragging}>
              Press and drag
            </Button>
          </PreferenceRow>
          <PreferenceRow title="Resize from the bottom-right" subtitle="startResizing(ResizeEdge.bottomRight)">
            <Button
              size="small"
              variant="normal"
              className="window-panel__hold"
              disabled={busy || !w.resizable}
              onPointerDown={resizing}
            >
              Press and drag
            </Button>
          </PreferenceRow>
        </PreferenceSection>
      </PanelPreferences>
    </Panel>
  )
}
