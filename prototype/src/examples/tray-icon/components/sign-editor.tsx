import { useState } from 'react'

import { Button, PreferenceGroup, PreferenceRow, PreferenceSection, SectionLabel, SegmentedControl, Switch, TextField } from '@dazzlabs/dazzui'

import { PanelPreferences } from '../../../components/panel'
import { contentOf, templateOf, validateContent } from '../sign-data'
import type { SignContent, SignEntry } from '../sign-types'

export interface SignEditorActions {
  content: (content: SignContent) => void
  restore: () => void
  option: (option: 'green' | 'english' | 'right', value: boolean) => void
}

export function SignEditor({ entry, actions }: { entry: SignEntry; actions: SignEditorActions }) {
  const [draft, setDraft] = useState(() => ({ ...contentOf(entry) }))
  const [error, setError] = useState<string | null>(null)
  const template = templateOf(entry.style)
  const apply = () => {
    const content = { primary: draft.primary.trim(), secondary: draft.secondary.trim(), distance: draft.distance.trim() }
    const problem = validateContent(entry.style, content)
    setError(problem)
    if (!problem) actions.content(content)
  }

  return <>
    <form className="tray-sign-panel__fields" onSubmit={event => { event.preventDefault(); apply() }}>
      <SectionLabel>Sign content</SectionLabel>
      {template.fields.map((field, index) => (
        <label key={field} className="tray-sign-panel__field">
          <span>{template.labels[index]}</span>
          <TextField value={draft[field]} aria-label={template.labels[index]} aria-invalid={!!error}
            onChange={event => setDraft(current => ({ ...current, [field]: event.target.value }))} />
        </label>
      ))}
      {error && <p className="tray-sign-panel__error" role="alert">{error}</p>}
      <div className="tray-sign-panel__actions">
        <Button variant="filled" size="small" type="submit">Apply sign</Button>
        <Button variant="normal" size="small" onClick={() => { actions.restore(); setDraft({ ...template.defaults }); setError(null) }}>Reset example</Button>
      </div>
    </form>
    <PanelPreferences>
      <PreferenceSection>
        <PreferenceGroup title="Display options">
          {entry.style === 'missing' && <PreferenceRow title="Sign color">
            <SegmentedControl size="small" value={entry.green ? 'green' : 'blue'}
              items={[{ value: 'blue', label: 'Blue' }, { value: 'green', label: 'Green' }]}
              onValueChange={value => actions.option('green', value === 'green')} />
          </PreferenceRow>}
          {entry.style !== 'welcome' && <PreferenceRow title={entry.style === 'missing' ? 'Cardinal directions' : 'Upper arrow direction'}>
            <SegmentedControl size="small" value={entry.right ? 'right' : 'left'}
              items={[{ value: 'left', label: entry.style === 'missing' ? 'W → E' : 'Left' }, { value: 'right', label: entry.style === 'missing' ? 'E → W' : 'Right' }]}
              onValueChange={value => actions.option('right', value === 'right')} />
          </PreferenceRow>}
          {(entry.style === 'welcome' || entry.style === 'guide') && <PreferenceRow title="Show English subtitle">
            <Switch checked={entry.english} onCheckedChange={value => actions.option('english', value)} />
          </PreferenceRow>}
        </PreferenceGroup>
      </PreferenceSection>
    </PanelPreferences>
  </>
}
