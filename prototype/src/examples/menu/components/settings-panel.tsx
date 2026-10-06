import { Button, PreferenceRow, PreferenceSection, SegmentedControl } from '@dazzlabs/dazzui'

import { Panel, PanelPreferences } from '../../../components/panel'
import type { Os } from '../../../components/platform'
import type { MenuBackend, ThemeChoice } from '../types'
import type { MenuActions } from '../use-menus'

export interface SettingsPanelProps {
  os: Os
  backend: MenuBackend
  theme: ThemeChoice
  winUi3: boolean
  actions: MenuActions
  onRapid: () => void
  onIssueMenu: () => void
}

const BRIGHTNESS_NOTE: Record<Os, string> = {
  macos: 'setBrightness: NSApp.appearance, for every window and menu',
  windows: 'setBrightness: dark frames on existing windows only',
  linux: 'setBrightness: GTK prefer-dark-theme',
}

/** How the menus are drawn — the backend, the appearance — and two stress runs. */
export function SettingsPanel({ os, backend, theme, winUi3, actions, onRapid, onIssueMenu }: SettingsPanelProps) {
  return (
    <Panel>
      <PanelPreferences>
        <PreferenceSection label="Appearance">
          <PreferenceRow
            title="Backend"
            subtitle={winUi3 ? 'setBackend on both menus · WinUI 3 is roomier' : 'isBackendSupported(kWinUI3) → false · Windows only'}
          >
            <SegmentedControl<MenuBackend>
              size="small"
              items={[
                { value: 'native', label: 'Native' },
                { value: 'winUi3', label: 'WinUI 3', disabled: !winUi3 },
              ]}
              value={backend}
              onValueChange={actions.setBackend}
            />
          </PreferenceRow>
          <PreferenceRow title="Theme" subtitle={BRIGHTNESS_NOTE[os]}>
            <SegmentedControl<ThemeChoice>
              size="small"
              items={[
                { value: 'system', label: 'System' },
                { value: 'light', label: 'Light' },
                { value: 'dark', label: 'Dark' },
              ]}
              value={theme}
              onValueChange={actions.setTheme}
            />
          </PreferenceRow>
        </PreferenceSection>
        <PreferenceSection label="Stress">
          <PreferenceRow title="Add 10 items" subtitle="addItem ten times in a row">
            <Button size="small" variant="normal" onClick={actions.addTen}>
              Add 10
            </Button>
          </PreferenceRow>
          <PreferenceRow title="Rapid open and close" subtitle="The positioning menu, five times, each closed after 100 ms">
            <Button size="small" variant="normal" onClick={onRapid}>
              Run
            </Button>
          </PreferenceRow>
          <PreferenceRow
            title="Checked and disabled (#4)"
            subtitle={
              os === 'windows'
                ? 'Win32 once lost the checkmark and the grey'
                : 'A Windows bug; it should look right here too'
            }
          >
            <Button size="small" variant="normal" onClick={onIssueMenu}>
              Open at (200, 200)
            </Button>
          </PreferenceRow>
        </PreferenceSection>
      </PanelPreferences>
    </Panel>
  )
}
