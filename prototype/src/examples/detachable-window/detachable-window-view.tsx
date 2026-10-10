import { useState } from 'react'

import type { WindowFramePlatform } from '@dazzlabs/dazzui'

import { DesktopStage, DesktopWindow } from '../../components/desktop-stage'
import { FloatingWindow } from './components/floating-window'
import { MainWindow } from './components/main-window'
import { SessionHud } from './components/session-hud'
import { LAYOUTS, type LayoutName } from './data'
import { useDetach } from './use-detach'
import './detachable-window-view.css'

export interface DetachableWindowViewProps {
  platform: WindowFramePlatform
  /** Where the panels start. */
  layout?: LayoutName
}

/**
 * The detachable window example: panels that dock in a main window's slots
 * or float in windows of their own, on top of `WindowDragSession` and
 * `WindowManager.getWindowAtPoint`. Drag a panel's header to tear it off and
 * onto an empty slot, in either window, to dock it; the Inspector's fields
 * and the running Stopwatch keep their state through every move.
 */
export function DetachableWindowView({ platform, layout = 'start' }: DetachableWindowViewProps) {
  // The app quits with its last main window; running it again is a new run.
  const [run, setRun] = useState(0)
  return <DetachableWindowRun key={run} platform={platform} layout={layout} onRestart={() => setRun(r => r + 1)} />
}

function DetachableWindowRun({
  platform,
  layout,
  onRestart,
}: Required<DetachableWindowViewProps> & { onRestart: () => void }) {
  const detach = useDetach(platform === 'omarchy', LAYOUTS[layout])
  const front = detach.frontId

  return (
    <DesktopStage
      platform={platform}
      appName="Detachable Window"
      layout="free"
      hint={detach.quit ? 'The last main window closed, so the app quit.' : undefined}
      overlay={<SessionHud detach={detach} onRestart={onRestart} />}
    >
      {/* The desktop itself, for hit testing in its coordinates. */}
      <span className="detachable-window-view__probe" ref={element => detach.attach(element?.parentElement ?? null)} />
      {!detach.quit &&
        detach.mains.map(window => (
          <DesktopWindow
            key={window.id}
            x={window.x}
            y={window.y}
            z={detach.rank(window.nativeId)}
            onPress={() => detach.focusWindow(window.nativeId)}
          >
            <div className="detachable-window-view__grab" onPointerDown={event => detach.pressWindow(event, { main: window.id })}>
              <MainWindow platform={platform} detach={detach} window={window} inactive={window.nativeId !== front} />
            </div>
          </DesktopWindow>
        ))}
      {!detach.quit &&
        detach.floatingPanels.map(id => {
          const window = detach.floating(id)!
          return (
            <DesktopWindow
              key={window.nativeId}
              x={window.x - detach.inset.x}
              y={window.y - detach.inset.y}
              z={detach.rank(window.nativeId)}
              onPress={() => detach.focusWindow(window.nativeId)}
            >
              <div className="detachable-window-view__grab" onPointerDown={event => detach.pressWindow(event, { panel: id })}>
                <FloatingWindow platform={platform} detach={detach} id={id} window={window} inactive={window.nativeId !== front} />
              </div>
            </DesktopWindow>
          )
        })}
    </DesktopStage>
  )
}
