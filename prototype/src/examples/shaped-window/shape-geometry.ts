import { SHAPES } from './data'
import type { DemoShape, Point } from './types'

/*
 * The example's polygons, ported from its shape_geometry.dart: the same
 * points are the native window's shape, the Flutter clip and here the CSS
 * clip-path, so what is drawn, what is hit and what casts the shadow agree.
 */

const pt = (x: number, y: number): Point => ({ x, y })
const lerp = (a: Point, b: Point, t: number): Point => pt(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t)
const distance = (a: Point, b: Point) => Math.hypot(b.x - a.x, b.y - a.y)

const radial = (size: number, count: number, radius: (a: number) => number) =>
  Array.from({ length: count }, (_, i) => {
    const a = -Math.PI / 2 + (i * Math.PI * 2) / count
    const r = size * radius(a)
    return pt(size / 2 + r * Math.cos(a), size / 2 + r * Math.sin(a))
  })

/** Quadratic corner arcs, sampled into the polygon so the corners are round everywhere it is used. */
function roundCorners(points: Point[]): Point[] {
  const rounded: Point[] = []
  points.forEach((corner, i) => {
    const before = points[(i + points.length - 1) % points.length]!
    const after = points[(i + 1) % points.length]!
    const entry = lerp(corner, before, 0.2)
    const exit = lerp(corner, after, 0.2)
    for (let j = 0; j <= 6; j++) {
      const t = j / 6
      const a = (1 - t) * (1 - t)
      const b = 2 * (1 - t) * t
      const c = t * t
      rounded.push(pt(entry.x * a + corner.x * b + exit.x * c, entry.y * a + corner.y * b + exit.y * c))
    }
  })
  return rounded
}

/** A shape's polygon in a `size` × `size` window, in content-local logical pixels. */
export function shapePoints(shape: DemoShape, size: number): Point[] {
  const c = size / 2
  const points: Point[] = (() => {
    switch (shape) {
      case 'circle':
        return Array.from({ length: 128 }, (_, i) => {
          const a = (i * Math.PI * 2) / 128
          return pt(c + (c - 4) * Math.cos(a), c + (c - 4) * Math.sin(a))
        })
      case 'star':
        return Array.from({ length: 10 }, (_, i) => {
          const a = -Math.PI / 2 + (i * Math.PI) / 5
          const r = (c - 4) * (i % 2 === 0 ? 1 : 0.6)
          return pt(c + r * Math.cos(a), c + r * Math.sin(a))
        })
      case 'heart':
        return Array.from({ length: 96 }, (_, i) => {
          const t = (i * Math.PI * 2) / 96
          const x = 16 * Math.sin(t) ** 3
          const y = 13 * Math.cos(t) - 5 * Math.cos(2 * t) - 2 * Math.cos(3 * t) - Math.cos(4 * t)
          return pt(size * (0.5 + x / 36), size * (0.48 - y / 36))
        })
      case 'flower':
        return radial(size, 96, a => 0.39 + 0.075 * Math.cos(6 * (a + Math.PI / 2)))
      case 'hexagon':
        return radial(size, 6, () => 0.47)
      case 'squircle':
        return Array.from({ length: 96 }, (_, i) => {
          const a = -Math.PI / 2 + (i * Math.PI * 2) / 96
          const axis = (v: number) => Math.sign(v) * Math.abs(v) ** 0.5
          return pt(c + size * 0.43 * axis(Math.cos(a)), c + size * 0.43 * axis(Math.sin(a)))
        })
      case 'blob':
        return radial(size, 96, a => 0.39 + 0.045 * Math.sin(3 * a + 1) + 0.025 * Math.cos(5 * a))
      case 'burst':
        return radial(size, 24, a => 0.4 + 0.065 * Math.cos(12 * (a + Math.PI / 2)))
      case 'droplet':
        return Array.from({ length: 96 }, (_, i) => {
          const t = (i * Math.PI * 2) / 96
          return pt(size * (0.5 + 0.46 * Math.sin(t) * (0.72 - 0.28 * Math.cos(t))), size * (0.5 - 0.46 * Math.cos(t)))
        })
      case 'diamond':
        return radial(size, 4, () => 0.47)
      case 'shield':
        return [
          [0.5, 0.07],
          [0.9, 0.16],
          [0.85, 0.6],
          [0.7, 0.8],
          [0.5, 0.95],
          [0.3, 0.8],
          [0.15, 0.6],
          [0.1, 0.16],
        ].map(([x, y]) => pt(size * x!, size * y!))
      case 'bubble':
        return [
          [0.12, 0.08],
          [0.88, 0.08],
          [0.96, 0.16],
          [0.96, 0.72],
          [0.88, 0.8],
          [0.4, 0.8],
          [0.16, 0.96],
          [0.21, 0.8],
          [0.12, 0.8],
          [0.04, 0.72],
          [0.04, 0.16],
        ].map(([x, y]) => pt(size * x!, size * y!))
    }
  })()
  const angular: DemoShape[] = ['star', 'bubble', 'hexagon', 'burst', 'diamond', 'shield']
  return angular.includes(shape) ? roundCorners(points) : points
}

const REFERENCE = 320

/**
 * Every contour resampled at the same perimeter landmarks — every original
 * vertex of every shape included — so any two can be morphed point for
 * point without twisting. `null` is the restored rectangle.
 */
function referenceContours(): Map<DemoShape | null, Point[]> {
  const polygons = new Map<DemoShape | null, Point[]>()
  for (const shape of SHAPES) polygons.set(shape, shapePoints(shape, REFERENCE))
  polygons.set(null, [pt(REFERENCE / 2, 0), pt(REFERENCE, 0), pt(REFERENCE, REFERENCE), pt(0, REFERENCE), pt(0, 0)])
  // The same top-centre starting landmark for every clockwise contour.
  const circle = polygons.get('circle')!
  polygons.set('circle', [...circle.slice(96), ...circle.slice(0, 96)])
  const bubble = polygons.get('bubble')!
  polygons.set('bubble', [pt(REFERENCE / 2, REFERENCE * 0.08), ...bubble.slice(7), ...bubble.slice(0, 7)])

  const fractionsOf = new Map<DemoShape | null, number[]>()
  const landmarks = new Set<number>(Array.from({ length: 128 }, (_, i) => i / 128))
  for (const [shape, points] of polygons) {
    const distances = [0]
    points.forEach((p, i) => distances.push(distances[distances.length - 1]! + distance(p, points[(i + 1) % points.length]!)))
    const total = distances[distances.length - 1]!
    const fractions = distances.map(d => d / total)
    fractionsOf.set(shape, fractions)
    fractions.slice(0, points.length).forEach(f => landmarks.add(f))
  }
  const parameters = [...landmarks].sort((a, b) => a - b)
  const contours = new Map<DemoShape | null, Point[]>()
  for (const [shape, points] of polygons) {
    const fractions = fractionsOf.get(shape)!
    let edge = 0
    contours.set(
      shape,
      parameters.map(t => {
        while (edge + 1 < points.length && fractions[edge + 1]! <= t) edge++
        const span = fractions[edge + 1]! - fractions[edge]!
        return lerp(points[edge]!, points[(edge + 1) % points.length]!, span > 0 ? (t - fractions[edge]!) / span : 0)
      }),
    )
  }
  return contours
}

let reference: Map<DemoShape | null, Point[]> | null = null

/** A shape's morphable contour at `extent` × `extent`; null for the rectangle. */
export function morphContour(shape: DemoShape | null, extent: number): Point[] {
  reference ??= referenceContours()
  const points = reference.get(shape)!
  // Circle and star keep their absolute 4 px inset; the rest scale fully.
  if (shape === 'circle' || shape === 'star') {
    const k = (extent - 8) / (REFERENCE - 8)
    return points.map(p => pt(extent / 2 + (p.x - REFERENCE / 2) * k, extent / 2 + (p.y - REFERENCE / 2) * k))
  }
  const k = extent / REFERENCE
  return points.map(p => pt(p.x * k, p.y * k))
}

export const interpolateContour = (from: Point[], to: Point[], t: number) => from.map((p, i) => lerp(p, to[i]!, t))

/** A polygon as CSS `clip-path`, in pixels of the element it clips. */
export const clipPathOf = (points: readonly Point[]) =>
  `polygon(evenodd, ${points.map(p => `${p.x.toFixed(1)}px ${p.y.toFixed(1)}px`).join(', ')})`

export const easeInOutCubic = (t: number) => (t < 0.5 ? 4 * t * t * t : 1 - (-2 * t + 2) ** 3 / 2)
export const easeInOut = (t: number) => (t < 0.5 ? 2 * t * t : 1 - (-2 * t + 2) ** 2 / 2)
