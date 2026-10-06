import { Button, PreferenceRow, PreferenceSection, Switch, type WindowFramePlatform } from '@dazzlabs/dazzui'

import { Panel, PanelPreferences } from '../../../components/panel'
import { ReadBack } from '../../../components/read-back'
import type { Area, SimWindow } from '../types'
import { boundsOf, rectText, type WindowActions } from '../use-windows'
import './panels.css'

export interface WindowPanelProps {
  w: SimWindow
  platform: WindowFramePlatform
  area: Area
  focused: boolean
  actions: WindowActions
}

/** Visibility, maximized / minimized / full screen, and focus — what the badges above say, and the calls that change it. */
export function StatePanel({ w, platform, area, focused, actions }: WindowPanelProps) {
  const { bounds, content } = boundsOf(w, area, platform)
  const id = w.id
  return (
    <Panel>
      <ReadBack
        keyWidth="8.5rem"
        rows={[
          ['isVisible', String(w.visible)],
          ['isFocused', String(focused)],
          ['isMaximized', String(w.maximized)],
          ['isMinimized', String(w.minimized)],
          ['isFullScreen', String(w.fullScreen)],
          ['isAlwaysOnTop', String(w.alwaysOnTop)],
          ['getBounds', rectText(bounds)],
          ['getContentBounds', rectText(content)],
        ]}
      />
      <PanelPreferences>
        <PreferenceSection label="Visibility">
          <PreferenceRow title="Visible" subtitle={w.visible ? 'Shown' : 'Hidden: out of the taskbar and the Dock too'}>
            <div className="example-panel__actions">
              <Button size="small" variant="normal" onClick={() => actions.show(id)}>
                Show
              </Button>
              <Button size="small" variant="normal" onClick={() => actions.showInactive(id)}>
                Show inactive
              </Button>
              <Button size="small" variant="normal" onClick={() => actions.hide(id)}>
                Hide
              </Button>
            </div>
          </PreferenceRow>
        </PreferenceSection>
        <PreferenceSection label="Window state">
          <PreferenceRow title="Maximized" subtitle={w.maximizable ? (w.maximized ? 'Fills the work area' : 'No') : 'isMaximizable → false'}>
            <div className="example-panel__actions">
              <Button size="small" variant="normal" onClick={() => actions.maximize(id)}>
                Maximize
              </Button>
              <Button size="small" variant="normal" onClick={() => actions.unmaximize(id)}>
                Unmaximize
              </Button>
            </div>
          </PreferenceRow>
          <PreferenceRow title="Minimized" subtitle={w.minimized ? 'In the taskbar or the Dock: click it there to restore' : 'No'}>
            <div className="example-panel__actions">
              <Button size="small" variant="normal" onClick={() => actions.minimize(id)}>
                Minimize
              </Button>
              <Button size="small" variant="normal" onClick={() => actions.restore(id)}>
                Restore
              </Button>
            </div>
          </PreferenceRow>
          <PreferenceRow title="Full screen" subtitle={w.fullScreenable ? 'Covers the whole display, bar included' : 'isFullScreenable → false'}>
            <Switch checked={w.fullScreen} onCheckedChange={value => actions.setFullScreen(id, value)} />
          </PreferenceRow>
        </PreferenceSection>
        <PreferenceSection label="Focus">
          <PreferenceRow title="Focused" subtitle={focused ? 'The key window: it gets the keyboard' : 'No'}>
            <div className="example-panel__actions">
              <Button size="small" variant="normal" onClick={() => actions.focus(id)}>
                Focus
              </Button>
              <Button size="small" variant="normal" onClick={() => actions.blur(id)}>
                Blur
              </Button>
            </div>
          </PreferenceRow>
        </PreferenceSection>
      </PanelPreferences>
    </Panel>
  )
}
