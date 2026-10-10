import { useCallback, useMemo, useRef, useState } from 'react'

import type { WindowFramePlatform } from '@dazzlabs/dazzui'

import { useEventLog } from '../../components/event-bar'
import { osOf } from '../../components/platform'
import { displaysFor, ORIENTATION_NAMES, pluggableFor, SCALES } from './data'
import type { DisplayInfo, PlacedDisplay, Scene } from './types'

const portrait = (d: DisplayInfo) => d.orientation === 'portrait' || d.orientation === 'portraitFlipped'

/** `getSize()`: the panel's pixels over the scale, turned with the display. */
export function sizeOf(d: DisplayInfo) {
  const w = Math.round(d.pixelWidth / d.scale)
  const h = Math.round(d.pixelHeight / d.scale)
  return portrait(d) ? { width: h, height: w } : { width: w, height: h }
}

/**
 * The system's arrangement: the primary display's top-left is the origin of
 * the desktop coordinates, and the others stand edge to edge on its sides,
 * so a display that turns or rescales pushes its neighbours along.
 */
export function layout(displays: readonly DisplayInfo[]): PlacedDisplay[] {
  const primary = displays.find(d => d.primary) ?? displays[0]
  if (!primary) return []
  const place = (d: DisplayInfo, x: number): PlacedDisplay => {
    const { width, height } = sizeOf(d)
    const y = d.primary ? 0 : d.offsetY
    const m = d.margins
    return {
      ...d,
      x,
      y,
      width,
      height,
      workArea: { x: x + m.left, y: y + m.top, width: width - m.left - m.right, height: height - m.top - m.bottom },
    }
  }
  const placed = new Map<number, PlacedDisplay>()
  const first = place(primary, 0)
  placed.set(primary.id, first)
  let right = first.width
  let left = 0
  for (const d of displays) {
    if (d === primary) continue
    if (d.side === 'right') {
      const p = place(d, right)
      right += p.width
      placed.set(d.id, p)
    } else {
      const p = place(d, left - sizeOf(d).width)
      left = p.x
      placed.set(d.id, p)
    }
  }
  // Enumeration order, as getAll() returns them.
  return displays.map(d => placed.get(d.id)!)
}

export const scaleText = (scale: number) => String(Number(scale.toFixed(2)))

export const sizeText = (d: { width: number; height: number }) => `${d.width} × ${d.height}`

/** How far the work area stays off each edge: `T:33 B:76`, or `None`. */
export function marginsText(d: PlacedDisplay) {
  const parts = [
    ['T', d.workArea.y - d.y],
    ['B', d.y + d.height - (d.workArea.y + d.workArea.height)],
    ['L', d.workArea.x - d.x],
    ['R', d.x + d.width - (d.workArea.x + d.workArea.width)],
  ].filter(([, v]) => (v as number) > 0)
  return parts.length ? parts.map(([k, v]) => `${k}:${v}`).join(' ') : 'None'
}

/**
 * The example's `DisplayManager`, simulated: the displays it reports, the
 * selection, and the events a real one emits when a display is plugged in,
 * unplugged, turned or rescaled. The example listens to them, so the list
 * follows without a Refresh; Refresh calls `getAll()` again all the same.
 */
export function useDisplays(platform: WindowFramePlatform, scene: Scene) {
  const os = osOf(platform)
  const [displays, setDisplays] = useState<DisplayInfo[]>(() => displaysFor(platform, scene))
  const [selectedId, setSelectedId] = useState(1)
  const nextId = useRef(displays.length + 1)
  const placed = useMemo(() => layout(displays), [displays])
  const primary = placed.find(d => d.primary) ?? placed[0]!
  const selected = placed.find(d => d.id === selectedId) ?? primary
  const log = useEventLog(
    [
      `getAll() → ${displays.length} display${displays.length === 1 ? '' : 's'}`,
      `getPrimary() → #${primary.id} "${primary.name}"`,
      'addListener(DisplayEvent) → 1',
    ],
    'Listening for display changes',
  )
  const { event, call } = log

  const refresh = useCallback(() => {
    event(
      `Found ${displays.length} display${displays.length === 1 ? '' : 's'}`,
      `getAll() → ${displays.length} display${displays.length === 1 ? '' : 's'}`,
    )
  }, [displays.length, event])

  const pluggable = pluggableFor(platform, 0)
  const plugged = displays.some(d => d.name === pluggable.name)

  const plugIn = () => {
    if (plugged) return
    const added = pluggableFor(platform, nextId.current++)
    setDisplays(list => [...list, added])
    setSelectedId(added.id)
    const { width, height } = sizeOf(added)
    event(`Display added: ${added.name}`, `DisplayAddedEvent → #${added.id} "${added.name}" ${width} × ${height}`)
  }

  /** The selected display, or the last secondary one when the primary is selected. */
  const unplugTarget = selected.primary ? [...displays].reverse().find(d => !d.primary) : selected
  const unplug = () => {
    if (!unplugTarget) return
    setDisplays(list => list.filter(d => d.id !== unplugTarget.id))
    if (unplugTarget.id === selectedId) setSelectedId(primary.id)
    event(`Display removed: ${unplugTarget.name}`, `DisplayRemovedEvent → #${unplugTarget.id} "${unplugTarget.name}"`)
  }

  const change = (id: number, update: (d: DisplayInfo) => DisplayInfo, what: (d: DisplayInfo) => string) => {
    const before = displays.find(d => d.id === id)
    if (!before) return
    const after = update(before)
    setDisplays(list => list.map(d => (d.id === id ? after : d)))
    event(`Display changed: ${after.name}`, `DisplayChangedEvent → #${id} ${what(after)}, ${sizeText(sizeOf(after))}`)
  }

  const rotate = () =>
    change(
      selected.id,
      d =>
        portrait(d)
          ? { ...d, orientation: 'landscape', offsetY: d.primary ? 0 : Math.round(d.offsetY / 3) }
          : { ...d, orientation: 'portrait', offsetY: d.primary ? 0 : -Math.round(sizeOf(d).width / 3) },
      d => `orientation ${ORIENTATION_NAMES[d.orientation]}`,
    )

  const scales = SCALES[os]
  const nextScale = scales[(scales.indexOf(selected.scale) + 1) % scales.length] ?? scales[0]!
  const changeScale = () =>
    change(
      selected.id,
      d => ({ ...d, scale: nextScale }),
      d => `scaleFactor ${scaleText(d.scale)}`,
    )

  const select = (id: number) => {
    setSelectedId(id)
    const d = placed.find(p => p.id === id)
    if (d) call(`getAll()[${placed.indexOf(d)}] → #${d.id} "${d.name}"`)
  }

  return {
    displays: placed,
    primary,
    selected,
    log,
    plugged,
    pluggableName: pluggable.name,
    unplugTarget,
    nextScale,
    actions: { refresh, plugIn, unplug, rotate, changeScale, select },
  }
}
