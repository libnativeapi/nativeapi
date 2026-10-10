import { useEffect, useState } from 'react'

import type { WindowFramePlatform } from '@dazzlabs/dazzui'

import { type HooksOptions, HooksSimulation } from './simulate-hooks'

/** The example's windows and hooks, redrawn whenever they change. */
export function useHooks(platform: WindowFramePlatform, options: HooksOptions) {
  const [, setVersion] = useState(0)
  const [hooks] = useState(() => new HooksSimulation(platform, options, () => setVersion(v => v + 1)))
  useEffect(() => () => hooks.dispose(), [hooks])
  return hooks
}
