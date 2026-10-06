import { Dismiss16Regular, WindowNew20Regular } from '@fluentui/react-icons'

import {
  Badge,
  Button,
  Icon,
  IconButton,
  PreferenceGroup,
  PreferenceRow,
  PreferenceSection,
  SegmentedControl,
  Switch,
} from '@dazzlabs/dazzui'

import { Panel, PanelPreferences } from '../../../components/panel'
import type { Os } from '../../../components/platform'
import { ReadBack } from '../../../components/read-back'
import type { ApplicationActions, ApplicationState } from '../use-application'
import './panels.css'

export interface WindowsPanelProps {
  os: Os
  state: ApplicationState
  actions: ApplicationActions
}

const MENU_NOTES = {
  macos: 'About and Quit in the menu bar at the top of the screen',
  windows: 'A Win32 menu bar on the primary window: false without one',
  linux: 'GTK 3 legacy menus cannot be an app menu bar: always false',
}

/** The app's menu bar and its windows: which one is primary, and how many there are. */
export function WindowsPanel({ os, state, actions }: WindowsPanelProps) {
  return (
    <Panel>
      <PanelPreferences>
        <PreferenceSection>
          <PreferenceGroup title="Menu bar">
            <PreferenceRow title="setMenuBar" subtitle={MENU_NOTES[os]}>
              <Switch checked={state.menuBar} onCheckedChange={actions.setMenuBar} />
            </PreferenceRow>
          </PreferenceGroup>
          <PreferenceGroup
            title="Windows"
            action={
              <Button size="small" variant="plain" onClick={actions.openWindow}>
                <Icon icon={WindowNew20Regular} />
                Open window
              </Button>
            }
          >
            <PreferenceRow title="setPrimaryWindow" subtitle="The taskbar badge and progress go on it on Windows">
              <SegmentedControl
                size="small"
                items={state.windows.map(w => ({ value: String(w.id), label: `#${w.id}` }))}
                value={String(state.primaryId ?? '')}
                onValueChange={id => actions.setPrimaryWindow(Number(id))}
              />
            </PreferenceRow>
            {state.windows.map(w => (
              <PreferenceRow key={w.id} title={`Window #${w.id}`} subtitle={w.title}>
                {w.id === state.primaryId && (
                  <Badge size="small" variant="tinted" tint="primary">
                    Primary
                  </Badge>
                )}
                <IconButton label={`Close window #${w.id}`} size="small" variant="plain" onClick={() => actions.closeWindow(w.id)}>
                  <Icon icon={Dismiss16Regular} />
                </IconButton>
              </PreferenceRow>
            ))}
          </PreferenceGroup>
        </PreferenceSection>
      </PanelPreferences>
      <ReadBack
        keyWidth="8.5rem"
        rows={[
          ['getPrimaryWindow', state.primaryId === null ? 'null' : `window #${state.primaryId}`],
          ['getAllWindows', `[${state.windows.map(w => `#${w.id}`).join(', ')}]`],
          ['setMenuBar', state.returns.setMenuBar === undefined ? String(os !== 'linux') : String(state.returns.setMenuBar)],
        ]}
      />
      <p className="example-panel__note">Closing window #1 quits: the example ends its run with its main window.</p>
    </Panel>
  )
}
