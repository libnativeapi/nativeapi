import { Add12Filled, ArrowRight12Filled, Document20Regular, Link12Filled, Prohibited12Regular, TextDescription20Regular } from '@fluentui/react-icons'

import { cx, Icon } from '@dazzlabs/dazzui'

import type { DragSession } from '../types'
import './drag-ghost.css'

const BADGES = { copy: Add12Filled, move: ArrowRight12Filled, link: Link12Filled, none: Prohibited12Regular }

/**
 * What the platform draws under the pointer during a drag: the data's image
 * and name, and a badge for what the target under it would do — copy, move,
 * link, or nothing.
 */
export function DragGhost({ session }: { session: DragSession }) {
  const { payload } = session
  const shown = session.over ? session.operation : null
  return (
    <div className="drag-ghost" style={{ left: session.x, top: session.y }} aria-hidden>
      <span className="drag-ghost__card">
        <Icon icon={payload.paths.length ? Document20Regular : TextDescription20Regular} />
        <span className="drag-ghost__label">{payload.label}</span>
        {payload.paths.length > 1 && <span className="drag-ghost__count">{payload.paths.length}</span>}
      </span>
      {shown && (
        <span className={cx('drag-ghost__badge', `drag-ghost__badge--${shown}`)}>
          <Icon icon={BADGES[shown]} size={12} />
        </span>
      )}
    </div>
  )
}
