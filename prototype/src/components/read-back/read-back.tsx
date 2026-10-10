import type { CSSProperties, ReactNode } from 'react'

import { Card, SectionLabel } from '@dazzlabs/dazzui'

import './read-back.css'

export interface ReadBackProps {
  /** The card's label; "Read back from getters" by default. */
  label?: ReactNode
  /** Trailing the label: counters, a refresh button. */
  action?: ReactNode
  /** What the getters return, as `name` → value. Never what was written. */
  rows: readonly (readonly [string, ReactNode])[]
  columns?: number
  /** The width of the names' column. */
  keyWidth?: string
}

/**
 * What the native getters return after the calls, in code type: the proof
 * that a setter took, rather than an echo of what the UI asked for.
 */
export function ReadBack({ label = 'Read back from getters', action, rows, columns = 2, keyWidth }: ReadBackProps) {
  return (
    <Card variant="outlined" size="small" className="read-back">
      <div className="read-back__head">
        <SectionLabel>{label}</SectionLabel>
        {action}
      </div>
      <dl
        className="read-back__grid"
        style={{ '--read-back-columns': columns, '--read-back-key-width': keyWidth } as CSSProperties}
      >
        {rows.map(([name, value]) => (
          <div key={name}>
            <dt>{name}</dt>
            <dd>{value}</dd>
          </div>
        ))}
      </dl>
    </Card>
  )
}
