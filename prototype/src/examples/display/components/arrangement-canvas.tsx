import { Desktop20Regular, Laptop20Regular } from '@fluentui/react-icons'
import { type PointerEvent, useEffect, useRef, useState } from 'react'

import { Badge, cx, Icon } from '@dazzlabs/dazzui'

import { scaleText } from '../use-displays'
import type { PlacedDisplay, Point, Rect } from '../types'
import './arrangement-canvas.css'

export interface ArrangementCanvasProps {
  displays: readonly PlacedDisplay[]
  selectedId: number
  onSelect: (id: number) => void
  /** This window's frame in desktop coordinates; null where the platform cannot say. */
  windowRect: Rect | null
  windowTitle: string
  /** `getCursorPosition()`; null where the platform cannot say. */
  cursor: Point | null
  /** The pointer is over a display on the canvas: where that is on the desktop. */
  onPointer: (point: Point) => void
}

/** A hair between neighbouring displays, so their edges stay apart. */
const GAP = 1.5

/**
 * The subject of the example: every display in desktop coordinates, drawn to
 * scale — the bezel where the system's bars are, the work area inside it —
 * with this window and the cursor laid over them, live. The pointer over the
 * canvas stands for the cursor on that display.
 */
export function ArrangementCanvas({
  displays,
  selectedId,
  onSelect,
  windowRect,
  windowTitle,
  cursor,
  onPointer,
}: ArrangementCanvasProps) {
  const box = useRef<HTMLDivElement>(null)
  const [size, setSize] = useState({ width: 0, height: 0 })
  useEffect(() => {
    const el = box.current
    if (!el) return
    const observer = new ResizeObserver(([entry]) => {
      const { width, height } = entry!.contentRect
      setSize({ width, height })
    })
    observer.observe(el)
    return () => observer.disconnect()
  }, [])

  const minX = Math.min(...displays.map(d => d.x))
  const minY = Math.min(...displays.map(d => d.y))
  const maxX = Math.max(...displays.map(d => d.x + d.width))
  const maxY = Math.max(...displays.map(d => d.y + d.height))
  const scale = Math.min(size.width / (maxX - minX), size.height / (maxY - minY)) * 0.9 || 0
  const ox = (size.width - (maxX - minX) * scale) / 2
  const oy = (size.height - (maxY - minY) * scale) / 2
  const toCanvas = (x: number, y: number) => ({ left: ox + (x - minX) * scale, top: oy + (y - minY) * scale })

  const move = (event: PointerEvent) => {
    const rect = box.current!.getBoundingClientRect()
    const x = minX + (event.clientX - rect.left - ox) / scale
    const y = minY + (event.clientY - rect.top - oy) / scale
    // The cursor never leaves the displays.
    if (displays.some(d => x >= d.x && x < d.x + d.width && y >= d.y && y < d.y + d.height)) {
      onPointer({ x: Math.round(x), y: Math.round(y) })
    }
  }

  return (
    <div ref={box} className="display-canvas" onPointerMove={move}>
      {scale > 0 &&
        displays.map(d => {
          const at = toCanvas(d.x, d.y)
          const width = d.width * scale - GAP * 2
          const height = d.height * scale - GAP * 2
          // The display clips it: the gap comes off the work area's outer edges.
          const work = {
            left: (d.workArea.x - d.x) * scale,
            top: (d.workArea.y - d.y) * scale,
            width: d.workArea.width * scale,
            height: d.workArea.height * scale,
          }
          const selected = d.id === selectedId
          return (
            <button
              key={d.id}
              type="button"
              className={cx('display-canvas__display', selected && 'display-canvas__display--selected')}
              style={{ left: at.left + GAP, top: at.top + GAP, width, height }}
              aria-label={`Select ${d.name}`}
              onClick={() => onSelect(d.id)}
            >
              <span className="display-canvas__work" style={work}>
                <span className="display-canvas__label">
                  {height > 70 && <Icon icon={d.name.startsWith('Built-in') ? Laptop20Regular : Desktop20Regular} />}
                  <span className="display-canvas__name">{d.name}</span>
                  <span className="display-canvas__size">
                    {d.width}×{d.height} @{scaleText(d.scale)}x
                  </span>
                  {d.primary && height > 96 && (
                    <Badge size="small" variant="tinted" tint="success">
                      Primary
                    </Badge>
                  )}
                </span>
              </span>
            </button>
          )
        })}
      {scale > 0 && windowRect && (
        <div
          className="display-canvas__window"
          style={{ ...toCanvas(windowRect.x, windowRect.y), width: windowRect.width * scale, height: windowRect.height * scale }}
        >
          <span className="display-canvas__window-title">{windowTitle}</span>
        </div>
      )}
      {scale > 0 && cursor && <span className="display-canvas__cursor" style={toCanvas(cursor.x, cursor.y)} />}
    </div>
  )
}

/** What the marks on the canvas mean. */
export function ArrangementLegend({ strips, cursor }: { strips: string; cursor: boolean }) {
  return (
    <div className="display-canvas-legend">
      <span>
        <i className="display-canvas-legend__work" />
        Work area
      </span>
      <span>
        <i className="display-canvas-legend__bezel" />
        {strips}
      </span>
      <span>
        <i className="display-canvas-legend__window" />
        This window
      </span>
      {cursor && (
        <span>
          <i className="display-canvas-legend__cursor" />
          Cursor
        </span>
      )}
      <span className="display-canvas-legend__hint">Click a display to inspect it</span>
    </div>
  )
}
