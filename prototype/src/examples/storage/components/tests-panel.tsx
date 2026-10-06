import { CheckmarkCircle16Filled, Circle16Regular, DismissCircle16Filled, Play16Regular } from '@fluentui/react-icons'

import { Badge, Button, Icon, PreferenceGroup, PreferenceRow, PreferenceSection } from '@dazzlabs/dazzui'

import { Panel, PanelPreferences } from '../../../components/panel'
import { TESTS } from '../data'
import type { TestId, TestResult, TestStatus } from '../types'
import './panels.css'

export interface TestsPanelProps {
  tests: Record<TestId, TestResult>
  storeLabel: string
  onRun: (id: TestId) => void
  onRunAll: () => void
  onReset: () => void
}

const STATUS_ICON = {
  pass: <Icon icon={CheckmarkCircle16Filled} className="storage-panel__pass" />,
  fail: <Icon icon={DismissCircle16Filled} className="storage-panel__fail" />,
  open: <Icon icon={Circle16Regular} className="storage-panel__open" />,
} satisfies Record<TestStatus, unknown>

/**
 * The example's nine test cases, each a row with Run and what it found. The
 * first seven act on the selected store; the last two on every store at once.
 */
export function TestsPanel({ tests, storeLabel, onRun, onRunAll, onReset }: TestsPanelProps) {
  const results = Object.values(tests)
  const passed = results.filter(r => r.status === 'pass').length
  const failed = results.filter(r => r.status === 'fail').length

  const row = (test: (typeof TESTS)[number]) => {
    const result = tests[test.id]
    return (
      <PreferenceRow
        key={test.id}
        icon={STATUS_ICON[result.status]}
        title={test.name}
        subtitle={
          result.status === 'open' ? (
            test.does
          ) : (
            <span className="storage-panel__detail" data-status={result.status}>
              {result.detail}
            </span>
          )
        }
      >
        <Button size="small" variant="normal" onClick={() => onRun(test.id)}>
          <Icon icon={Play16Regular} />
          {result.status === 'open' ? 'Run' : 'Run again'}
        </Button>
      </PreferenceRow>
    )
  }

  return (
    <Panel>
      <div className="storage-panel__head">
        <span className="storage-panel__summary">
          {passed} pass · {failed} fail · {results.length - passed - failed} not run
        </span>
        {failed > 0 && (
          <Badge size="small" variant="tinted" tint="danger">
            {failed} failed
          </Badge>
        )}
        <Button size="small" variant="plain" disabled={passed + failed === 0} onClick={onReset}>
          Reset
        </Button>
        <Button size="small" variant="filled" onClick={onRunAll}>
          Run all
        </Button>
      </div>
      <PanelPreferences dense>
        <PreferenceSection>
          <PreferenceGroup title={`On ${storeLabel}`}>{TESTS.filter(t => !t.all).map(row)}</PreferenceGroup>
          <PreferenceGroup title="Across stores">{TESTS.filter(t => t.all).map(row)}</PreferenceGroup>
        </PreferenceSection>
      </PanelPreferences>
    </Panel>
  )
}
