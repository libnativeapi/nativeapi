import type { CSSProperties } from 'react'

import './avatar-image.css'

/**
 * The header's avatar, as the example paints it in Dart before encoding it
 * to a PNG for `ImageView.setImage`: a round badge with a diagonal gradient
 * off the accent, a light ring and a highlight. `seed` moves the highlight
 * and twists the gradient, so every "New avatar" looks a little different.
 */
export function AvatarImage({ seed }: { seed: number }) {
  // The same small generator for the same seed, so a seed always paints alike.
  let state = (seed * 2654435761 + 7) >>> 0
  const random = () => {
    state = (state * 1664525 + 1013904223) >>> 0
    return state / 2 ** 32
  }
  const glowX = 25 + random() * 50
  const glowY = 20 + random() * 40
  const twist = random() * 60 - 30
  return (
    <span
      className="avatar-image"
      style={
        {
          '--avatar-glow-x': `${glowX}%`,
          '--avatar-glow-y': `${glowY}%`,
          '--avatar-angle': `${135 + twist}deg`,
        } as CSSProperties
      }
    />
  )
}
