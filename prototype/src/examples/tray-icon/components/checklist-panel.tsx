import { CheckmarkCircle16Filled, Circle16Regular, Copy16Regular, DismissCircle16Filled } from '@fluentui/react-icons'
import { useState } from 'react'

import {
  Badge,
  Button,
  Icon,
  PreferenceGroup,
  PreferenceRow,
  Preferences,
  PreferenceSection,
  ToggleGroup,
} from '@dazzlabs/dazzui'

import type { Capabilities, CheckItem, CheckStatus } from '../types'
import { checklistReport, type TrayActions } from '../use-tray'
import './panels.css'

export interface ChecklistPanelProps {
  checklist: CheckItem[]
  caps: Capabilities
  actions: TrayActions
}

const STATUS_ICON = {
  pass: <Icon icon={CheckmarkCircle16Filled} style={{ color: 'var(--color-success-500)' }} />,
  fail: <Icon icon={DismissCircle16Filled} style={{ color: 'var(--color-danger-500)' }} />,
  open: <Icon icon={Circle16Regular} style={{ color: 'var(--color-content-faint)' }} />,
} satisfies Record<CheckStatus, unknown>

const MARKS = [
  { value: 'pass', label: 'Pass' },
  { value: 'fail', label: 'Fail' },
] as const

/** Acceptance in one screen: what ticked itself, and what still needs a look at the tray. */
export function ChecklistPanel({ checklist, caps, actions }: ChecklistPanelProps) {
  const [copied, setCopied] = useState(false)
  const passed = checklist.filter(i => i.status === 'pass').length
  const failed = checklist.filter(i => i.status === 'fail').length

  const copy = async () => {
    await navigator.clipboard?.writeText(checklistReport(checklist, caps)).catch(() => {})
    setCopied(true)
    window.setTimeout(() => setCopied(false), 1500)
  }

  const row = (item: CheckItem) => (
    <PreferenceRow key={item.id} icon={STATUS_ICON[item.status]} title={item.label}>
      {item.detail && <span className="tray-panel__detail">{item.detail}</span>}
      {item.note && (
        <Badge size="small" variant="outlined">
          {item.note}
        </Badge>
      )}
      {item.manual && (
        <ToggleGroup<CheckStatus>
          size="tiny"
          items={MARKS.map(m => ({ ...m }))}
          value={item.status === 'open' ? [] : [item.status]}
          onValueChange={values => actions.mark(item.id, values[0] ?? item.status)}
        />
      )}
    </PreferenceRow>
  )

  return (
    <div className="tray-panel">
      <div className="tray-panel__checklist-head">
        <span className="tray-panel__summary">
          {passed} pass · {failed} fail · {checklist.length - passed - failed} open
        </span>
        <Button size="small" variant="normal" onClick={copy}>
          <Icon icon={Copy16Regular} />
          {copied ? 'Copied' : 'Copy report'}
        </Button>
        <Button size="small" variant="plain" onClick={actions.resetChecklist}>
          Reset
        </Button>
      </div>
      <Preferences className="tray-panel__preferences tray-panel__preferences--dense">
        <PreferenceSection>
          <PreferenceGroup title="Auto · ticks itself from events and return values">
            {checklist.filter(i => !i.manual).map(row)}
          </PreferenceGroup>
          <PreferenceGroup title="Manual · look at the tray, then mark it">
            {checklist.filter(i => i.manual).map(row)}
          </PreferenceGroup>
        </PreferenceSection>
      </Preferences>
    </div>
  )
}
