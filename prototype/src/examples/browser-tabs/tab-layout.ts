import type { Os } from '../../components/platform'

/**
 * The tab strip's geometry, as the example's `TabLayout`: pure functions
 * shared by the strip that draws the tabs and the controller that hit-tests
 * them, so both always agree.
 */
export const STRIP = {
  height: 40,
  tabTop: 6,
  newTabButtonWidth: 40,
  minTabWidth: 72,
  maxTabWidth: 200,
  /** How far above or below the strip a dragged tab may go before it is torn off. */
  detachMargin: 28,
  /** How far a pressed tab must move before it leaves its slot. */
  popOutDistance: 8,
}

export interface TabLayout {
  leadingInset: number
  trailingInset: number
}

/**
 * On macOS the strip sits under a transparent title bar and leaves room for
 * the traffic lights; elsewhere the title bar is hidden and the strip carries
 * its own close button at its end.
 */
export const layoutFor = (os: Os): TabLayout =>
  os === 'macos' ? { leadingInset: 78, trailingInset: 8 } : { leadingInset: 8, trailingInset: 48 }

/** The width of every tab when `count` tabs share a strip `stripWidth` wide. */
export function tabExtent(layout: TabLayout, stripWidth: number, count: number) {
  const available = stripWidth - layout.leadingInset - layout.trailingInset - STRIP.newTabButtonWidth
  if (count <= 0) return STRIP.maxTabWidth
  return Math.min(STRIP.maxTabWidth, Math.max(STRIP.minTabWidth, available / count))
}

export const tabLeft = (layout: TabLayout, index: number, extent: number) => layout.leadingInset + index * extent

/** Where a dragged tab whose left edge is at `left` belongs among `count` tabs, itself included. */
export function indexForLeft(layout: TabLayout, left: number, extent: number, count: number) {
  if (count <= 1) return 0
  return Math.min(count - 1, Math.max(0, Math.round((left - layout.leadingInset) / extent)))
}

/** Keeps a dragged tab's left edge within the tabs. */
export const clampLeft = (layout: TabLayout, left: number, extent: number, count: number) =>
  Math.min(layout.leadingInset + (count - 1) * extent, Math.max(layout.leadingInset, left))
