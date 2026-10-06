import type { CSSProperties, ReactNode } from 'react'

import { cx } from '@dazzlabs/dazzui'

import type { ShapeLook } from '../types'
import './shape-art.css'

export interface ShapeArtProps {
  look: ShapeLook
  className?: string
  style?: CSSProperties
  children?: ReactNode
}

/** A silhouette's picture: its gradient, two soft discs, a ring and a dot grid, all plain CSS. */
export function ShapeArt({ look, className, style, children }: ShapeArtProps) {
  const [a, b, c] = look.colors
  return (
    <div
      className={cx('shape-art', className)}
      style={{ ...style, '--shape-a': a, '--shape-b': b, '--shape-c': c } as CSSProperties}
    >
      {children}
    </div>
  )
}
