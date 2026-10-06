import { useState } from 'react'

import { CodeBlock, SectionLabel, SegmentedControl, Step, StepList } from '@dazzlabs/dazzui'

import { Panel } from '../../../components/panel'
import { BUILD_STEPS, COMMANDS, PODFILES, PUBSPEC } from '../data'
import type { Target } from '../types'
import './build-panel.css'

/**
 * How the example is built, and why it exists: Swift Package Manager is off
 * in pubspec.yaml, so Flutter falls back to CocoaPods — the Podfile, the
 * command, and what the build does on the way.
 */
export function BuildPanel({ initialTarget = 'macos' }: { initialTarget?: Target }) {
  const [target, setTarget] = useState<Target>(initialTarget)
  return (
    <Panel>
      <div className="build-panel__head">
        <SectionLabel>Setup</SectionLabel>
        <SegmentedControl<Target>
          size="small"
          items={[
            { value: 'macos', label: 'macOS' },
            { value: 'ios', label: 'iOS' },
          ]}
          value={target}
          onValueChange={setTarget}
        />
      </div>
      <CodeBlock title="pubspec.yaml" language="yaml" code={PUBSPEC} />
      <CodeBlock title={`${target}/Podfile`} language="ruby" code={PODFILES[target]} lineNumbers />
      <CodeBlock title="Build" language="sh" code={COMMANDS[target]} copyable />
      <SectionLabel>What the build does</SectionLabel>
      <StepList>
        {BUILD_STEPS[target].map(step => (
          <Step key={step.label} status="done" label={step.label} meta={step.meta} />
        ))}
      </StepList>
    </Panel>
  )
}
