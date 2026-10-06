import { useEffect, useState } from 'react'

import type { Brightness } from './types'

/** The `data-theme` the toolbar put on the page: `macos27-light`; `:root`'s Studio Light when there is none. */
const rootTheme = () => document.documentElement.getAttribute('data-theme') ?? 'studio-light'

/**
 * The theme a window forced to `brightness` is drawn in: the toolbar's style
 * in the forced appearance, or nothing for `System`, which follows the
 * toolbar's own Light/Dark. Tracks the toolbar as it changes.
 */
export function useForcedTheme() {
  const [root, setRoot] = useState(rootTheme)
  useEffect(() => {
    const observer = new MutationObserver(() => setRoot(rootTheme()))
    observer.observe(document.documentElement, { attributes: true, attributeFilter: ['data-theme'] })
    return () => observer.disconnect()
  }, [])
  const style = root.replace(/-(light|dark)$/, '')
  const system: 'light' | 'dark' = root.endsWith('-dark') ? 'dark' : 'light'
  return {
    /** What `System` currently means. */
    system,
    themeOf: (brightness: Brightness) => (brightness === 'system' ? undefined : `${style}-${brightness}`),
    appearanceOf: (brightness: Brightness): 'light' | 'dark' => (brightness === 'system' ? system : brightness),
  }
}
