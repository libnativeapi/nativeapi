import { Delete20Regular, Search16Regular } from '@fluentui/react-icons'
import { useState } from 'react'

import {
  Button,
  Icon,
  PreferenceGroup,
  PreferenceRow,
  PreferenceSection,
  SegmentedControl,
  Switch,
  TextField,
} from '@dazzlabs/dazzui'

import { Panel, PanelPreferences } from '../../../components/panel'
import type { Os } from '../../../components/platform'
import { ReadBack } from '../../../components/read-back'
import { SCOPES } from '../data'
import type { Scope, ShortcutEntry } from '../types'
import type { ShortcutActions } from '../use-shortcuts'
import { Chord } from './chord'
import './panes.css'

export interface ManagerPaneProps {
  os: Os
  shortcuts: ShortcutEntry[]
  managerEnabled: boolean
  focused: boolean
  wayland: boolean
  actions: ShortcutActions
}

/** `ShortcutManager` as a whole: on or off, what it holds, and the lookups. */
export function ManagerPane({ os, shortcuts, managerEnabled, focused, wayland, actions }: ManagerPaneProps) {
  const [query, setQuery] = useState('Ctrl+Shift+A')
  const [found, setFound] = useState<ShortcutEntry | null | undefined>(undefined)
  const [scope, setScope] = useState<Scope>('global')
  const byScope = shortcuts.filter(s => s.scope === scope)

  const lookUp = () => {
    const hit = shortcuts.find(s => s.accelerator === query) ?? null
    actions.read(`get("${query}") → ${hit ? `#${hit.id}` : 'null'}`)
    setFound(hit)
  }

  return (
    <Panel>
      <PanelPreferences>
        <PreferenceSection>
          <PreferenceGroup title="Manager">
            <PreferenceRow title="Shortcut processing" subtitle="setEnabled(false) keeps every shortcut registered, and silent">
              <Switch checked={managerEnabled} onCheckedChange={actions.setManagerEnabled} />
            </PreferenceRow>
            <PreferenceRow
              title="App has focus"
              subtitle={wayland ? 'On Hyprland the X11 grab only fires while it does' : 'Application-scope shortcuts only fire while it does'}
            >
              <Switch checked={focused} onCheckedChange={actions.setFocused} />
            </PreferenceRow>
            <PreferenceRow title="Unregister all" subtitle="Returns how many it removed">
              <Button size="small" variant="normal" tint="danger" disabled={shortcuts.length === 0} onClick={actions.unregisterAll}>
                <Icon icon={Delete20Regular} />
                Unregister all
              </Button>
            </PreferenceRow>
          </PreferenceGroup>
          <PreferenceGroup title="Look up">
            <PreferenceRow title="get(accelerator)">
              <form
                className="shortcut-pane__inline"
                onSubmit={e => {
                  e.preventDefault()
                  lookUp()
                }}
              >
                <TextField
                  size="small"
                  mono
                  aria-label="Accelerator to look up"
                  value={query}
                  onChange={e => {
                    setQuery((e.target as HTMLInputElement).value)
                    setFound(undefined)
                  }}
                />
                <Button size="small" variant="normal" type="submit">
                  <Icon icon={Search16Regular} />
                  Get
                </Button>
              </form>
            </PreferenceRow>
            <PreferenceRow
              title="getByScope"
              subtitle={byScope.length ? byScope.map(s => s.accelerator).join(', ') : 'None in this scope'}
            >
              <SegmentedControl<Scope>
                size="small"
                items={SCOPES}
                value={scope}
                onValueChange={next => {
                  setScope(next)
                  actions.read(`getByScope(${next === 'global' ? 'Global' : 'Application'}) → ${shortcuts.filter(s => s.scope === next).length}`)
                }}
              />
            </PreferenceRow>
          </PreferenceGroup>
        </PreferenceSection>
      </PanelPreferences>
      {found !== undefined && (
        <p className="example-panel__note shortcut-pane__found">
          get("{query}") →{' '}
          {found ? (
            <>
              #{found.id} <Chord accelerator={found.accelerator} os={os} />
            </>
          ) : (
            'null'
          )}
        </p>
      )}
      <ReadBack
        keyWidth="9.5rem"
        rows={[
          ['isSupported', 'true'],
          ['isEnabled', String(managerEnabled)],
          ['getAll', `${shortcuts.length}`],
          ['getByScope(Global)', String(shortcuts.filter(s => s.scope === 'global').length)],
          ['getByScope(App)', String(shortcuts.filter(s => s.scope === 'application').length)],
        ]}
      />
    </Panel>
  )
}
