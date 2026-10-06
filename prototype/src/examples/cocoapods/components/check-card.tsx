import { CheckmarkCircle20Filled, Circle20Regular, ErrorCircle20Filled } from '@fluentui/react-icons'

import { Card, Icon, Spinner } from '@dazzlabs/dazzui'

import type { Glyph } from '../data'
import type { CheckResult } from '../types'
import './check-card.css'

export interface CheckCardProps {
  name: string
  icon: Glyph
  result: CheckResult
  /** Shown while the check has not run: why not. */
  idleDetail?: string
  selected?: boolean
  /** Picks the card: its whole detail shows. */
  onPress?: () => void
}

/** One check: the module's icon, its name, and what the call returned or threw, in code type. */
export function CheckCard({ name, icon, result, idleDetail = 'Waiting…', selected, onPress }: CheckCardProps) {
  const status =
    result.status === 'running' ? (
      <Spinner size="small" />
    ) : result.status === 'pass' ? (
      <Icon icon={CheckmarkCircle20Filled} className="check-card__pass" />
    ) : result.status === 'fail' ? (
      <Icon icon={ErrorCircle20Filled} className="check-card__fail" />
    ) : (
      <Icon icon={Circle20Regular} className="check-card__idle" />
    )

  return (
    <Card
      variant={result.status === 'fail' ? 'tinted' : 'sunken'}
      tint={result.status === 'fail' ? 'danger' : undefined}
      size="small"
      className="check-card"
      data-status={result.status}
      data-selected={selected || undefined}
      onClick={onPress}
    >
      <span className="check-card__module">
        <Icon icon={icon} />
      </span>
      <span className="check-card__text">
        <span className="check-card__name">{name}</span>
        <span className="check-card__detail" title={result.detail || undefined}>
          {result.status === 'idle' ? idleDetail : result.status === 'running' ? 'Running…' : result.detail}
        </span>
      </span>
      <span className="check-card__status">{status}</span>
    </Card>
  )
}
