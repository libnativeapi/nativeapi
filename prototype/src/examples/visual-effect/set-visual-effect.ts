import type { Os } from '../../components/platform'
import type { VisualEffect } from './types'

/**
 * `Window.isVisualEffectSupported`, simulated for the newest version of each
 * desktop: macOS has every material; Windows 11 22H2 draws every value as a
 * system backdrop; Linux has none, and `none` is supported everywhere.
 */
export function simulateIsVisualEffectSupported(effect: VisualEffect, os: Os): boolean {
  return effect === 'none' || os !== 'linux'
}

/** `Window.setVisualEffect`: true where the effect is in force, false (and the previous one kept) where it is not. */
export function simulateSetVisualEffect(effect: VisualEffect, os: Os): boolean {
  return simulateIsVisualEffectSupported(effect, os)
}

/**
 * The material the desktop really draws for a value: Windows has no HUD,
 * popover or menu material and no plain blur that covers the title bar, so it
 * uses its acrylic backdrop for all four.
 */
export function drawnAs(effect: VisualEffect, os: Os): VisualEffect {
  if (os === 'windows' && (effect === 'blur' || effect === 'hud' || effect === 'popover' || effect === 'menu')) return 'acrylic'
  return effect
}
