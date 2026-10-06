import { cx } from '@dazzlabs/dazzui'

import { PLACEMENT_NAMES } from '../data'
import type { Placement } from '../types'
import './placement-pad.css'

export interface PlacementPadProps {
  value: Placement
  onChange: (value: Placement) => void
}

/** The pad's cells, row by row: each placement sits on the side of the point it puts the menu. */
const GRID: readonly (Placement | 'point' | null)[] = [
  null,
  'topStart',
  'top',
  'topEnd',
  null,
  'leftStart',
  null,
  null,
  null,
  'rightStart',
  'left',
  null,
  'point',
  null,
  'right',
  'leftEnd',
  null,
  null,
  null,
  'rightEnd',
  null,
  'bottomStart',
  'bottom',
  'bottomEnd',
  null,
]

/** Short names, so twelve fit around the point. */
const SHORT: Record<Placement, string> = {
  top: 'Top',
  topStart: 'Start',
  topEnd: 'End',
  right: 'Right',
  rightStart: 'Start',
  rightEnd: 'End',
  bottom: 'Bottom',
  bottomStart: 'Start',
  bottomEnd: 'End',
  left: 'Left',
  leftStart: 'Start',
  leftEnd: 'End',
}

/**
 * Where the menu opens around the click: every `Placement`, laid out on the
 * side of the point it puts the menu, with a sketch of the menu there.
 */
export function PlacementPad({ value, onChange }: PlacementPadProps) {
  return (
    <div className="placement-pad">
      <div className="placement-pad__grid" role="radiogroup" aria-label="Placement">
        {GRID.map((cell, i) =>
          cell === 'point' ? (
            <span key={i} className="placement-pad__point">
              <span className="placement-pad__sketch" data-placement={value} />
            </span>
          ) : cell ? (
            <button
              key={i}
              type="button"
              role="radio"
              aria-checked={cell === value}
              title={PLACEMENT_NAMES[cell]}
              className={cx('placement-pad__cell', cell === value && 'placement-pad__cell--active')}
              onClick={() => onChange(cell)}
            >
              {SHORT[cell]}
            </button>
          ) : (
            <span key={i} />
          ),
        )}
      </div>
      <span className="placement-pad__name">Placement.{value}</span>
    </div>
  )
}
