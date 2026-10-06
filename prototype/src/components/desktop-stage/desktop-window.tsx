import type { CSSProperties, ReactNode } from 'react'

import { cx } from '@dazzlabs/dazzui'

export interface DesktopWindowProps {
  /** The window's frame on the desktop, in pixels from the desktop's top-left. */
  x: number
  y: number
  /** Stacking order: higher is in front. */
  z?: number
  hidden?: boolean
  /** A press anywhere in the window: the window manager brings it forward. */
  onPress?: () => void
  className?: string
  style?: CSSProperties
  children?: ReactNode
}

/**
 * A window standing where it was put on a `free` `DesktopStage`. The frame
 * inside it — a `WindowFrame` or an `ExampleWindow` — draws itself; this only
 * places it, and keeps its presses from reaching the desktop.
 */
export function DesktopWindow({ x, y, z = 1, hidden, onPress, className, style, children }: DesktopWindowProps) {
  if (hidden) return null
  return (
    <div
      className={cx('desktop-stage__placed', className)}
      style={{ ...style, left: x, top: y, zIndex: z }}
      onPointerDownCapture={onPress}
      onClick={event => event.stopPropagation()}
    >
      {children}
    </div>
  )
}
