import { useRef, useState } from 'react'

import type { WindowFramePlatform } from '@dazzlabs/dazzui'

import { DesktopStage, DesktopWindow, usePointerDrag } from '../../components/desktop-stage'
import { osOf } from '../../components/platform'
import { ControlsWindow } from './components/controls-window'
import { ShapePreview } from './components/shape-preview'
import { CONTROLS_AT } from './data'
import { type ShapedWindowOptions, useShapedWindow } from './use-shaped-window'
import './shaped-window-view.css'

export interface ShapedWindowViewProps extends ShapedWindowOptions {
  platform: WindowFramePlatform
}

/**
 * The shaped window example: a controls window, "Window shapes", and the
 * "Shape preview" window beside it on the desktop, clipped to the chosen
 * silhouette with a shadow that follows the contour. Picking a shape morphs
 * the window over 450 ms; the handle drags it, and a press outside the
 * silhouette falls through to whatever is behind. On Linux the preview is a
 * child of the controls, so it stays above them where the app cannot place it.
 */
export function ShapedWindowView({ platform, ...options }: ShapedWindowViewProps) {
  const os = osOf(platform)
  const sw = useShapedWindow(os, options)
  const [front, setFront] = useState<'controls' | 'preview'>('controls')
  // A child window stays above its parent, whichever is brought forward.
  const previewOnTop = sw.usesInputShape || front === 'preview'

  const from = useRef(sw.at)
  const drag = usePointerDrag({
    onStart: () => {
      from.current = sw.at
      setFront('preview')
      sw.log.call('startDragging()')
    },
    onMove: ({ dx, dy }) => sw.setAt({ x: from.current.x + dx, y: Math.max(0, from.current.y + dy) }),
    onEnd: ({ dx, dy }) => {
      if (dx !== 0 || dy !== 0) sw.log.event('Preview moved', `WindowMovedEvent → ${from.current.x + dx}, ${Math.max(0, from.current.y + dy)}`)
    },
  })

  return (
    <DesktopStage platform={platform} appName="Window Shapes" layout="free">
      <DesktopWindow x={CONTROLS_AT.x} y={CONTROLS_AT.y} z={previewOnTop ? 1 : 2} onPress={() => setFront('controls')}>
        <ControlsWindow platform={platform} sw={sw} inactive={front !== 'controls'} />
      </DesktopWindow>
      <DesktopWindow
        x={sw.at.x}
        y={sw.at.y}
        z={previewOnTop ? 2 : 1}
        className="shaped-window-view__preview"
        onPress={() => setFront('preview')}
      >
        <ShapePreview
          shape={sw.shape}
          points={sw.points}
          size={sw.size}
          restored={sw.restored}
          shadow={sw.shadow}
          paint={sw.paint}
          count={sw.count}
          onTap={sw.tap}
          onDragPress={drag}
        />
      </DesktopWindow>
    </DesktopStage>
  )
}
