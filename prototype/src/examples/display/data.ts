import type { WindowFramePlatform } from '@dazzlabs/dazzui'

import { osOf, type Os } from '../../components/platform'
import type { DisplayInfo, Margins, Scene } from './types'

type Model = Omit<DisplayInfo, 'id' | 'margins'>

/**
 * The displays each system reports: a built-in panel as the primary, an
 * external one beside it, and a third on the other side. Names are what
 * `getName()` returns there — the model on macOS and Windows, the model and
 * connector on Linux.
 */
const MODELS: Record<Os, readonly Model[]> = {
  macos: [
    {
      name: 'Built-in Retina Display',
      primary: true,
      pixelWidth: 3024,
      pixelHeight: 1964,
      scale: 2,
      refreshRate: 120,
      bitDepth: 30,
      orientation: 'landscape',
      side: 'right',
      offsetY: 0,
    },
    {
      name: 'DELL U2723QE',
      primary: false,
      pixelWidth: 2560,
      pixelHeight: 1440,
      scale: 1,
      refreshRate: 60,
      bitDepth: 24,
      orientation: 'landscape',
      side: 'right',
      offsetY: -230,
    },
    {
      name: 'LG UltraFine',
      primary: false,
      pixelWidth: 1920,
      pixelHeight: 1080,
      scale: 1,
      refreshRate: 60,
      bitDepth: 24,
      orientation: 'landscape',
      side: 'left',
      offsetY: -98,
    },
  ],
  windows: [
    {
      name: 'Dell U2723QE',
      primary: true,
      pixelWidth: 3840,
      pixelHeight: 2160,
      scale: 1.5,
      refreshRate: 60,
      bitDepth: 32,
      orientation: 'landscape',
      side: 'right',
      offsetY: 0,
    },
    {
      name: 'HP E24 G5',
      primary: false,
      pixelWidth: 1920,
      pixelHeight: 1080,
      scale: 1,
      refreshRate: 75,
      bitDepth: 32,
      orientation: 'landscape',
      side: 'right',
      offsetY: 180,
    },
    {
      name: 'Surface Laptop Display',
      primary: false,
      pixelWidth: 2496,
      pixelHeight: 1664,
      scale: 1.5,
      refreshRate: 120,
      bitDepth: 32,
      orientation: 'landscape',
      side: 'left',
      offsetY: 331,
    },
  ],
  linux: [
    {
      name: 'Built-in display (eDP-1)',
      primary: true,
      pixelWidth: 2880,
      pixelHeight: 1800,
      scale: 2,
      refreshRate: 90,
      bitDepth: 24,
      orientation: 'landscape',
      side: 'right',
      offsetY: 0,
    },
    {
      name: 'LG 27UL850 (DP-1)',
      primary: false,
      pixelWidth: 3840,
      pixelHeight: 2160,
      scale: 1.5,
      refreshRate: 60,
      bitDepth: 24,
      orientation: 'landscape',
      side: 'right',
      offsetY: -270,
    },
    {
      name: 'DELL P2419H (HDMI-1)',
      primary: false,
      pixelWidth: 1920,
      pixelHeight: 1080,
      scale: 1,
      refreshRate: 60,
      bitDepth: 24,
      orientation: 'landscape',
      side: 'left',
      offsetY: -90,
    },
  ],
}

/** What Plug in display connects: a new monitor at the far right. */
const PLUGGABLE: Record<Os, Model> = {
  macos: {
    name: 'Studio Display',
    primary: false,
    pixelWidth: 5120,
    pixelHeight: 2880,
    scale: 2,
    refreshRate: 60,
    bitDepth: 30,
    orientation: 'landscape',
    side: 'right',
    offsetY: -230,
  },
  windows: {
    name: 'LG 27GP850',
    primary: false,
    pixelWidth: 2560,
    pixelHeight: 1440,
    scale: 1,
    refreshRate: 165,
    bitDepth: 32,
    orientation: 'landscape',
    side: 'right',
    offsetY: 0,
  },
  linux: {
    name: 'Samsung S27R (DP-2)',
    primary: false,
    pixelWidth: 1920,
    pixelHeight: 1080,
    scale: 1,
    refreshRate: 75,
    bitDepth: 24,
    orientation: 'landscape',
    side: 'right',
    offsetY: 0,
  },
}

/** The scales Change scale steps through, per system. */
export const SCALES: Record<Os, readonly number[]> = {
  macos: [1, 2],
  windows: [1, 1.25, 1.5, 1.75, 2],
  linux: [1, 1.25, 1.5, 2],
}

/** What each desktop's bars are called, for the legend. */
export const STRIPS: Record<WindowFramePlatform, string> = {
  macos: 'Menu bar, Dock',
  macos15: 'Menu bar, Dock',
  windows: 'Taskbar',
  gnome: 'Top bar',
  ubuntu: 'Top bar, Dock',
  kde: 'Panel',
  omarchy: 'Waybar',
}

/**
 * What the desktop's bars keep off a display: the menu bar on every Mac
 * display and the Dock on the primary; the taskbar on every Windows display;
 * GNOME's top bar (and Ubuntu's dock) and KDE's panel on the primary only;
 * Waybar on every Hyprland output.
 */
export function marginsFor(platform: WindowFramePlatform, primary: boolean, builtIn: boolean): Margins {
  const none = { top: 0, right: 0, bottom: 0, left: 0 }
  switch (platform) {
    case 'macos':
    case 'macos15':
      // The notch makes the built-in panel's menu bar taller.
      return { ...none, top: builtIn ? 33 : 25, bottom: primary ? 76 : 0 }
    case 'windows':
      return { ...none, bottom: 48 }
    case 'gnome':
      return primary ? { ...none, top: 32 } : none
    case 'ubuntu':
      return primary ? { ...none, top: 32, left: 64 } : none
    case 'kde':
      return primary ? { ...none, bottom: 44 } : none
    case 'omarchy':
      return { ...none, top: 26 }
  }
}

/** The displays a scene starts with, numbered from 1 as `DisplayManager` hands them out. */
export function displaysFor(platform: WindowFramePlatform, scene: Scene): DisplayInfo[] {
  const os = osOf(platform)
  const models = MODELS[os]
  const picked = scene === 'single' ? models.slice(0, 1) : scene === 'three' ? models : models.slice(0, 2)
  return picked.map((model, i) => {
    const rotated = scene === 'rotated' && i === 1
    return {
      ...model,
      id: i + 1,
      orientation: rotated ? 'portrait' : model.orientation,
      // A portrait display stands taller: centre it on the primary.
      offsetY: rotated ? -Math.round(model.pixelWidth / model.scale / 3) : model.offsetY,
      margins: marginsFor(platform, model.primary, i === 0 && os !== 'windows'),
    }
  })
}

export function pluggableFor(platform: WindowFramePlatform, id: number): DisplayInfo {
  return { ...PLUGGABLE[osOf(platform)], id, margins: marginsFor(platform, false, false) }
}

export const ORIENTATION_NAMES: Record<DisplayInfo['orientation'], string> = {
  portrait: 'kPortrait',
  landscape: 'kLandscape',
  portraitFlipped: 'kPortraitFlipped',
  landscapeFlipped: 'kLandscapeFlipped',
}
