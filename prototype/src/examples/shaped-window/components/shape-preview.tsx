import type { CSSProperties, PointerEvent as ReactPointerEvent } from 'react'

import { Button } from '@dazzlabs/dazzui'

import { LOOKS, SHADOW_COLORS } from '../data'
import { clipPathOf } from '../shape-geometry'
import type { DemoShape, Point, Shadow } from '../types'
import type { ShadowPaint } from '../use-shaped-window'
import { ShapeArt } from './shape-art'
import './shape-preview.css'

export interface ShapePreviewProps {
  shape: DemoShape
  /** The contour on screen this frame; ignored once the rectangle is restored. */
  points: readonly Point[]
  size: number
  restored: boolean
  shadow: Shadow
  paint: ShadowPaint
  count: number
  onTap: () => void
  /** A press on the handle: `startDragging()`. */
  onDragPress: (event: ReactPointerEvent) => void
}

/** The shadow's colour this frame, with its opacity folded in. */
function shadowColor({ opacity }: Shadow, { from, to, t }: ShadowPaint) {
  const hue = from === to ? SHADOW_COLORS[to] : `color-mix(in srgb, ${SHADOW_COLORS[to]} ${(t * 100).toFixed(1)}%, ${SHADOW_COLORS[from]})`
  return `color-mix(in srgb, ${hue} ${(opacity * 100).toFixed(1)}%, transparent)`
}

/**
 * The "Shape preview" window: no title bar, a transparent background, and
 * the content clipped to the polygon — outside it the window is not there,
 * neither to the eye nor to the pointer. Its shadow follows the contour.
 * The handle drags the window; everything else is the art and a counter.
 */
export function ShapePreview({ shape, points, size, restored, shadow, paint, count, onTap, onDragPress }: ShapePreviewProps) {
  const look = LOOKS[shape]
  const filter = shadow.enabled
    ? `drop-shadow(${shadow.x.toFixed(1)}px ${shadow.y.toFixed(1)}px ${(shadow.blur / 2).toFixed(1)}px ${shadowColor(shadow, paint)})`
    : 'none'
  return (
    <div className="shape-preview" style={{ filter, width: size, height: size }}>
      <ShapeArt
        look={look}
        className="shape-preview__art"
        style={{ clipPath: restored ? 'none' : clipPathOf(points), width: size, height: size } as CSSProperties}
      >
        <div className="shape-preview__stack">
          <button type="button" className="shape-preview__handle" onPointerDown={onDragPress}>
            ⠿&nbsp; DRAG ME
          </button>
          <span className="shape-preview__name">{restored ? 'rectangle' : shape}</span>
          <Button size="small" variant="normal" tint="neutral" onClick={onTap}>
            Tap · {count}
          </Button>
          <span className="shape-preview__look">{look.name}</span>
        </div>
      </ShapeArt>
    </div>
  )
}
