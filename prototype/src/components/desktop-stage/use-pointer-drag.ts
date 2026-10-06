import { type PointerEvent as ReactPointerEvent, useCallback, useRef } from 'react'

export interface PointerDrag {
  /** Pixels moved since the press. */
  dx: number
  dy: number
  /** Where the pointer is, in client coordinates. */
  x: number
  y: number
}

export interface PointerDragHandlers {
  onStart?: (event: ReactPointerEvent) => void
  onMove: (drag: PointerDrag) => void
  onEnd?: (drag: PointerDrag) => void
}

/**
 * A press-and-drag on an element, as the window manager runs one after
 * `startDragging` or `startResizing`: the pointer is captured, and every move
 * reports how far it has come since the press. Returns the `onPointerDown`
 * to put on the handle.
 */
export function usePointerDrag({ onStart, onMove, onEnd }: PointerDragHandlers) {
  const handlers = useRef({ onStart, onMove, onEnd })
  handlers.current = { onStart, onMove, onEnd }

  return useCallback((event: ReactPointerEvent) => {
    if (event.button !== 0) return
    event.preventDefault()
    event.stopPropagation()
    const startX = event.clientX
    const startY = event.clientY
    const target = event.currentTarget as HTMLElement
    target.setPointerCapture(event.pointerId)
    handlers.current.onStart?.(event)
    const at = (e: PointerEvent): PointerDrag => ({ dx: e.clientX - startX, dy: e.clientY - startY, x: e.clientX, y: e.clientY })
    const move = (e: PointerEvent) => handlers.current.onMove(at(e))
    const up = (e: PointerEvent) => {
      target.removeEventListener('pointermove', move)
      target.removeEventListener('pointerup', up)
      target.removeEventListener('pointercancel', up)
      handlers.current.onEnd?.(at(e))
    }
    target.addEventListener('pointermove', move)
    target.addEventListener('pointerup', up)
    target.addEventListener('pointercancel', up)
  }, [])
}
