import type { CSSProperties } from 'react'

import { KeyCap } from '@dazzlabs/dazzui'

import type { Os } from '../../../components/platform'
import { capLabel, formatKeycode, keycodeOf, layoutOf } from '../data'
import './keyboard-visualizer.css'

export interface KeyboardVisualizerProps {
  os: Os
  /** Held keys, by `KeyboardEvent.code`. */
  pressed: ReadonlySet<string>
  /** Not monitoring: the keys stay dark whatever is pressed. */
  idle?: boolean
}

/**
 * A keyboard drawn from DazzUI key caps, lit where the monitor saw a key go
 * down. Each cap carries the raw code this platform reports for it.
 */
export function KeyboardVisualizer({ os, pressed, idle }: KeyboardVisualizerProps) {
  return (
    <div className="keyboard-visualizer" data-idle={idle ? '' : undefined} aria-label="Keyboard">
      {layoutOf(os).map((row, i) => (
        <div key={i} className="keyboard-visualizer__row" data-fn={i === 0 ? '' : undefined}>
          {row.map(key => {
            const keycode = keycodeOf(key.code, os)
            return (
              <KeyCap
                key={key.code}
                variant="key"
                size="small"
                className="keyboard-visualizer__key"
                data-pressed={pressed.has(key.code) ? '' : undefined}
                title={`${key.code} · keycode ${formatKeycode(keycode, os)}`}
                style={{ '--key-w': key.w ?? 1 } as CSSProperties}
              >
                <span className="keyboard-visualizer__cap">{capLabel(key.code, os)}</span>
                <span className="keyboard-visualizer__code">{key.code === 'Fn' ? '63' : formatKeycode(keycode, os)}</span>
              </KeyCap>
            )
          })}
        </div>
      ))}
    </div>
  )
}
