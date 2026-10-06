import { ArrowMove20Regular } from '@fluentui/react-icons'
import { useLayoutEffect, useRef } from 'react'

import { Button, Card, Icon, SectionLabel, WindowFrame, type WindowFramePlatform } from '@dazzlabs/dazzui'

import { DesktopStage, DesktopWindow, usePointerDrag } from '../../components/desktop-stage'
import { EventBar } from '../../components/event-bar'
import { ResizeHandles } from './components/resize-handles'
import { LIMITED_EDGES, WINDOW_TITLE } from './data'
import { type DragAreasOptions, useDragAreas } from './use-drag-areas'
import './window-drag-areas-view.css'

export interface WindowDragAreasViewProps {
  platform: WindowFramePlatform
  options?: DragAreasOptions
}

/**
 * The drag areas example: a window with its native title bar hidden, moved
 * and resized by the page alone. The coloured bar hands a press to
 * `startDragging()` (a double-click maximizes), the tinted frame's eight
 * handles hand it to `startResizing(edge)`, and the middle lets clicks
 * through. A HUD on the desktop shows the call each gesture made and the
 * event that came back.
 */
export function WindowDragAreasView({ platform, options }: WindowDragAreasViewProps) {
  const { w, frame, clicks, limited, log, actions } = useDragAreas(options)
  const measure = useRef<HTMLDivElement>(null)

  useLayoutEffect(() => {
    const el = measure.current
    if (!el) return
    const update = () => actions.setDesk({ width: Math.round(el.clientWidth), height: Math.round(el.clientHeight) })
    update()
    const observer = new ResizeObserver(update)
    observer.observe(el)
    return () => observer.disconnect()
    // The observer is set up once; setDesk reads the newest window itself.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [])

  const restoreOnMove = useRef(false)
  const move = usePointerDrag({
    onStart: () => {
      actions.beginMove()
      restoreOnMove.current = w.maximized
    },
    onMove: ({ dx, dy, x }) => {
      if (restoreOnMove.current) {
        // Dragging a maximized window restores it under the pointer.
        if (Math.abs(dx) + Math.abs(dy) < 6) return
        restoreOnMove.current = false
        const left = x - (measure.current?.getBoundingClientRect().left ?? 0) - w.width / 2 - dx
        actions.restoreUnder(left)
      }
      actions.move(dx, dy)
    },
    onEnd: actions.endMove,
  })

  return (
    <DesktopStage
      platform={platform}
      appName={WINDOW_TITLE}
      layout="free"
      overlay={
        <Card variant="raised" size="small" className="window-drag-areas-view__hud" onClick={e => e.stopPropagation()}>
          <EventBar lastEvent={log.lastEvent} log={log.log} onClear={log.clear} />
        </Card>
      }
    >
      <div ref={measure} className="window-drag-areas-view__measure" />
      {w.x >= 0 && (
        <DesktopWindow x={frame.x} y={frame.y}>
          <WindowFrame
            platform={platform}
            title={WINDOW_TITLE}
            titlebar={false}
            controls={false}
            width={frame.width}
            height={frame.height}
            className="window-drag-areas-view__window"
            data-maximized={w.maximized ? '' : undefined}
          >
            <ResizeHandles
              enabled={limited ? LIMITED_EDGES : null}
              onStart={actions.beginResize}
              onMove={actions.resize}
              onEnd={actions.endResize}
            />
            <div className="window-drag-areas-view__panel">
              <div
                className="window-drag-areas-view__bar"
                onPointerDown={move}
                onDoubleClick={actions.toggleMaximize}
              >
                <Icon icon={ArrowMove20Regular} className="window-drag-areas-view__bar-icon" />
                <span>Drag here to move</span>
              </div>
              <div className="window-drag-areas-view__body">
                <strong className="window-drag-areas-view__size">
                  Size: {frame.width} x {frame.height}
                </strong>
                <p className="window-drag-areas-view__lead">
                  Double-click the bar to maximize, drag the tinted frame to resize.
                </p>
                <div className="window-drag-areas-view__cards">
                  <Card variant="sunken" size="small" className="window-drag-areas-view__card">
                    <SectionLabel>Pass-through</SectionLabel>
                    <strong className="window-drag-areas-view__value">Clicks: {clicks}</strong>
                    <span className="window-drag-areas-view__note">The middle of the resize area lets clicks through.</span>
                    <Button size="small" variant="filled" onClick={actions.click}>
                      +1
                    </Button>
                  </Card>
                  <Card variant="sunken" size="small" className="window-drag-areas-view__card">
                    <SectionLabel>Resize handles</SectionLabel>
                    <strong className="window-drag-areas-view__value">{limited ? '3 of 8 handles' : '8 of 8 handles'}</strong>
                    <span className="window-drag-areas-view__note">enableResizeEdges: every handle, or only three.</span>
                    <Button size="small" variant="normal" tint="neutral" onClick={actions.toggleEdges}>
                      {limited ? 'Edges: right and bottom' : 'Edges: all'}
                    </Button>
                  </Card>
                </div>
              </div>
            </div>
          </WindowFrame>
        </DesktopWindow>
      )}
    </DesktopStage>
  )
}
