import { useState } from 'react'

import type { WindowFramePlatform } from '@dazzlabs/dazzui'

import { DesktopStage, DesktopWindow } from '../../components/desktop-stage'
import { osOf } from '../../components/platform'
import { BrowserWindow } from './components/browser-window'
import { SessionHud } from './components/session-hud'
import type { Scene } from './types'
import { useBrowserTabs } from './use-browser-tabs'
import './browser-tabs-view.css'

export interface BrowserTabsViewProps {
  platform: WindowFramePlatform
  /** The windows the example opens with. */
  scene?: Scene
}

/**
 * The browser tabs example: Chrome-style tabs across windows, on top of
 * `WindowDragSession` and `WindowManager.getWindowAtPoint`. Two windows open
 * with four and two tabs. Drag a tab along its strip to reorder it, pull it
 * out of the strip to tear it off into a window that follows the cursor, drop
 * that on another strip to merge, or drag the empty strip to move the window.
 * Every tab's page keeps its state through all of it.
 */
export function BrowserTabsView({ platform, scene = 'twoWindows' }: BrowserTabsViewProps) {
  // The app quits with its last window; running it again is a new run.
  const [run, setRun] = useState(0)
  return <BrowserTabsRun key={run} platform={platform} scene={scene} onRestart={() => setRun(r => r + 1)} />
}

function BrowserTabsRun({ platform, scene, onRestart }: Required<BrowserTabsViewProps> & { onRestart: () => void }) {
  const wayland = platform === 'omarchy'
  const tabs = useBrowserTabs(osOf(platform), wayland, scene)
  const front = tabs.frontId
  // Stacking by rank, so the windows stay under the HUD however often they are raised.
  const order = [...tabs.windows].sort((a, b) => a.z - b.z).map(w => w.id)

  return (
    <DesktopStage
      platform={platform}
      appName="Browser"
      layout="free"
      className="browser-tabs-view"
      hint={
        tabs.quit
          ? 'The last window closed, so the app quit.'
          : wayland
            ? 'Drag a tab to reorder it · drag the empty strip to move the window'
            : 'Drag a tab to reorder · pull it 28px out of the strip to tear it off · drop it on another strip to merge'
      }
      overlay={<SessionHud tabs={tabs} wayland={wayland} onRestart={onRestart} />}
    >
      {tabs.windows.map(window => (
        <DesktopWindow key={window.id} x={window.x} y={window.y} z={order.indexOf(window.id) + 1} onPress={() => tabs.focus(window.id)}>
          <BrowserWindow platform={platform} tabs={tabs} window={window} inactive={window.id !== front} />
        </DesktopWindow>
      ))}
    </DesktopStage>
  )
}
