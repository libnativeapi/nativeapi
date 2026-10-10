import { Power20Regular } from '@fluentui/react-icons'
import { useState } from 'react'

import { Badge, Button, Card, Icon, NumberField, PreferenceGroup, PreferenceRow, PreferenceSection } from '@dazzlabs/dazzui'

import { Panel, PanelPreferences } from '../../../components/panel'
import type { Os } from '../../../components/platform'
import { ReadBack } from '../../../components/read-back'
import type { ApplicationActions, ApplicationState } from '../use-application'
import './panels.css'

export interface LifecyclePanelProps {
  os: Os
  state: ApplicationState
  actions: ApplicationActions
}

/**
 * The run itself: `run()` blocking on the loop, `quit()` with the code it
 * returns, and the activation events the desktop raises as the focus moves.
 */
export function LifecyclePanel({ os, state, actions }: LifecyclePanelProps) {
  const [code, setCode] = useState(0)
  const mac = os === 'macos'

  return (
    <Panel>
      <Card variant="outlined" size="small" className="application-panel__hero">
        <span className="application-panel__pulse" data-state={state.quitting ? 'quitting' : 'running'} />
        <div className="application-panel__hero-text">
          <strong>{state.quitting ? 'Quitting…' : 'Running'}</strong>
          <span>run(window #1) is blocking the main thread until quit() stops the loop.</span>
        </div>
        <NumberField
          size="small"
          aria-label="Exit code"
          className="application-panel__code"
          value={code}
          min={-1}
          max={255}
          onValueChange={value => setCode(value ?? 0)}
        />
        <Button size="small" variant="filled" tint="danger" disabled={state.quitting} onClick={() => actions.quit(code)}>
          <Icon icon={Power20Regular} />
          Quit
        </Button>
      </Card>

      <PanelPreferences>
        <PreferenceSection>
          <PreferenceGroup title="Focus">
            <PreferenceRow
              title="Active"
              subtitle="Click the desktop to deactivate; the window, its dock tile or taskbar button to activate"
            >
              <Badge size="small" variant="tinted" tint={state.active ? 'success' : 'neutral'}>
                {state.active ? 'Activated' : 'Deactivated'}
              </Badge>
            </PreferenceRow>
            <PreferenceRow
              title="Hide and show the app"
              subtitle={mac ? 'All windows at once, as Cmd+H; the Dock tile brings them back' : 'No application-level hiding here: both return false'}
            >
              <Button size="small" variant="normal" onClick={actions.hide}>
                Hide
              </Button>
              <Button size="small" variant="normal" onClick={actions.show}>
                Show
              </Button>
            </PreferenceRow>
          </PreferenceGroup>
        </PreferenceSection>
      </PanelPreferences>

      <ReadBack
        keyWidth="8.5rem"
        rows={[
          ['isRunning', String(state.running)],
          ['isSingleInstance', 'false'],
          ['isVisible', String(!state.hidden)],
          ['getPrimaryWindow', state.primaryId === null ? 'null' : `window #${state.primaryId}`],
          ['getAllWindows', `${state.windows.length} window${state.windows.length === 1 ? '' : 's'}`],
          ['hide / show', state.returns.hide === undefined && state.returns.show === undefined ? '—' : `${state.returns.hide ?? '—'} / ${state.returns.show ?? '—'}`],
        ]}
      />
      <p className="example-panel__note">
        quit() emits ApplicationQuitRequestedEvent, then ApplicationExitingEvent, and run() returns the code.
        {mac && ' A quit the user starts (Cmd+Q, logout) emits QuitRequested too, and cannot be vetoed.'}
      </p>
    </Panel>
  )
}
