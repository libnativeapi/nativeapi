import { contentOf, headingOf } from './sign-data'
import type { SignEntry } from './sign-types'

let context: CanvasRenderingContext2D | null = null

function measure(text: string, size: number) {
  context ??= typeof document === 'undefined' ? null : document.createElement('canvas').getContext('2d')
  if (!context) return [...text].length * size
  const font = getComputedStyle(document.documentElement).getPropertyValue('--base-font-ui').trim() || 'sans-serif'
  context.font = `700 ${size}px ${font}`
  return context.measureText(text).width
}

/** A 100-unit-high face, shared by rendering and the simulated bounds getter. */
export function signLayoutOf(entry: SignEntry) {
  const content = contentOf(entry)
  const bilingual = entry.style === 'missing' || entry.style === 'travel' || entry.english
  const layout = {
    bilingual, main: bilingual ? 35 : 46, sub: 18.5,
    width: 0, body: 0, side: 0, caption: '',
    edge: 100 / 26, faceHeight: 100 - 200 / 26, badgeHeight: 72, inset: 0, badge: 0,
  }
  if (entry.style === 'missing') {
    const pinyin = content.secondary.split(/[ -]+/).map(word => word ? word[0]!.toUpperCase() + word.slice(1).toLowerCase() : '').join('')
    layout.caption = `WoZai${pinyin}HenXiangNi`
    layout.side = 35
    layout.body = Math.max(measure(headingOf(entry), layout.main), bilingual ? measure(layout.caption, layout.sub) : 0) + 12
    layout.width = Math.ceil(layout.body + layout.side * 2)
  } else if (entry.style === 'travel') {
    layout.main = 28
    layout.sub = 26
    layout.side = 60
    layout.body = Math.max(measure(content.primary, layout.main), measure(content.secondary, layout.sub)) + 30
    layout.width = Math.ceil(layout.body + layout.side)
  } else {
    layout.inset = (layout.faceHeight - layout.badgeHeight) / 2
    layout.badge = entry.style === 'guide' ? layout.badgeHeight + layout.inset : 0
    layout.side = entry.style === 'welcome' ? 0 : 70
    layout.caption = entry.style === 'welcome' ? `WELCOME TO ${content.secondary.toUpperCase()}` : `${content.secondary.toUpperCase()}  /  ${content.distance}`
    layout.main *= entry.style === 'welcome' ? 1.12 : 1.1
    layout.body = Math.max(measure(headingOf(entry), layout.main), bilingual ? measure(layout.caption, layout.sub) : 0) + 44
    layout.width = Math.ceil(layout.badge + layout.side + layout.body + layout.edge * 2)
  }
  return layout
}
