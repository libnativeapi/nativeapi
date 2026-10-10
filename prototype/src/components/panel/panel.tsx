import type { CSSProperties, ReactNode } from 'react'

import { cx, Preferences } from '@dazzlabs/dazzui'

import './panel.css'

/** An example's tab: a padded column the pane scrolls. */
export function Panel({ className, style, children }: { className?: string; style?: CSSProperties; children?: ReactNode }) {
  return (
    <div className={cx('example-panel', className)} style={style}>
      {children}
    </div>
  )
}

/** DazzUI's `Preferences`, spaced for a tool window; `dense` for a list to scan. */
export function PanelPreferences({ dense, children }: { dense?: boolean; children?: ReactNode }) {
  return (
    <Preferences className={cx('example-panel__preferences', dense && 'example-panel__preferences--dense')}>
      {children}
    </Preferences>
  )
}

/** A value a row reports, in code type. */
export function PanelValue({ children }: { children?: ReactNode }) {
  return <span className="example-panel__value">{children}</span>
}

/** Equal tiles in a row: `columns` of them. */
export function PanelTiles({ columns = 4, children }: { columns?: number; children?: ReactNode }) {
  return (
    <div className="example-panel__tiles" style={{ '--example-tiles': columns } as CSSProperties}>
      {children}
    </div>
  )
}
