import { ArrowUndo16Regular } from '@fluentui/react-icons'
import { useState } from 'react'

import { Badge, Button, Callout, FormField, Icon, TextField } from '@dazzlabs/dazzui'

import { Panel } from '../../../components/panel'
import { ReadBack } from '../../../components/read-back'
import { MACOS_GLOBAL_KEYS } from '../data'
import type { StorageState } from '../use-storage'
import './panels.css'

export interface LookupPanelProps {
  state: StorageState
}

/**
 * One key at a time — `get` with its default, `contains`, `remove` (with
 * Undo) — and the whole store: `getKeys`, `getSize`, `getAll`. What the last
 * call returned is read back under the buttons.
 */
export function LookupPanel({ state }: LookupPanelProps) {
  const { actions, lookup, removed, info, os } = state
  const [key, setKey] = useState('theme')
  const [fallback, setFallback] = useState('')
  const merged = os === 'macos' && info.kind === 'preferences'
  const k = key.trim()

  return (
    <Panel>
      <form
        className="storage-lookup__fields"
        onSubmit={event => {
          event.preventDefault()
          if (k) actions.get(k, fallback)
        }}
      >
        <FormField label="Key">
          <TextField size="small" mono value={key} placeholder="theme" onChange={e => setKey(e.target.value)} />
        </FormField>
        <FormField label="Default value">
          <TextField
            size="small"
            mono
            value={fallback}
            placeholder='""'
            onChange={e => setFallback(e.target.value)}
          />
        </FormField>
      </form>
      <div className="example-panel__actions">
        <Button size="small" variant="filled" disabled={!k} onClick={() => actions.get(k, fallback)}>
          Get
        </Button>
        <Button size="small" variant="normal" disabled={!k} onClick={() => actions.contains(k)}>
          Contains
        </Button>
        <Button size="small" variant="normal" tint="danger" disabled={!k} onClick={() => actions.remove(k)}>
          Remove
        </Button>
        <span className="storage-lookup__divider" />
        <Button size="small" variant="normal" onClick={actions.listKeys}>
          List keys
        </Button>
        <Button size="small" variant="normal" onClick={actions.getSize}>
          Get size
        </Button>
        <Button size="small" variant="normal" onClick={actions.getAll}>
          Get all
        </Button>
      </div>

      {removed && (
        <Callout
          size="small"
          tint="neutral"
          title={`Removed "${removed.key}"`}
          action={
            <Button size="small" variant="normal" onClick={actions.undoRemove}>
              <Icon icon={ArrowUndo16Regular} />
              Undo
            </Button>
          }
        >
          It held "{removed.value.length > 32 ? `${removed.value.slice(0, 32)}…` : removed.value}"; Undo sets it again.
        </Callout>
      )}

      <ReadBack
        label="Last call"
        columns={1}
        keyWidth="6rem"
        action={
          lookup && (
            <Badge size="small" variant="tinted" tint={lookup.ok ? 'success' : 'neutral'}>
              {lookup.ok ? 'Found' : 'Not found'}
            </Badge>
          )
        }
        rows={
          lookup
            ? [
                ['call', lookup.call],
                ['returned', <span className="storage-lookup__result">{lookup.result}</span>],
              ]
            : [['call', '—']]
        }
      />

      <ReadBack
        label="This store"
        columns={2}
        keyWidth="6.5rem"
        rows={[
          ['getScope()', `"${info.scope}"`],
          ['getSize()', String(Object.keys(state.mergedView(info, state.entries)).length)],
          ['own entries', String(Object.keys(state.entries).length)],
          info.kind === 'secure' ? ['isAvailable()', String(state.secureAvailable)] : ['encrypted', 'false'],
        ]}
      />

      {merged && (
        <Callout size="small" tint="warning" title="macOS reports the merged NSUserDefaults view">
          getKeys(), getSize() and getAll() read the suite's dictionaryRepresentation, which also holds{' '}
          {MACOS_GLOBAL_KEYS.length} NSGlobalDomain keys such as AppleLocale — and clear() tries to remove them too. The
          Entries tab shows the suite's own domain.
        </Callout>
      )}
    </Panel>
  )
}
