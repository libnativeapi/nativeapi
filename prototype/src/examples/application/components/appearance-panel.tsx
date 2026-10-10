import { WindowNew20Regular } from '@fluentui/react-icons'

import { Button, Callout, Icon, PreferenceGroup, PreferenceRow, PreferenceSection, SegmentedControl } from '@dazzlabs/dazzui'

import { Panel, PanelPreferences } from '../../../components/panel'
import type { Os } from '../../../components/platform'
import { ReadBack } from '../../../components/read-back'
import { BRIGHTNESSES } from '../data'
import type { Brightness } from '../types'
import type { ApplicationActions, ApplicationState } from '../use-application'
import './panels.css'

export interface AppearancePanelProps {
  os: Os
  state: ApplicationState
  actions: ApplicationActions
  /** What a window forced to a brightness looks like right now. */
  appearanceOf: (brightness: Brightness) => 'light' | 'dark'
}

const NOTES = {
  macos: 'Sets the NSApplication appearance: every window and menu inherits it, the ones made later too.',
  windows:
    'Toggles the dark title bar and frame on the windows open now. A window created afterwards keeps the system’s: open one to see.',
  linux:
    'Sets GTK’s prefer-dark-theme; switching to Light while a “-dark” theme is active picks its light variant. System restores both.',
}

/** `setBrightness`: the app forced light or dark, whatever the system says. */
export function AppearancePanel({ os, state, actions, appearanceOf }: AppearancePanelProps) {
  return (
    <Panel>
      <PanelPreferences>
        <PreferenceSection>
          <PreferenceGroup title="Appearance">
            <PreferenceRow title="setBrightness" subtitle="System follows the Appearance toolbar">
              <SegmentedControl<Brightness>
                size="small"
                items={BRIGHTNESSES}
                value={state.brightness}
                onValueChange={actions.setBrightness}
              />
            </PreferenceRow>
            <PreferenceRow title="Another window" subtitle="Made now, after the call">
              <Button size="small" variant="normal" onClick={actions.openWindow}>
                <Icon icon={WindowNew20Regular} />
                Open window
              </Button>
            </PreferenceRow>
          </PreferenceGroup>
        </PreferenceSection>
      </PanelPreferences>
      <Callout size="small" tint={os === 'windows' ? 'warning' : 'info'} title={os === 'windows' ? 'Partial on Windows' : undefined}>
        {NOTES[os]}
      </Callout>
      <ReadBack
        label="What each window is drawn in"
        keyWidth="7rem"
        rows={[
          ['setBrightness', state.returns.setBrightness === undefined ? '—' : String(state.returns.setBrightness)],
          ...state.windows.map(
            w => [`window #${w.id}`, `${appearanceOf(w.brightness)}${w.brightness === 'system' ? ' · system' : ''}`] as const,
          ),
        ]}
      />
    </Panel>
  )
}
