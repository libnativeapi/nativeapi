import { useRef } from 'react'

import type { WindowFramePlatform } from '@dazzlabs/dazzui'

import { DesktopStage, DesktopWindow, usePointerDrag } from '../../components/desktop-stage'
import { EventBar } from '../../components/event-bar'
import { osOf } from '../../components/platform'
import { ReadBack } from '../../components/read-back'
import { BackdropWindow } from './components/backdrop-window'
import { MaterialWindow } from './components/material-window'
import { WallpaperShapes } from './components/wallpaper-shapes'
import { BACKDROP_INSET, WINDOW_SIZE } from './data'
import { simulateIsVisualEffectSupported } from './set-visual-effect'
import type { Point, VisualEffect } from './types'
import { useVisualEffect } from './use-visual-effect'

export interface VisualEffectViewProps {
  platform: WindowFramePlatform
  /** An effect applied as the example starts. */
  initialEffect?: VisualEffect
  /** The red backdrop window already behind the example. */
  backdrop?: boolean
}

/**
 * The visual effect example: one window whose background is a translucent
 * material, on a wallpaper with shapes to see through to. A raised card picks
 * the `VisualEffect` (greyed where `isVisualEffectSupported` is false), the
 * rest of the window stays unpainted, and the Backdrop switch puts a plain
 * red parent window behind it. Both windows drag by their title bars.
 */
export function VisualEffectView({ platform, initialEffect, backdrop: backdropAtStart }: VisualEffectViewProps) {
  const os = osOf(platform)
  const ve = useVisualEffect(os, { initialEffect, backdrop: backdropAtStart })
  const { log, effect, at, backdrop } = ve

  // Where each window stood when its drag began.
  const from = useRef<{ window: Point; backdrop: Point }>({ window: at, backdrop: at })
  const startDrag = () => {
    from.current = { window: at, backdrop: backdrop?.at ?? at }
  }
  const dragWindow = usePointerDrag({
    onStart: () => {
      startDrag()
      ve.setActive(true)
    },
    onMove: ({ dx, dy }) => ve.moveWindow({ x: from.current.window.x + dx, y: Math.max(0, from.current.window.y + dy) }),
  })
  const dragBackdrop = usePointerDrag({
    onStart: startDrag,
    onMove: ({ dx, dy }) =>
      ve.moveBackdrop({ x: from.current.backdrop.x + dx, y: Math.max(0, from.current.backdrop.y + dy) }, from.current),
  })

  return (
    <DesktopStage platform={platform} appName="Visual Effect Example" layout="free" onPress={() => ve.setActive(false)}>
      <WallpaperShapes />
      {backdrop && (
        // The parent: a press brings it forward, but its child stays above it.
        <DesktopWindow x={backdrop.at.x} y={backdrop.at.y} z={1} onPress={() => ve.setActive(false)}>
          <BackdropWindow
            platform={platform}
            id={backdrop.id}
            width={WINDOW_SIZE.width + 2 * BACKDROP_INSET.x}
            height={WINDOW_SIZE.height + 2 * BACKDROP_INSET.y}
            inactive={ve.active}
            onTitlePress={dragBackdrop}
            onClose={ve.toggleBackdrop}
          />
        </DesktopWindow>
      )}
      <DesktopWindow x={at.x} y={at.y} z={2} onPress={() => ve.setActive(true)}>
        <MaterialWindow
          platform={platform}
          effect={effect}
          note={ve.note}
          active={ve.active}
          backdrop={backdrop !== null}
          onApply={ve.apply}
          onToggleBackdrop={ve.toggleBackdrop}
          onTitlePress={dragWindow}
          readBack={
            <ReadBack
              columns={2}
              keyWidth="7rem"
              rows={[
                ['visualEffect', effect],
                ['isSupported', String(simulateIsVisualEffectSupported(effect, os))],
                ['parentWindow', backdrop ? `#${backdrop.id} Backdrop` : 'null'],
                ['background', effect === 'none' ? 'canvas, shown' : 'kept, not shown'],
              ]}
            />
          }
          footer={<EventBar lastEvent={log.lastEvent} log={log.log} onClear={log.clear} />}
        />
      </DesktopWindow>
    </DesktopStage>
  )
}
