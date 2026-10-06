import type { Frame, ViewAlignmentName } from './types'

/** What the layout needs to know about one subview (`LayoutChild`, `view_layout.h`). */
export interface StackChild {
  visible: boolean
  /** The size the view asks for; 0 on an axis means "use intrinsic". */
  preferred: { width: number; height: number }
  intrinsic?: { width: number; height: number }
  flex: number
  alignment: ViewAlignmentName
}

/**
 * core's `ComputeStackLayout`, line for line: the playground lays its boxes
 * out with it rather than with CSS flexbox, so the frames it reads back are
 * the ones the native layout pass would compute — hidden children take no
 * space and get an empty frame, flex children share what the fixed ones,
 * the gaps and the padding leave, and nothing is clamped below 0.
 */
export function simulateStackLayout(
  layout: 'row' | 'column',
  container: { width: number; height: number },
  padding: number,
  spacing: number,
  children: readonly StackChild[],
): Frame[] {
  const row = layout === 'row'
  const nonNegative = (v: number) => (v < 0 ? 0 : v)
  const natural = (preferred: number, intrinsic: number) => (preferred > 0 ? preferred : intrinsic)
  const mainOf = (s: { width: number; height: number }) => (row ? s.width : s.height)
  const crossOf = (s: { width: number; height: number }) => (row ? s.height : s.width)
  const zero = { width: 0, height: 0 }
  const naturalMain = (c: StackChild) => natural(mainOf(c.preferred), mainOf(c.intrinsic ?? zero))
  const naturalCross = (c: StackChild) => natural(crossOf(c.preferred), crossOf(c.intrinsic ?? zero))

  const mainExtent = nonNegative((row ? container.width : container.height) - 2 * padding)
  const crossExtent = nonNegative((row ? container.height : container.width) - 2 * padding)

  const visible = children.filter(c => c.visible)
  const flexTotal = visible.reduce((sum, c) => sum + (c.flex > 0 ? c.flex : 0), 0)
  const fixedTotal = visible.reduce((sum, c) => sum + (c.flex > 0 ? 0 : naturalMain(c)), 0)
  const gaps = visible.length > 1 ? spacing * (visible.length - 1) : 0
  const leftover = nonNegative(mainExtent - fixedTotal - gaps)

  let cursor = padding
  let first = true
  return children.map(child => {
    if (!child.visible) return { x: 0, y: 0, width: 0, height: 0 }
    if (!first) cursor += spacing
    first = false
    const mainSize = child.flex > 0 ? leftover * (child.flex / flexTotal) : naturalMain(child)
    let crossSize = crossExtent
    let crossPos = padding
    if (child.alignment !== 'stretch') {
      crossSize = Math.min(naturalCross(child), crossExtent)
      if (child.alignment === 'center') crossPos = padding + (crossExtent - crossSize) / 2
      if (child.alignment === 'end') crossPos = padding + crossExtent - crossSize
    }
    const frame = row
      ? { x: cursor, y: crossPos, width: mainSize, height: crossSize }
      : { x: crossPos, y: cursor, width: crossSize, height: mainSize }
    cursor += mainSize
    return frame
  })
}

/** A frame as the inspector and the playground print it: `x,y w×h`, rounded. */
export const frameText = (f: Frame) =>
  `${Math.round(f.x)},${Math.round(f.y)} ${Math.round(f.width)}×${Math.round(f.height)}`
