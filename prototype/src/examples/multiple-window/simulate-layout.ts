import { BLOCK_SHARE, CASCADE_STEP, CREATION_ORDER, DEFAULT_SIZE } from './data'
import type { Rect, WindowKey } from './types'

/** The block the hook lays the windows out in: 60% of the work area, centred. */
export function blockOf(workArea: Rect): Rect {
  const width = workArea.width * BLOCK_SHARE
  const height = workArea.height * BLOCK_SHARE
  return {
    x: workArea.x + (workArea.width - width) / 2,
    y: workArea.y + (workArea.height - height) / 2,
    width,
    height,
  }
}

/**
 * Where the will-show hook puts a window, as the example's
 * `_positionPrimaryWindow` and its siblings compute it: the primary window
 * the top half of the block, the other two the bottom half, side by side.
 */
export function simulateHookPlacement(key: WindowKey, workArea: Rect): Rect {
  const block = blockOf(workArea)
  const row = block.height * 0.5
  const half = block.width * 0.5
  const frame =
    key === 'primary'
      ? { x: block.x, y: block.y, width: block.width, height: row }
      : { x: key === 'secondary' ? block.x : block.x + half, y: block.y + row, width: half, height: row }
  return round(frame)
}

/** Where the platform puts a window nobody placed: 800 × 600, cascaded in creation order. */
export function defaultFrame(key: WindowKey, workArea: Rect): Rect {
  const i = CREATION_ORDER.indexOf(key)
  return round({ x: workArea.x + 40 + CASCADE_STEP * i, y: workArea.y + 24 + CASCADE_STEP * i, ...DEFAULT_SIZE })
}

/**
 * Hyprland's dwindle tiling, which a Wayland client cannot override: one
 * window fills the work area, the next splits it, the third splits the second.
 */
export function simulateHyprlandTiles(visible: readonly WindowKey[], workArea: Rect): Partial<Record<WindowKey, Rect>> {
  const gap = 10
  const area = { x: workArea.x + gap, y: workArea.y + gap, width: workArea.width - 2 * gap, height: workArea.height - 2 * gap }
  const tiles: Partial<Record<WindowKey, Rect>> = {}
  const [a, b, c] = visible
  if (!a) return tiles
  if (!b) return { [a]: round(area) }
  const left = { ...area, width: (area.width - gap) / 2 }
  const right = { ...left, x: area.x + left.width + gap }
  tiles[a] = round(left)
  if (!c) tiles[b] = round(right)
  else {
    tiles[b] = round({ ...right, height: (right.height - gap) / 2 })
    tiles[c] = round({ ...right, y: right.y + (right.height + gap) / 2, height: (right.height - gap) / 2 })
  }
  return tiles
}

const round = (r: Rect): Rect => ({ x: Math.round(r.x), y: Math.round(r.y), width: Math.round(r.width), height: Math.round(r.height) })

export const rectText = (r: Rect) => `${r.x}, ${r.y}  ${r.width} × ${r.height}`
