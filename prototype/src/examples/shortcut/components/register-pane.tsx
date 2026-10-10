import { Add20Regular, CheckmarkCircle16Filled, DismissCircle16Filled, Warning16Filled } from '@fluentui/react-icons'
import { useState } from 'react'

import {
  Button,
  Callout,
  Icon,
  PreferenceGroup,
  PreferenceRow,
  PreferenceSection,
  SegmentedControl,
  ShortcutRecorder,
  Switch,
  Tag,
  TextField,
} from '@dazzlabs/dazzui'

import { Panel, PanelPreferences } from '../../../components/panel'
import type { Os } from '../../../components/platform'
import { acceleratorFromGlyphs, simulateIsValidAccelerator, SYNTAX } from '../accelerator'
import { presetsOf, SCOPES, SYSTEM_TAKEN } from '../data'
import type { RegistrationFailure, Scope, ShortcutDraft, ShortcutEntry } from '../types'
import type { ShortcutActions } from '../use-shortcuts'
import './panes.css'

export interface RegisterPaneProps {
  os: Os
  shortcuts: ShortcutEntry[]
  failure: RegistrationFailure | null
  initial?: Partial<ShortcutDraft>
  actions: ShortcutActions
}

/**
 * `register(options)`: the accelerator typed or recorded, checked live as
 * `isValidAccelerator` and `isAvailable` would answer, and the options with it.
 */
export function RegisterPane({ os, shortcuts, failure, initial, actions }: RegisterPaneProps) {
  const [draft, setDraft] = useState<ShortcutDraft>({
    accelerator: initial?.accelerator ?? 'Ctrl+Shift+C',
    description: initial?.description ?? '',
    scope: initial?.scope ?? 'global',
    enabled: initial?.enabled ?? true,
  })
  const set = (patch: Partial<ShortcutDraft>) => setDraft(d => ({ ...d, ...patch }))
  const valid = simulateIsValidAccelerator(draft.accelerator)
  const available = !shortcuts.some(s => s.accelerator === draft.accelerator)
  const heldBy = SYSTEM_TAKEN[os][draft.accelerator]

  const check = (ok: boolean, label: string) => (
    <span className="register-pane__check" data-ok={ok ? '' : undefined}>
      <Icon icon={ok ? CheckmarkCircle16Filled : DismissCircle16Filled} size={14} />
      {label} → {String(ok)}
    </span>
  )

  return (
    <Panel>
      {failure && (
        <Callout
          size="small"
          tint="danger"
          title={`register("${failure.accelerator}") → null`}
          action={
            <Button size="small" variant="plain" onClick={actions.dismissFailure}>
              Dismiss
            </Button>
          }
        >
          ShortcutRegistrationFailedEvent #{failure.id}: “{failure.error}”
          {failure.error === 'Platform registration failed' && SYSTEM_TAKEN[os][failure.accelerator]
            ? ` — ${SYSTEM_TAKEN[os][failure.accelerator]} holds it.`
            : ''}
        </Callout>
      )}
      <div className="register-pane__field">
        <TextField
          mono
          aria-label="Accelerator"
          placeholder="Ctrl+Shift+A"
          state={draft.accelerator && !valid ? 'error' : 'default'}
          value={draft.accelerator}
          onChange={e => set({ accelerator: (e.target as HTMLInputElement).value })}
        />
        <ShortcutRecorder
          aria-label="Record an accelerator"
          value=""
          placeholder="Record"
          recordingLabel="Press keys…"
          onValueChange={glyphs => glyphs && set({ accelerator: acceleratorFromGlyphs(glyphs, os) })}
        />
      </div>
      <div className="register-pane__checks">
        {check(valid, 'isValidAccelerator')}
        {check(available, 'isAvailable')}
        {heldBy && valid && available && (
          <span className="register-pane__check" data-warn="">
            <Icon icon={Warning16Filled} size={14} />
            Held by {heldBy}: only register() finds out
          </span>
        )}
      </div>
      <div className="register-pane__presets">
        {presetsOf(os).map(preset => (
          <Tag
            key={preset.accelerator}
            size="small"
            title={preset.note}
            selected={draft.accelerator === preset.accelerator}
            onSelectedChange={() => set({ accelerator: preset.accelerator })}
          >
            {preset.accelerator}
          </Tag>
        ))}
      </div>

      <PanelPreferences>
        <PreferenceSection>
          <PreferenceGroup title="ShortcutOptions">
            <PreferenceRow title="Description" subtitle="Optional, for lists and help">
              <TextField
                size="small"
                aria-label="Description"
                placeholder="What it does"
                className="register-pane__description"
                value={draft.description}
                onChange={e => set({ description: (e.target as HTMLInputElement).value })}
              />
            </PreferenceRow>
            <PreferenceRow
              title="Scope"
              subtitle={draft.scope === 'global' ? 'Fires whichever app has the focus' : 'Fires only while this app has the focus'}
            >
              <SegmentedControl<Scope> size="small" items={SCOPES} value={draft.scope} onValueChange={scope => set({ scope })} />
            </PreferenceRow>
            <PreferenceRow title="Enabled" subtitle="Registered disabled, it fires once enabled">
              <Switch checked={draft.enabled} onCheckedChange={enabled => set({ enabled })} />
            </PreferenceRow>
          </PreferenceGroup>
        </PreferenceSection>
      </PanelPreferences>

      <div className="example-panel__actions">
        <Button variant="filled" onClick={() => actions.register(draft)}>
          <Icon icon={Add20Regular} />
          Register
        </Button>
        <span className="example-panel__note">{SYNTAX[os]}. Keys: A–Z, 0–9, F1–F24, Space, Tab, Enter, Escape, arrows, punctuation.</span>
      </div>

    </Panel>
  )
}
