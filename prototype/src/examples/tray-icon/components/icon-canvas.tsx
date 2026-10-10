import { type CSSProperties, useEffect, useRef } from 'react'

import { cx } from '@dazzlabs/dazzui'

import { COLORS } from '../data'
import { paintAnimation, paintStill } from '../icon-paint'
import type { IconAnimation, IconColor, StillIcon } from '../types'
import './icon-canvas.css'

export interface IconCanvasProps {
  animation: IconAnimation | null
  still: StillIcon | null
  /** The animation time to draw, in seconds; read on every display frame. */
  time: () => number
  /** The image's own pixels per side — what `TrayIcon.setIcon` is handed. */
  pixels: number
  /** The size it is shown at, in CSS pixels. */
  size: number
  color?: IconColor
  /** Shows the image's real pixels, enlarged without smoothing. */
  pixelated?: boolean
  className?: string
}

/**
 * One tray icon image, drawn the way the example's `IconAnimator` draws it:
 * on a canvas of the icon's own pixel size, every frame. The colour is the
 * element's `color`, so `auto` takes whatever ink surrounds it — the menu
 * bar's, the taskbar's, a card's.
 */
export function IconCanvas({ animation, still, time, pixels, size, color = 'auto', pixelated, className }: IconCanvasProps) {
  const canvas = useRef<HTMLCanvasElement>(null)
  const timeRef = useRef(time)
  timeRef.current = time

  useEffect(() => {
    const el = canvas.current
    const ctx = el?.getContext('2d')
    if (!el || !ctx) return
    let frame = 0
    let drawn = ''
    const draw = () => {
      const ink = getComputedStyle(el).color
      const t = timeRef.current()
      // A still image or a paused animation is drawn again only when it changes.
      const key = `${ink}|${animation === 'clock' ? Math.floor(Date.now() / 1000) : t}`
      if (key !== drawn) {
        drawn = key
        ctx.clearRect(0, 0, pixels, pixels)
        if (animation) paintAnimation(ctx, animation, pixels, t, ink)
        else if (still) paintStill(ctx, still, pixels, ink)
      }
      frame = requestAnimationFrame(draw)
    }
    draw()
    return () => cancelAnimationFrame(frame)
  }, [animation, still, pixels])

  const token = COLORS.find(c => c.value === color)?.token
  return (
    <canvas
      ref={canvas}
      width={pixels}
      height={pixels}
      aria-hidden
      className={cx('tray-icon-canvas', pixelated && 'tray-icon-canvas--pixelated', className)}
      style={{ width: size, height: size, ...(token ? { color: token } : {}) } as CSSProperties}
    />
  )
}
