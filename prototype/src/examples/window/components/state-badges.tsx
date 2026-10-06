import { Badge, type Tint } from '@dazzlabs/dazzui'

import type { SimWindow } from '../types'
import './panels.css'

/** The window's boolean state, one badge each: tinted while on, outlined while off. */
export function StateBadges({ w, focused }: { w: SimWindow; focused: boolean }) {
  const badge = (label: string, on: boolean, tint: Tint) => (
    <Badge key={label} size="small" variant={on ? 'tinted' : 'outlined'} tint={on ? tint : 'neutral'}>
      {label}
    </Badge>
  )
  return (
    <div className="window-panel__badges">
      {badge('Visible', w.visible, 'success')}
      {badge('Focused', focused, 'primary')}
      {badge('Maximized', w.maximized, 'info')}
      {badge('Minimized', w.minimized, 'warning')}
      {badge('Full screen', w.fullScreen, 'danger')}
      {badge('On top', w.alwaysOnTop, 'info')}
    </div>
  )
}
