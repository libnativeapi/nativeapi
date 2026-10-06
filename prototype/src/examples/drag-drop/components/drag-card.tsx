import type { FluentIcon } from '@fluentui/react-icons'
import type { PointerEvent } from 'react'

import { Card, cx, Icon } from '@dazzlabs/dazzui'

import './drag-card.css'

export interface DragCardProps {
  icon: FluentIcon
  title: string
  subtitle: string
  /** This card's drag is in progress. */
  dragging?: boolean
  onPointerDown: (event: PointerEvent) => void
}

/** A `DragOutArea`: a raised card with a grab cursor that starts a drag out of the window. */
export function DragCard({ icon, title, subtitle, dragging, onPointerDown }: DragCardProps) {
  return (
    <Card
      variant="raised"
      size="small"
      className={cx('drag-card', dragging && 'drag-card--dragging')}
      onPointerDown={onPointerDown}
    >
      <Icon icon={icon} className="drag-card__icon" />
      <span className="drag-card__text">
        <strong>{title}</strong>
        <span>{subtitle}</span>
      </span>
    </Card>
  )
}
