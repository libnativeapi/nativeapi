import { Delete20Regular, Flash20Regular } from '@fluentui/react-icons'
import { useState } from 'react'

import {
  Badge,
  Button,
  Card,
  Icon,
  PreferenceGroup,
  PreferenceRow,
  PreferenceSection,
  Switch,
  TextField,
} from '@dazzlabs/dazzui'

import { Panel, PanelPreferences } from '../../../components/panel'
import type { Os } from '../../../components/platform'
import { ReadBack } from '../../../components/read-back'
import { SCOPE_NAMES } from '../data'
import type { ShortcutEntry } from '../types'
import type { ShortcutActions } from '../use-shortcuts'
import { Chord } from './chord'
import './panes.css'

export interface ShortcutPaneProps {
  os: Os
  entry: ShortcutEntry
  /** Bumped on every activation: the header flashes again. */
  flash: number
  managerEnabled: boolean
  focused: boolean
  wayland: boolean
  actions: ShortcutActions
}

/** One registered `Shortcut`: press it, or drive it from here. */
export function ShortcutPane({ os, entry, flash, managerEnabled, focused, wayland, actions }: ShortcutPaneProps) {
  const [description, setDescription] = useState(entry.description)
  const quiet = !managerEnabled
    ? 'The manager is disabled: nothing fires'
    : !entry.enabled
      ? 'Disabled: it stays registered, and does not fire'
      : !focused && entry.scope === 'application'
        ? 'Application scope: it waits for the app to have the focus'
        : !focused && wayland
          ? 'A Wayland window has the focus: the X11 grab does not fire'
          : `Press ${entry.accelerator} anywhere on the page`

  return (
    <Panel>
      <Card variant="outlined" size="small" className="shortcut-pane__hero">
        <div key={flash} className="shortcut-pane__chord" data-flash={flash > 0 ? '' : undefined}>
          <Chord accelerator={entry.accelerator} os={os} size="large" />
        </div>
        <div className="shortcut-pane__hero-text">
          <strong>{entry.description || 'No description'}</strong>
          <span>{quiet}</span>
        </div>
        <div className="shortcut-pane__counters">
          <Badge size="small" variant={entry.activations ? 'tinted' : 'outlined'} tint={entry.activations ? 'primary' : 'neutral'}>
            Activated {entry.activations}
          </Badge>
          <Badge size="small" variant={entry.calls ? 'tinted' : 'outlined'} tint={entry.calls ? 'success' : 'neutral'}>
            Callback {entry.calls}
          </Badge>
        </div>
      </Card>

      <PanelPreferences>
        <PreferenceSection>
          <PreferenceGroup title="Shortcut">
            <PreferenceRow title="Enabled" subtitle="setEnabled: off keeps it registered">
              <Switch checked={entry.enabled} onCheckedChange={enabled => actions.setEnabled(entry.id, enabled)} />
            </PreferenceRow>
            <PreferenceRow title="Description" subtitle="setDescription">
              <form
                className="shortcut-pane__inline"
                onSubmit={e => {
                  e.preventDefault()
                  actions.setDescription(entry.id, description)
                }}
              >
                <TextField
                  size="small"
                  aria-label="Description"
                  placeholder="What it does"
                  value={description}
                  onChange={e => setDescription((e.target as HTMLInputElement).value)}
                />
                <Button size="small" variant="normal" type="submit" disabled={description === entry.description}>
                  Apply
                </Button>
              </form>
            </PreferenceRow>
            <PreferenceRow title="Invoke" subtitle="Runs the callback directly: no ShortcutActivatedEvent">
              <Button size="small" variant="normal" onClick={() => actions.invoke(entry.id)}>
                <Icon icon={Flash20Regular} />
                Invoke
              </Button>
            </PreferenceRow>
            <PreferenceRow title="Unregister" subtitle="By id, or by its accelerator">
              <Button size="small" variant="normal" tint="danger" onClick={() => actions.unregister(entry.id)}>
                <Icon icon={Delete20Regular} />
                By id
              </Button>
              <Button size="small" variant="plain" tint="danger" onClick={() => actions.unregister(entry.id, 'accelerator')}>
                By accelerator
              </Button>
            </PreferenceRow>
          </PreferenceGroup>
        </PreferenceSection>
      </PanelPreferences>

      <ReadBack
        keyWidth="7.5rem"
        rows={[
          ['getId', String(entry.id)],
          ['getAccelerator', `"${entry.accelerator}"`],
          ['getDescription', `"${entry.description}"`],
          ['getScope', SCOPE_NAMES[entry.scope].replace('ShortcutScope::', '')],
          ['isEnabled', String(entry.enabled)],
          ['get(accel)', `#${entry.id}`],
        ]}
      />
    </Panel>
  )
}
