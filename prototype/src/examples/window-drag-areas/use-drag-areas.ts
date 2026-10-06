import { useMemo, useRef, useState } from 'react'

import { useEventLog } from '../../components/event-bar'
import { CONTENT_SIZE, MINIMUM_SIZE, WINDOW_TITLE } from './data'
import type { DragAreasWindow, ResizeEdge } from './types'

export interface DragAreasOptions {
  /** Only the right and bottom handles, as after pressing Edges. */
  limited?: boolean
  maximized?: boolean
}

export interface Desk {
  width: number
  height: number
}

const sizeText = (w: { width: number; height: number }) => `${w.width} × ${w.height}`

/**
 * The prototype's stand-in for the example's window: the move and the
 * resizes the window manager runs once the page has called
 * `startDragging()` or `startResizing(edge)`, honouring the minimum size,
 * and the events that come back when the gesture ends.
 */
export function useDragAreas(options: DragAreasOptions = {}) {
  const log = useEventLog(
    [
      `setTitle("${WINDOW_TITLE}")`,
      'setTitleBarStyle(TitleBarStyle.hidden)',
      `setMinimumSize(${MINIMUM_SIZE.width}×${MINIMUM_SIZE.height})`,
      `setContentSize(${CONTENT_SIZE.width}×${CONTENT_SIZE.height})`,
      'center()',
    ],
    'Drag the bar or the tinted frame',
  )
  const [desk, setDesk] = useState<Desk>({ width: 1320, height: 720 })
  const [w, setW] = useState<DragAreasWindow>({ x: -1, y: -1, ...CONTENT_SIZE, maximized: options.maximized ?? false })
  const [clicks, setClicks] = useState(0)
  const [limited, setLimited] = useState(options.limited ?? false)
  const ref = useRef(w)
  ref.current = w
  /** The frame a gesture started from. */
  const start = useRef(w)

  const frame = w.maximized ? { x: 0, y: 0, width: desk.width, height: desk.height } : w

  const actions = useMemo(
    () => ({
      setDesk: (next: Desk) => {
        setDesk(next)
        if (ref.current.x < 0) {
          setW(c => ({
            ...c,
            x: Math.round((next.width - c.width) / 2),
            y: Math.max(0, Math.round((next.height - c.height) / 2)),
          }))
        }
      },
      beginMove: () => {
        start.current = ref.current
        log.call('startDragging()')
      },
      /** A maximized window restores under the pointer once it is dragged. */
      restoreUnder: (left: number) => {
        start.current = { ...ref.current, maximized: false, x: left, y: 0 }
        log.event('Restored by dragging', 'WindowRestoredEvent')
      },
      move: (dx: number, dy: number) =>
        setW(() => ({
          ...start.current,
          maximized: false,
          x: Math.round(Math.min(Math.max(start.current.x + dx, 80 - start.current.width), desk.width - 80)),
          y: Math.round(Math.min(Math.max(start.current.y + dy, 0), desk.height - 40)),
        })),
      endMove: () => {
        const c = ref.current
        if (c.x !== start.current.x || c.y !== start.current.y) log.event(`Moved to ${c.x}, ${c.y}`, `WindowMovedEvent → ${c.x}, ${c.y}`)
      },
      beginResize: (edge: ResizeEdge) => {
        const c = ref.current
        // A maximized window resizes from the frame it fills.
        start.current = c.maximized ? { x: 0, y: 0, width: desk.width, height: desk.height, maximized: false } : c
        log.call(`startResizing(ResizeEdge.${edge})`)
      },
      resize: (edge: ResizeEdge, dx: number, dy: number) => {
        const s = start.current
        const west = edge === 'left' || edge === 'topLeft' || edge === 'bottomLeft'
        const north = edge === 'top' || edge === 'topLeft' || edge === 'topRight'
        const east = edge === 'right' || edge === 'topRight' || edge === 'bottomRight'
        const south = edge === 'bottom' || edge === 'bottomLeft' || edge === 'bottomRight'
        // The window manager keeps the minimum size; the opposite edge stays.
        const width = Math.round(Math.min(Math.max(s.width + (east ? dx : west ? -dx : 0), MINIMUM_SIZE.width), desk.width))
        const height = Math.round(Math.min(Math.max(s.height + (south ? dy : north ? -dy : 0), MINIMUM_SIZE.height), desk.height))
        setW({
          maximized: false,
          width,
          height,
          x: west ? s.x + s.width - width : s.x,
          y: north ? Math.max(0, s.y + s.height - height) : s.y,
        })
      },
      endResize: () => {
        const c = ref.current
        if (c.width !== start.current.width || c.height !== start.current.height) {
          log.event(`Resized to ${sizeText(c)}`, `WindowResizedEvent → ${sizeText(c)}`)
        }
      },
      toggleMaximize: () => {
        const maximized = !ref.current.maximized
        log.call(maximized ? 'maximize()' : 'unmaximize()')
        log.event(maximized ? 'Maximized' : 'Restored', maximized ? 'WindowMaximizedEvent' : 'WindowRestoredEvent')
        setW(c => ({ ...c, maximized }))
      },
      click: () => {
        setClicks(n => n + 1)
        log.event('Clicked through the resize area', 'Pass-through button pressed')
      },
      toggleEdges: () => {
        log.call(`enableResizeEdges: ${limited ? 'all' : '[right, bottom, bottomRight]'}`)
        setLimited(!limited)
      },
    }),
    [desk, limited, log],
  )

  return { w, frame, desk, clicks, limited, log, actions }
}
