import { useCallback, useEffect, useRef, useState } from 'react'

import { useEventLog } from '../../components/event-bar'
import type { Os } from '../../components/platform'
import { DEFAULT_SHADOW, MORPH_MS, PREVIEW_AT, SHADOW_MS, SHADOW_PRESETS, SIZES } from './data'
import { easeInOut, easeInOutCubic, interpolateContour, morphContour } from './shape-geometry'
import type { DemoShape, Point, Shadow, ShadowColor, ShadowPreset } from './types'

export interface ShapedWindowOptions {
  /** The shape the preview starts in. */
  initialShape?: DemoShape
  /** The shadow preset the preview starts with. */
  initialPreset?: ShadowPreset
  /** Start in the shadow's Adjust view. */
  editingShadow?: boolean
  /** Start with the rectangle restored. */
  restored?: boolean
}

/** A shadow's colour mid-change: from one colour toward another. */
export interface ShadowPaint {
  from: ShadowColor
  to: ShadowColor
  t: number
}

const presetShadow = (preset: ShadowPreset): Shadow =>
  preset === 'None' ? { ...DEFAULT_SHADOW, enabled: false } : { ...SHADOW_PRESETS[preset], x: 0, enabled: true }

/** The preset a shadow matches, or Custom when the sliders made it. */
export function presetOf(shadow: Shadow): ShadowPreset | 'Custom' {
  if (!shadow.enabled) return 'None'
  for (const [name, p] of Object.entries(SHADOW_PRESETS)) {
    if (shadow.opacity === p.opacity && shadow.blur === p.blur && shadow.y === p.y && shadow.x === 0 && shadow.color === p.color) {
      return name as ShadowPreset
    }
  }
  return 'Custom'
}

const shadowLine = (s: Shadow) =>
  `setCustomShadow(WindowShadow(${s.color.toLowerCase()} ${Math.round(s.opacity * 100)}%, blur ${Math.round(s.blur)}, offset ${Math.round(s.x)},${Math.round(s.y)})) → true`

/**
 * The shaped window example's state and calls. The preview's contour morphs
 * over 450 ms, every frame handed to `setShape` (or, on Linux,
 * `setInputShape` with the clip drawn by the app); the shadow eases between
 * presets over 220 ms; the controls read the result back.
 */
export function useShapedWindow(os: Os, options: ShapedWindowOptions = {}) {
  const { initialShape = 'circle', initialPreset = 'Soft', restored: restoredAtStart = false } = options
  // On Linux the app clips in Flutter and core takes only the input region.
  const usesInputShape = os === 'linux'
  const setter = usesInputShape ? 'setInputShape' : 'setShape'
  const startShadow = presetShadow(initialPreset)
  const startPoints = morphContour(restoredAtStart ? null : initialShape, SIZES[0])

  const statusOf = (shape: DemoShape, n: number) =>
    usesInputShape ? `${shape}: Flutter clip + native input region (${n} vertices)` : `${shape}: native shape active (${n} vertices)`

  const log = useEventLog(
    [
      'setTitleBarStyle(hidden)',
      'setBackgroundColor(transparent)',
      `setHasShadow(${startShadow.enabled})`,
      shadowLine(startShadow),
      'setResizable(false)',
      ...(usesInputShape ? ['setParentWindow(main) → true'] : []),
      `setContentSize(${SIZES[0]}×${SIZES[0]})`,
      'WindowShape.create()',
      `${setter}(WindowShape · ${startPoints.length} points) → true`,
      ...(restoredAtStart ? [`${setter}(null) → true`] : []),
    ],
    restoredAtStart ? 'Rectangle restored' : statusOf(initialShape, startPoints.length),
  )

  const [shape, setShape] = useState<DemoShape>(initialShape)
  const [points, setPoints] = useState<Point[]>(startPoints)
  const [size, setSize] = useState<number>(SIZES[0])
  const [toSize, setToSize] = useState<number>(SIZES[0])
  const [restored, setRestored] = useState(restoredAtStart)
  const [count, setCount] = useState(0)
  const [editingShadow, setEditingShadow] = useState(Boolean(options.editingShadow))
  const [shadow, setShadow] = useState<Shadow>(startShadow)
  const [paint, setPaint] = useState<ShadowPaint>({ from: startShadow.color, to: startShadow.color, t: 1 })
  const [at, setAt] = useState<Point>(PREVIEW_AT)

  // What is on screen now, for retargeting a morph from the displayed frame.
  const shown = useRef({ points, size })
  shown.current = { points, size }
  const morphFrame = useRef(0)
  const shadowFrame = useRef(0)
  useEffect(
    () => () => {
      cancelAnimationFrame(morphFrame.current)
      cancelAnimationFrame(shadowFrame.current)
    },
    [],
  )

  /** Morphs the preview to `target` (null: the rectangle) at `nextSize`, from whatever is displayed. */
  const transitionTo = useCallback(
    (target: DemoShape | null, nextSize: number) => {
      cancelAnimationFrame(morphFrame.current)
      const from = shown.current.points
      const fromSize = shown.current.size
      const to = morphContour(target, nextSize)
      const start = performance.now()
      let frames = 0
      setRestored(false)
      setToSize(nextSize)
      if (nextSize !== fromSize) log.call(`setContentSize(${nextSize}×${nextSize})`)
      const step = (now: number) => {
        const t = easeInOutCubic(Math.min(1, (now - start) / MORPH_MS))
        frames++
        setPoints(interpolateContour(from, to, t))
        setSize(Math.round(fromSize + (nextSize - fromSize) * t))
        if (t < 1) {
          morphFrame.current = requestAnimationFrame(step)
          return
        }
        log.call(`${setter}(WindowShape · ${to.length} points) → true · ${frames} frames in ${MORPH_MS} ms`)
        if (target === null) {
          setRestored(true)
          log.event('Rectangle restored', `${setter}(null) → true · isShaped → false`)
        } else {
          log.event(statusOf(target, to.length), `isShaped → true${usesInputShape ? ' · isInputShaped → true' : ''}`)
        }
      }
      morphFrame.current = requestAnimationFrame(step)
    },
    // statusOf only reads usesInputShape.
    // eslint-disable-next-line react-hooks/exhaustive-deps
    [log, setter, usesInputShape],
  )

  const selectShape = useCallback(
    (next: DemoShape) => {
      setShape(next)
      transitionTo(next, toSize)
    },
    [transitionTo, toSize],
  )

  const restoreRectangle = useCallback(() => transitionTo(null, toSize), [transitionTo, toSize])

  /** Toggles the destination size; a restored rectangle stays a rectangle. */
  const toggleSize = useCallback(
    () => transitionTo(restored ? null : shape, toSize === SIZES[0] ? SIZES[1] : SIZES[0]),
    [transitionTo, restored, shape, toSize],
  )

  /**
   * A direct change from the Adjust view: applied at once, no easing. A
   * slider logs its call when it is let go (`log: false` while it moves).
   */
  const changeShadow = useCallback(
    (change: Partial<Shadow>, { log: logIt = true } = {}) => {
      cancelAnimationFrame(shadowFrame.current)
      const next = { ...shadow, ...change }
      setShadow(next)
      if (logIt) log.call(shadowLine(next))
      if (change.color) setPaint({ from: change.color, to: change.color, t: 1 })
    },
    [shadow, log],
  )

  const toggleShadow = useCallback(() => {
    cancelAnimationFrame(shadowFrame.current)
    log.call(`setHasShadow(${!shadow.enabled}) · hasShadow → ${!shadow.enabled}`)
    setShadow({ ...shadow, enabled: !shadow.enabled })
  }, [shadow, log])

  const resetShadow = useCallback(() => {
    changeShadow({ ...DEFAULT_SHADOW, enabled: shadow.enabled })
    log.event('Shadow parameters reset')
  }, [changeShadow, shadow.enabled, log])

  /** Eases from the shadow on screen to a preset; None fades it out, then hides it. */
  const selectPreset = useCallback(
    (preset: ShadowPreset) => {
      cancelAnimationFrame(shadowFrame.current)
      if (preset === 'None' && !shadow.enabled) return
      const from: Shadow = { ...shadow, opacity: shadow.enabled ? shadow.opacity : 0 }
      const to: Shadow = preset === 'None' ? { ...shadow, opacity: 0 } : { ...from, ...SHADOW_PRESETS[preset], x: 0 }
      if (preset !== 'None' && !shadow.enabled) log.call('setHasShadow(true)')
      const start = performance.now()
      let frames = 0
      const step = (now: number) => {
        const t = easeInOut(Math.min(1, (now - start) / SHADOW_MS))
        const mix = (a: number, b: number) => a + (b - a) * t
        frames++
        setShadow({
          enabled: true,
          color: to.color,
          opacity: mix(from.opacity, to.opacity),
          blur: mix(from.blur, to.blur),
          x: mix(from.x, to.x),
          y: mix(from.y, to.y),
        })
        setPaint({ from: from.color, to: to.color, t })
        if (t < 1) {
          shadowFrame.current = requestAnimationFrame(step)
          return
        }
        if (preset === 'None') {
          setShadow({ ...shadow, enabled: false })
          log.event('Shadow: None', 'setHasShadow(false) · hasShadow → false')
        } else {
          setShadow({ ...to, enabled: true })
          log.call(`${shadowLine(to)} · ${frames} frames`)
          log.event(`Shadow: ${preset}`)
        }
      }
      shadowFrame.current = requestAnimationFrame(step)
    },
    [shadow, log],
  )

  const tap = useCallback(() => setCount(n => n + 1), [])

  return {
    log,
    usesInputShape,
    shape,
    points,
    size,
    toSize,
    restored,
    count,
    editingShadow,
    setEditingShadow,
    shadow,
    paint,
    at,
    setAt,
    selectShape,
    restoreRectangle,
    toggleSize,
    changeShadow,
    toggleShadow,
    resetShadow,
    selectPreset,
    tap,
  }
}

export type ShapedWindow = ReturnType<typeof useShapedWindow>
