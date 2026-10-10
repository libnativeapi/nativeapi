import type { ReactNode } from 'react'

import { contentOf, headingOf } from '../sign-data'
import { signLayoutOf } from '../sign-layout'
import type { SignEntry } from '../sign-types'
import './sign-art.css'

/** The same proportions at menu-bar, embedded and enlarged-preview sizes.
 * Width follows measured text; none of the lettering is stretched or clipped.
 * Geometry mirrors the native example, in a normalized 100-unit-high face. */
export function SignArt({ entry, size = 'preview' }: { entry: SignEntry; size?: 'tray' | 'preview' | 'large' | 'tile' }) {
  const content = contentOf(entry)
  const { bilingual, main, sub, width, body, side, caption, edge, faceHeight, badgeHeight, inset, badge } = signLayoutOf(entry)
  const light = 'var(--sign-light)', ink = 'var(--sign-ink)'
  const color = entry.style === 'guide' ? 'var(--sign-brown)' :
    entry.style === 'travel' || (entry.style === 'missing' && entry.green) ? 'var(--sign-green)' : 'var(--sign-blue)'
  const text = (label: string, x: number, y: number, fontSize: number, fill = light) => (
    <text x={x} y={y} fontSize={fontSize} fill={fill} textAnchor="middle" dominantBaseline="central">{label}</text>
  )
  const rect = (x: number, y: number, width: number, height: number, fill: string) => <rect x={x} y={y} width={width} height={height} fill={fill} />
  let drawing: ReactNode

  if (entry.style === 'missing') {
    const top = bilingual ? 68 : 100
    drawing = <>
      {rect(0, 0, width, 100, light)}
      {rect(0, 0, width, top, color)}
      {text(headingOf(entry), side + body / 2, top / 2, main)}
      {text(entry.right ? '东' : '西', side / 2, top / 2, main * 0.66)}
      {text(entry.right ? '西' : '东', width - side / 2, top / 2, main * 0.66)}
      {bilingual && <>
        {text(caption, side + body / 2, 84, sub, ink)}
        {text(entry.right ? 'E' : 'W', side / 2, 84, sub, ink)}
        {text(entry.right ? 'W' : 'E', width - side / 2, 84, sub, ink)}
      </>}
    </>
  } else if (entry.style === 'travel') {
    drawing = <>
      {rect(0, 0, width, 100, light)}
      {rect(width * 0.47, 0, 12, 100, 'var(--sign-brown)')}
      {rect(0, 0, width, 47, color)}
      {rect(0, 53, width, 47, 'var(--sign-ochre)')}
      {text(content.primary, body / 2, 23.5, 28)}
      {text(entry.right ? '➜' : '←', body + side / 2, 23.5, 31)}
      {text(entry.right ? '←' : '➜', side / 2, 76.5, 31, ink)}
      {text(content.secondary, side + body / 2, 76.5, 26, ink)}
    </>
  } else {
    const arrow = side
    const start = badge + (entry.right ? 0 : arrow) + edge
    drawing = <>
      {rect(0, 0, width, 100, light)}
      {rect(edge, edge, width - edge * 2, faceHeight, color)}
      {text(headingOf(entry), start + body / 2, bilingual ? 35 : 50, main)}
      {bilingual && text(caption, start + body / 2, 79.5, sub)}
      {arrow > 0 && text(entry.right ? '➜' : '←', entry.right ? width - edge - arrow / 2 : edge + badge + arrow / 2, 46, 50)}
      {badge > 0 && <>
        {rect(edge + inset, edge + inset, badgeHeight, badgeHeight, light)}
        {text('▲', edge + inset + badgeHeight / 2, 50, 34, color)}
      </>}
    </>
  }

  return (
    <svg className={`sign-art sign-art--${size}`} viewBox={`0 0 ${width} 100`} width={width} height={100}
      role="img" aria-label={`${headingOf(entry)} · ${content.secondary}${content.distance ? ` · ${content.distance}` : ''}`}
      data-sign-style={entry.style}>
      {drawing}
    </svg>
  )
}
