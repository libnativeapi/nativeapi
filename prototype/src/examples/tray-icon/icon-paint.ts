import type { IconAnimation, StillIcon } from './types'

/** Progress of the Progress animation at time `t`, 0..1; the Download scene's title reads it. */
export const progressAt = (t: number) => (t % 4) / 4

/**
 * Draws one frame of `animation` at `t` seconds into a `size` × `size` box —
 * the same drawings as `IconAnimation.paint` in the Flutter example.
 */
export function paintAnimation(ctx: CanvasRenderingContext2D, animation: IconAnimation, size: number, t: number, color: string) {
  const c = size / 2
  const stroke = size * 0.13
  const radius = size / 2 - stroke / 2 - size * 0.04
  ctx.strokeStyle = color
  ctx.fillStyle = color
  ctx.lineWidth = stroke
  ctx.lineCap = 'round'
  ctx.lineJoin = 'round'
  const faintRing = () => {
    ctx.save()
    ctx.globalAlpha = 0.22
    ctx.beginPath()
    ctx.arc(c, c, radius, 0, Math.PI * 2)
    ctx.stroke()
    ctx.restore()
  }

  switch (animation) {
    case 'spinner': {
      faintRing()
      const start = t * 2 * Math.PI
      ctx.beginPath()
      ctx.arc(c, c, radius, start, start + Math.PI * 0.6)
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
      faintRing()
      ctx.beginPath()
      ctx.arc(c, c, radius, -Math.PI / 2, -Math.PI / 2 + 2 * Math.PI * progressAt(t))
      ctx.stroke()
      break
    case 'wave': {
      const bars = 4
      const gap = size / (bars * 2 + 1)
      for (let i = 0; i < bars; i++) {
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
    case 'clock': {
      // Real data, not a loop: the hands show the wall clock.
      const now = new Date()
      const seconds = now.getSeconds() + now.getMilliseconds() / 1000
      const minutes = now.getMinutes() + seconds / 60
      const hours = (now.getHours() % 12) + minutes / 60
      ctx.lineWidth = stroke * 0.8
      ctx.beginPath()
      ctx.arc(c, c, radius, 0, Math.PI * 2)
      ctx.stroke()
      const hand = (turns: number, length: number, width: number) => {
        const a = turns * 2 * Math.PI - Math.PI / 2
        ctx.lineWidth = width
        ctx.beginPath()
        ctx.moveTo(c, c)
        ctx.lineTo(c + Math.cos(a) * radius * length, c + Math.sin(a) * radius * length)
        ctx.stroke()
      }
      hand(hours / 12, 0.45, stroke * 0.8)
      hand(minutes / 60, 0.7, stroke * 0.8)
      hand(seconds / 60, 0.8, stroke * 0.4)
      break
    }
    case 'widget': {
      // The Flutter example screenshots a live widget (a ring and a digit that
      // counts the seconds and bounces); here it is drawn the same way.
      const phase = t % 1
      const bounce = phase < 0.25 ? Math.sin((phase / 0.25) * Math.PI) : 0
      ctx.save()
      ctx.translate(c, c)
      ctx.scale(0.84 + 0.16 * bounce, 0.84 + 0.16 * bounce)
      ctx.lineWidth = size * 0.1
      ctx.beginPath()
      ctx.arc(0, 0, size / 2 - size * 0.05, 0, Math.PI * 2)
      ctx.stroke()
      ctx.font = `700 ${size * 0.58}px system-ui, sans-serif`
      ctx.textAlign = 'center'
      ctx.textBaseline = 'middle'
      ctx.fillText(String(Math.floor(t) % 10), 0, size * 0.04)
      ctx.restore()
      break
    }
  }
}

/** The 8 × 8 bitmap behind the Base64 icon: a decoded image, drawn pixel for pixel. */
const BASE64_BITMAP = ['01100110', '11111111', '11111111', '11111111', '01111110', '00111100', '00011000', '00000000']

/** Draws a still icon — the app's asset, a star drawn on a canvas, or a decoded bitmap. */
export function paintStill(ctx: CanvasRenderingContext2D, still: StillIcon, size: number, color: string) {
  ctx.fillStyle = color
  ctx.strokeStyle = color
  switch (still) {
    case 'asset': {
      // The example's asset: a rounded window with a title bar.
      const inset = size * 0.1
      ctx.lineWidth = size * 0.1
      ctx.beginPath()
      ctx.roundRect(inset, inset * 1.4, size - inset * 2, size - inset * 2.8, size * 0.14)
      ctx.stroke()
      ctx.fillRect(inset, inset * 1.4, size - inset * 2, size * 0.22)
      break
    }
    case 'drawn': {
      ctx.beginPath()
      for (let i = 0; i < 10; i++) {
        const r = (size / 2) * (i % 2 === 0 ? 0.95 : 0.42)
        const a = -Math.PI / 2 + (i * Math.PI) / 5
        const x = size / 2 + r * Math.cos(a)
        const y = size / 2 + r * Math.sin(a)
        if (i === 0) ctx.moveTo(x, y)
        else ctx.lineTo(x, y)
      }
      ctx.closePath()
      ctx.fill()
      break
    }
    case 'base64': {
      const cell = size / 8
      BASE64_BITMAP.forEach((row, y) =>
        [...row].forEach((bit, x) => {
          if (bit === '1') ctx.fillRect(Math.floor(x * cell), Math.floor(y * cell), Math.ceil(cell), Math.ceil(cell))
        }),
      )
      break
    }
  }
}
