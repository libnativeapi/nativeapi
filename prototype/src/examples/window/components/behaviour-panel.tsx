import { PreferenceRow, PreferenceSection, Switch } from '@dazzlabs/dazzui'

import { Panel, PanelPreferences } from '../../../components/panel'
import { osOf } from '../../../components/platform'
import { BEHAVIOUR } from '../data'
import type { BehaviourFlag, SimWindow } from '../types'
import { buttonsVisibleOf, unsupportedOf } from '../use-windows'
import type { WindowPanelProps } from './state-panel'
import './panels.css'

const GROUPS = ['Stacking', 'Capabilities', 'Platform specific'] as const

/**
 * Stacking, what the user may do to the window, and the platform switches:
 * one switch per setter, showing its getter. Where a setter is not
 * implemented the switch is greyed out and says so.
 */
export function BehaviourPanel({ w, platform, actions }: WindowPanelProps) {
  const os = osOf(platform)
  const value = (flag: BehaviourFlag, window: SimWindow) =>
    flag === 'controlButtonsVisible' ? buttonsVisibleOf(window, os) : window[flag]

  return (
    <Panel>
      <PanelPreferences dense>
        {GROUPS.map(group => (
          <PreferenceSection key={group} label={group}>
            {BEHAVIOUR.filter(b => b.group === group).map(b => {
              const unsupported = unsupportedOf(b.flag, os)
              return (
                <PreferenceRow
                  key={b.flag}
                  title={b.label}
                  subtitle={unsupported ?? `${b.getter} → ${value(b.flag, w)}${b.note ? ` · ${b.note}` : ''}`}
                >
                  <Switch
                    size="small"
                    disabled={Boolean(unsupported)}
                    checked={value(b.flag, w)}
                    onCheckedChange={v => actions.setFlag(w.id, b.flag, v)}
                  />
                </PreferenceRow>
              )
            })}
          </PreferenceSection>
        ))}
      </PanelPreferences>
    </Panel>
  )
}
