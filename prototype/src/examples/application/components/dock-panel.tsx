import { useState } from 'react'

import {
  Card,
  OptionCard,
  PreferenceGroup,
  PreferenceRow,
  PreferenceSection,
  SegmentedControl,
  Slider,
  Switch,
  TextField,
  type WindowFramePlatform,
} from '@dazzlabs/dazzui'

import { Panel, PanelPreferences, PanelTiles } from '../../../components/panel'
import { osOf } from '../../../components/platform'
import { ReadBack } from '../../../components/read-back'
import { BADGE_PRESETS, DOCK_SURFACE, ICON_PRESETS } from '../data'
import type { ApplicationActions, ApplicationState } from '../use-application'
import { AppIcon } from './dock'
import './panels.css'

export interface DockPanelProps {
  platform: WindowFramePlatform
  state: ApplicationState
  actions: ApplicationActions
}

type ProgressMode = 'none' | 'value' | 'busy'

const SURFACE_NAMES = {
  dock: 'Dock tile',
  'side-dock': 'Ubuntu Dock (LauncherEntry)',
  taskbar: 'Taskbar button',
  none: 'No launcher draws it',
} as const

/**
 * What the app shows on its icon in the dock or the taskbar: a badge, a
 * progress bar, the icon itself, and on macOS whether it has a Dock icon at
 * all. Every change is drawn on the desktop at once.
 */
export function DockPanel({ platform, state, actions }: DockPanelProps) {
  const os = osOf(platform)
  const surface = DOCK_SURFACE[platform]
  const [custom, setCustom] = useState('')
  const [value, setValue] = useState(state.progress >= 0 && state.progress <= 1 ? state.progress : 0.4)
  const mode: ProgressMode = state.progress < 0 ? 'none' : state.progress > 1 ? 'busy' : 'value'

  const badgeNote = {
    macos: 'The Dock badge: any text',
    windows: 'Overlay icon on the primary window: 3 characters',
    linux: 'A LauncherEntry count: text returns false',
  }[os]
  const progressNote = {
    macos: 'Drawn on the Dock tile; above 1 is busy',
    windows: 'On the primary window’s button; above 1 is busy',
    linux: 'No busy state: above 1 shows full',
  }[os]

  return (
    <Panel>
      <Card variant="sunken" size="small" className="application-panel__preview">
        <span className="application-panel__preview-tile">
          <AppIcon preset={state.icon} size={44} />
          {state.badge && <span className="application-panel__preview-badge">{state.badge}</span>}
        </span>
        <div className="application-panel__hero-text">
          <strong>{SURFACE_NAMES[surface]}</strong>
          <span>
            badge {state.badge ? `"${state.badge}"` : 'none'} · progress{' '}
            {state.progress < 0 ? 'none' : state.progress > 1 ? 'busy' : `${Math.round(state.progress * 100)}%`}
            {surface === 'none' && ' · Waybar shows no badges: the signal is sent, nothing draws it'}
          </span>
        </div>
      </Card>

      <PanelPreferences>
        <PreferenceSection>
          <PreferenceGroup title="Badge">
            <PreferenceRow title="setBadgeLabel" subtitle={badgeNote}>
              <SegmentedControl
                size="small"
                items={BADGE_PRESETS.map(p => ({ value: p.label, label: p.label }))}
                value={BADGE_PRESETS.find(p => p.value === state.badge)?.label ?? ''}
                onValueChange={label => actions.setBadgeLabel(BADGE_PRESETS.find(p => p.label === label)!.value)}
              />
              <form
                onSubmit={event => {
                  event.preventDefault()
                  actions.setBadgeLabel(custom)
                }}
              >
                <TextField
                  size="small"
                  aria-label="Badge text"
                  placeholder="Text…"
                  className="application-panel__badge-field"
                  value={custom}
                  onChange={event => setCustom((event.target as HTMLInputElement).value)}
                />
              </form>
            </PreferenceRow>
          </PreferenceGroup>
          <PreferenceGroup title="Progress">
            <PreferenceRow title="setProgressBar" subtitle={progressNote}>
              <SegmentedControl<ProgressMode>
                size="small"
                items={[
                  { value: 'none', label: 'None' },
                  { value: 'value', label: 'Value' },
                  { value: 'busy', label: 'Busy' },
                ]}
                value={mode}
                onValueChange={next => actions.setProgressBar(next === 'none' ? -1 : next === 'busy' ? 2 : value)}
              />
            </PreferenceRow>
            <PreferenceRow title="Fraction" subtitle={mode === 'value' ? undefined : 'Pick Value to set a fraction'}>
              <Slider
                size="small"
                className="application-panel__slider"
                disabled={mode !== 'value'}
                min={0}
                max={1}
                step={0.01}
                value={value}
                showValue
                formatValue={values => `${Math.round(values[0]! * 100)}%`}
                onValueChange={next => {
                  const v = next as number
                  setValue(v)
                  actions.setProgressBar(v, false)
                }}
                onValueCommitted={next => actions.setProgressBar(next as number)}
              />
            </PreferenceRow>
          </PreferenceGroup>
          <PreferenceGroup title="Icon">
            <PanelTiles columns={5}>
              {ICON_PRESETS.map(preset => (
                <OptionCard
                  key={preset.value}
                  className="example-panel__tile"
                  selected={state.icon === preset.value}
                  onClick={() => actions.setIcon(preset.value)}
                  title={
                    <span className="example-panel__tile-content">
                      {preset.value === 'missing' ? (
                        <span className="application-panel__missing">?</span>
                      ) : (
                        <AppIcon preset={preset.value} size={24} />
                      )}
                      {preset.label}
                    </span>
                  }
                />
              ))}
            </PanelTiles>
            <PreferenceRow
              title="Dock icon"
              subtitle={os === 'macos' ? 'Hidden: no Dock tile, no app switcher entry' : 'macOS only: elsewhere a no-op'}
            >
              <Switch
                disabled={os !== 'macos'}
                checked={state.dockIconVisible}
                onCheckedChange={actions.setDockIconVisible}
              />
            </PreferenceRow>
          </PreferenceGroup>
        </PreferenceSection>
      </PanelPreferences>

      <ReadBack
        label="Returned by the last calls"
        keyWidth="9rem"
        rows={(['setBadgeLabel', 'setProgressBar', 'setIcon', 'setDockIconVisible'] as const).map(name => [
          name,
          state.returns[name] === undefined ? '—' : String(state.returns[name]),
        ])}
      />
    </Panel>
  )
}
