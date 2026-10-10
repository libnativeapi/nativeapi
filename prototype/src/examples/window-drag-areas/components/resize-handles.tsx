import { usePointerDrag } from '../../../components/desktop-stage'
import { EDGES } from '../data'
import type { ResizeEdge } from '../types'
import './resize-handles.css'

export interface ResizeHandlesProps {
  /** The handles `enableResizeEdges` leaves on; all eight when left out. */
  enabled: readonly ResizeEdge[] | null
  onStart: (edge: ResizeEdge) => void
  onMove: (edge: ResizeEdge, dx: number, dy: number) => void
  onEnd: () => void
}

/**
 * The page's resize frame: eight tinted bands, inset from the window's edge
 * so that it is clear the page resizes the window, not the native frame. A
 * press on one hands the gesture to `startResizing(edge)`; between them the
 * middle lets clicks through to the content.
 */
export function ResizeHandles({ enabled, onStart, onMove, onEnd }: ResizeHandlesProps) {
  return (
    <>
      {EDGES.filter(({ edge }) => !enabled || enabled.includes(edge)).map(({ edge, cursor }) => (
        <Handle key={edge} edge={edge} cursor={cursor} onStart={onStart} onMove={onMove} onEnd={onEnd} />
      ))}
    </>
  )
}

function Handle({
  edge,
  cursor,
  onStart,
  onMove,
  onEnd,
}: { edge: ResizeEdge; cursor: string } & Omit<ResizeHandlesProps, 'enabled'>) {
  const press = usePointerDrag({
    onStart: () => onStart(edge),
    onMove: ({ dx, dy }) => onMove(edge, dx, dy),
    onEnd,
  })
  return (
    <span
      className="resize-handles__handle"
      data-edge={edge}
      style={{ cursor }}
      aria-label={`Resize from ${edge}`}
      onPointerDown={press}
    />
  )
}
