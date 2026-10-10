import type { Os } from '../../../components/platform'
import { describeMask, modifierFlagsOf } from '../data'
import './modifier-lights.css'

export interface ModifierLightsProps {
  os: Os
  /** The mask the last ModifierKeysChangedEvent carried. */
  mask: number
}

/** One light per `ModifierKey` flag, lit while the mask carries it, and the mask in hex. */
export function ModifierLights({ os, mask }: ModifierLightsProps) {
  return (
    <div className="modifier-lights">
      <div className="modifier-lights__row">
        {modifierFlagsOf(os).map(flag => (
          <span
            key={flag.name}
            className="modifier-lights__light"
            data-on={mask & flag.bit ? '' : undefined}
            data-unreported={flag.reported ? undefined : ''}
            title={flag.note}
          >
            <span className="modifier-lights__dot" />
            <span className="modifier-lights__name">{flag.name}</span>
            <span className="modifier-lights__bit">{`0x${flag.bit.toString(16).padStart(2, '0')}`}</span>
          </span>
        ))}
      </div>
      <code className="modifier-lights__mask">{describeMask(mask, os)}</code>
    </div>
  )
}
