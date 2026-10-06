import { useEffect, useState } from 'react'

import { DetachSimulation } from './simulate-detach'
import type { Layout } from './types'

/** The example's panels, windows and drag session, redrawn whenever they change. */
export function useDetach(wayland: boolean, layout: Layout) {
  const [, setVersion] = useState(0)
  const [detach] = useState(() => new DetachSimulation(wayland, layout, () => setVersion(v => v + 1)))
  useEffect(() => () => detach.dispose(), [detach])
  return detach
}
