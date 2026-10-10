import { useEffect, useState } from 'react'

import type { Os } from '../../components/platform'
import { TabsSimulation } from './simulate-tab-drag'
import type { Scene } from './types'

/** The example's windows and its drag session, redrawn whenever they change. */
export function useBrowserTabs(os: Os, wayland: boolean, scene: Scene) {
  const [, setVersion] = useState(0)
  const [tabs] = useState(() => new TabsSimulation(os, wayland, scene, () => setVersion(v => v + 1)))
  useEffect(() => () => tabs.dispose(), [tabs])
  return tabs
}
