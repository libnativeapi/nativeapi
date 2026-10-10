import { ArrowSync16Regular, Edit16Regular, WindowArrowUp20Regular } from '@fluentui/react-icons'
import { useState } from 'react'

import {
  Badge,
  Button,
  Card,
  Dialog,
  DialogBody,
  DialogFooter,
  DialogHeader,
  Icon,
  IconButton,
  PreferenceGroup,
  PreferenceRow,
  Preferences,
  PreferenceSection,
  SectionLabel,
  SegmentedControl,
  Switch,
  TextField,
} from '@dazzlabs/dazzui'

import { BACKENDS, TITLE_PRESETS, TOOLTIP_PRESETS, TRIGGERS } from '../data'
import type { Capabilities, MenuBackend, Trigger, TrayEntry } from '../types'
import { boundsOf, titleOf, type TrayActions, type TrayState } from '../use-tray'
import { useNow } from '../../../components/use-now'
import './panels.css'

export interface PropertiesPanelProps {
  entry: TrayEntry
  caps: Capabilities
  state: TrayState
  actions: TrayActions
}

const quote = (s: string | null) => (s === null ? 'null' : `"${s.replaceAll('\n', '\\n')}"`)

/** The preset a value matches, or none: a value typed in is no preset. */
const presetOf = (presets: readonly { label: string; value: string | null }[], value: string | null) =>
  presets.find(p => (p.value ?? '') === (value ?? ''))?.label ?? ''

/**
 * One row per `TrayIcon` API. The card on top never shows what was written:
 * it shows what the native getters return afterwards, and counts the click
 * events as they arrive.
 */
export function PropertiesPanel({ entry, caps, state, actions }: PropertiesPanelProps) {
  const [editing, setEditing] = useState<'title' | 'tooltip' | null>(null)
  const now = useNow(250, entry.scene === 'download' || entry.scene === 'recording')
  const title = caps.title ? titleOf(entry, now) : null

  const counter = (label: string, count: number) => (
    <Badge size="small" variant={count > 0 ? 'tinted' : 'outlined'} tint={count > 0 ? 'primary' : 'neutral'}>
      {label} {count}
    </Badge>
  )

  const bounds = caps.bounds ? boundsOf(entry, state.entries, caps) : 'unsupported'

  const readBack: [string, string][] = [
    ['id', String(entry.id)],
    ['visible', String(entry.visible)],
    ['trigger', entry.trigger],
    ['title', caps.title ? quote(title) : 'null · Windows'],
    ['tooltip', quote(entry.tooltip)],
    ['bounds', bounds],
    ['supported', 'true'],
    ['getAll', String(state.entries.length)],
  ]

  return (
    <div className="tray-panel">
      <Card variant="outlined" size="small" className="tray-panel__readback">
        <div className="tray-panel__readback-head">
          <SectionLabel>Read back from getters</SectionLabel>
          <div className="tray-panel__counters">
            {counter('Left', entry.clicks)}
            {counter('Right', entry.rightClicks)}
            {counter('Double', entry.doubleClicks)}
            <Button size="tiny" variant="plain" onClick={actions.resetCounters}>
              Reset
            </Button>
          </div>
          <Button size="small" variant="plain" disabled={!caps.bounds} onClick={actions.moveWindowToIcon}>
            <Icon icon={WindowArrowUp20Regular} />
            Window to icon
          </Button>
          <IconButton label="Refresh" size="small" variant="plain" onClick={actions.refresh}>
            <Icon icon={ArrowSync16Regular} />
          </IconButton>
        </div>
        <dl className="tray-panel__readback-grid">
          {readBack.map(([key, value]) => (
            <div key={key}>
              <dt>{key}</dt>
              <dd>{value}</dd>
            </div>
          ))}
        </dl>
      </Card>

      <Preferences className="tray-panel__preferences">
        <PreferenceSection>
          <PreferenceGroup title="Icon">
            <PreferenceRow title="Title" subtitle={caps.title ? undefined : 'Windows draws no titles'}>
              <SegmentedControl
                size="small"
                disabled={!caps.title}
                items={TITLE_PRESETS.map(p => ({ value: p.label, label: p.label }))}
                value={presetOf(TITLE_PRESETS, title)}
                onValueChange={label => actions.setTitle(TITLE_PRESETS.find(p => p.label === label)!.value)}
              />
              <IconButton label="Edit title" size="small" disabled={!caps.title} onClick={() => setEditing('title')}>
                <Icon icon={Edit16Regular} />
              </IconButton>
            </PreferenceRow>
            <PreferenceRow title="Tooltip">
              <SegmentedControl
                size="small"
                items={TOOLTIP_PRESETS.map(p => ({ value: p.label, label: p.label }))}
                value={presetOf(TOOLTIP_PRESETS, entry.tooltip)}
                onValueChange={label => actions.setTooltip(TOOLTIP_PRESETS.find(p => p.label === label)!.value)}
              />
              <IconButton label="Edit tooltip" size="small" onClick={() => setEditing('tooltip')}>
                <Icon icon={Edit16Regular} />
              </IconButton>
            </PreferenceRow>
            <PreferenceRow title="Visible">
              <Switch checked={entry.visible} onCheckedChange={actions.setVisible} />
            </PreferenceRow>
          </PreferenceGroup>
          <PreferenceGroup title="Menu">
            <PreferenceRow title="Trigger">
              <SegmentedControl<Trigger>
                size="small"
                items={TRIGGERS}
                value={entry.trigger}
                onValueChange={actions.setTrigger}
              />
            </PreferenceRow>
            <PreferenceRow title="Popup" subtitle="Click shows the window, blur hides it">
              <Switch checked={state.popupMode} onCheckedChange={actions.setPopupMode} />
            </PreferenceRow>
            <PreferenceRow
              title="Open from code"
              subtitle={caps.openMenu ? undefined : 'Only the shell opens it on Linux'}
            >
              <Button size="small" variant="normal" disabled={!caps.openMenu} onClick={() => actions.openMenu()}>
                Open
              </Button>
              <Button size="small" variant="normal" disabled={!caps.openMenu} onClick={() => actions.openMenu(2000)}>
                Open, close in 2 s
              </Button>
            </PreferenceRow>
            {caps.backend && (
              <PreferenceRow title="Backend">
                <SegmentedControl<MenuBackend>
                  size="small"
                  items={BACKENDS}
                  value={state.backend}
                  onValueChange={actions.setBackend}
                />
              </PreferenceRow>
            )}
          </PreferenceGroup>
        </PreferenceSection>
      </Preferences>

      {editing && (
        <TextDialog
          title={editing === 'title' ? 'Title' : 'Tooltip'}
          initial={(editing === 'title' ? title : entry.tooltip) ?? ''}
          multiline={editing === 'tooltip'}
          onClose={() => setEditing(null)}
          onApply={value => (editing === 'title' ? actions.setTitle(value || null) : actions.setTooltip(value || null))}
        />
      )}
    </div>
  )
}

interface TextDialogProps {
  title: string
  initial: string
  multiline: boolean
  onClose: () => void
  onApply: (value: string) => void
}

/** A one-field dialog, for values the presets don't cover. */
function TextDialog({ title, initial, multiline, onClose, onApply }: TextDialogProps) {
  const [value, setValue] = useState(initial)
  const apply = () => {
    onApply(value)
    onClose()
  }
  return (
    <Dialog open onOpenChange={open => !open && onClose()} width={360}>
      <DialogHeader title={title} />
      <DialogBody>
        {multiline ? (
          <TextField multiline rows={3} autoFocus value={value} onChange={event => setValue(event.target.value)} />
        ) : (
          <TextField
            autoFocus
            value={value}
            onChange={event => setValue(event.target.value)}
            onKeyDown={event => event.key === 'Enter' && apply()}
          />
        )}
      </DialogBody>
      <DialogFooter>
        <Button variant="normal" onClick={onClose}>
          Cancel
        </Button>
        <Button variant="filled" onClick={apply}>
          Apply
        </Button>
      </DialogFooter>
    </Dialog>
  )
}
