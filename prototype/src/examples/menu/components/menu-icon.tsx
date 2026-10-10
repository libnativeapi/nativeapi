import { Star16Filled } from '@fluentui/react-icons'
import { useEffect, useRef } from 'react'

import { Icon } from '@dazzlabs/dazzui'

import { nowSeconds } from '../../../components/use-now'
import type { IconAnimation, ItemIcon } from '../types'
import './menu-icon.css'

/**
 * One frame of `AnimatedIconGenerator`'s animations at `t` seconds, drawn the
 * way it draws them into a 32 px image: a spinner, a pulse, a blink, a
 * progress ring, a wave of bars, a turning square.
 */
function paint(ctx: CanvasRenderingContext2D, kind: IconAnimation, size: number, t: number, color: string) {
  const c = size / 2
  const stroke = size * 0.13
  const r = size / 2 - stroke / 2 - size * 0.04
  ctx.clearRect(0, 0, size, size)
  ctx.strokeStyle = color
  ctx.fillStyle = color
  ctx.lineWidth = stroke
  ctx.lineCap = 'round'
  const ring = () => {
    ctx.save()
    ctx.globalAlpha = 0.22
    ctx.beginPath()
    ctx.arc(c, c, r, 0, Math.PI * 2)
    ctx.stroke()
    ctx.restore()
  }
  switch (kind) {
    case 'spinner': {
      ring()
      const start = t * 2 * Math.PI
      ctx.beginPath()
      ctx.arc(c, c, r, start, start + Math.PI * 0.6)
      ctx.stroke()
      break
    }
    case 'pulse': {
      const k = 0.5 - 0.5 * Math.cos((t * 2 * Math.PI) / 1.2)
      ctx.beginPath()
      ctx.arc(c, c, (size / 2 - 1) * (0.4 + 0.6 * k), 0, Math.PI * 2)
      ctx.fill()
      break
    }
    case 'blink':
      if (t % 1 < 0.5) {
        ctx.beginPath()
        ctx.arc(c, c, size * 0.36, 0, Math.PI * 2)
        ctx.fill()
      }
      break
    case 'progress':
      ring()
      ctx.beginPath()
      ctx.arc(c, c, r, -Math.PI / 2, -Math.PI / 2 + 2 * Math.PI * ((t % 4) / 4))
      ctx.stroke()
      break
    case 'wave': {
      const gap = size / 9
      for (let i = 0; i < 4; i++) {
        const k = 0.5 + 0.5 * Math.sin((t * 2 * Math.PI) / 0.9 - i * 0.9)
        const h = size * (0.25 + 0.6 * k)
        ctx.beginPath()
        ctx.roundRect(gap * (1 + i * 2), (size - h) / 2, gap, h, gap / 2)
        ctx.fill()
      }
      break
    }
    case 'rotate': {
      ctx.save()
      ctx.translate(c, c)
      ctx.rotate((t * 2 * Math.PI) / 2.4)
      const side = size * 0.56
      ctx.beginPath()
      ctx.roundRect(-side / 2, -side / 2, side, side, size * 0.08)
      ctx.stroke()
      ctx.restore()
      break
    }
  }
}

/** An animated icon: a new frame every 100 ms, as the generator's timer hands them to `setIcon`. */
function AnimatedIcon({ kind, startedAt }: { kind: IconAnimation; startedAt: number }) {
  const canvas = useRef<HTMLCanvasElement>(null)
  useEffect(() => {
    const el = canvas.current
    const ctx = el?.getContext('2d')
    if (!el || !ctx) return
    const draw = () => paint(ctx, kind, 32, nowSeconds() - startedAt, getComputedStyle(el).color)
    draw()
    const id = window.setInterval(draw, 100)
    return () => clearInterval(id)
  }, [kind, startedAt])
  return <canvas ref={canvas} width={32} height={32} className="menu-icon menu-icon--animated" aria-hidden />
}

/** The example's asset, `images/flutter_logo.png`: drawn as the logo's two strokes. */
function AssetIcon() {
  return (
    <svg viewBox="0 0 16 16" className="menu-icon menu-icon--asset" aria-hidden>
      <path d="M9.5 1 2.5 8l2.1 2.1L13.7 1Z" />
      <path d="M9.5 7.6 5.9 11.2l3.6 3.8h4.2l-3.7-3.8 3.7-3.6Z" className="menu-icon__shade" />
    </svg>
  )
}

export interface MenuIconProps {
  icon: ItemIcon | null
  animation: { kind: IconAnimation; startedAt: number } | null
}

/** What `MenuItem.getIcon()` shows: an animation frame, the asset, the widget's star — or nothing. */
export function MenuIcon({ icon, animation }: MenuIconProps) {
  if (animation) return <AnimatedIcon kind={animation.kind} startedAt={animation.startedAt} />
  if (icon === 'asset') return <AssetIcon />
  if (icon === 'widget') return <Icon icon={Star16Filled} size={16} className="menu-icon menu-icon--widget" />
  return null
}
