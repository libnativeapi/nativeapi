import { ArrowSync16Regular, Info16Regular } from '@fluentui/react-icons'
import { useState } from 'react'

import {
  Badge,
  Button,
  Callout,
  FormField,
  Icon,
  IconButton,
  PreferenceGroup,
  PreferenceRow,
  PreferenceSection,
  Switch,
  TextField,
} from '@dazzlabs/dazzui'

import { Panel, PanelPreferences } from '../../../components/panel'
import { ReadBack } from '../../../components/read-back'
import { SAMPLE_ARGUMENTS } from '../data'
import type { LaunchAtLoginState } from '../use-launch-at-login'
import './panels.css'

const list = (args: readonly string[]) => (args.length ? `[${args.map(a => `"${a}"`).join(', ')}]` : '[]')

/**
 * The switch, and what the getters return: the manager's local config, and
 * `isEnabled()` — which reads the OS, so a registration switched off in
 * Task Manager or removed in System Settings reads false here too.
 */
export function LoginItemPanel({ state }: { state: LaunchAtLoginState }) {
  const { supported, enabled, config, registration, approved, os, launchedAtLogin, actions } = state
  const stale =
    registration !== null &&
    (registration.displayName !== config.displayName ||
      registration.executablePath !== config.executablePath ||
      list(registration.arguments) !== list(config.arguments))

  return (
    <Panel className="lal-panel">
      {launchedAtLogin && (
        <Callout size="small" tint="success" icon={<Icon icon={Info16Regular} />} title="Launched at login">
          The OS started this run as the session began
          {os === 'macos'
            ? ' — SMAppService opens the bundle without arguments.'
            : launchedAtLogin.length
              ? `, with ${launchedAtLogin.join(' ')}.`
              : ', with no arguments.'}
        </Callout>
      )}
      {!supported && (
        <Callout size="small" tint="warning" title="Not supported on this platform">
          isSupported() is false — as on Android, iOS and OpenHarmony. enable() and disable() return false and nothing
          is registered.
        </Callout>
      )}
      <PanelPreferences>
        <PreferenceSection>
          <PreferenceGroup title="Login item">
            <PreferenceRow
              title="Launch at login"
              subtitle={
                !supported
                  ? 'Not available'
                  : enabled
                    ? 'Starts when you sign in'
                    : registration && !approved
                      ? 'Registered, but switched off in Task Manager'
                      : 'Not registered'
              }
            >
              <Switch
                disabled={!supported}
                checked={enabled}
                onCheckedChange={on => (on ? actions.enable() : actions.disable())}
              />
            </PreferenceRow>
          </PreferenceGroup>
        </PreferenceSection>
      </PanelPreferences>
      <ReadBack
        label="Registered as"
        columns={1}
        keyWidth="10rem"
        action={
          <IconButton label="Refresh" size="small" variant="plain" disabled={!supported} onClick={actions.refresh}>
            <Icon icon={ArrowSync16Regular} />
          </IconButton>
        }
        rows={
          supported
            ? [
                ['isSupported()', 'true'],
                ['getId()', `"${config.id}"`],
                ['getDisplayName()', `"${config.displayName}"`],
                ['getExecutablePath()', `"${config.executablePath}"`],
                ['getArguments()', list(config.arguments)],
                ['isEnabled()', String(enabled)],
              ]
            : [['isSupported()', 'false']]
        }
      />
      {stale && (
        <p className="example-panel__note">
          The getters return the local config; the OS still holds what the last enable() wrote. Turn the switch off and
          on to register the change.
        </p>
      )}
    </Panel>
  )
}

/**
 * The setters: the display name, and the program with its arguments. Both
 * change the local config only; on macOS the program is not the example's
 * to choose — SMAppService registers the app bundle itself.
 */
export function ProgramPanel({ state }: { state: LaunchAtLoginState }) {
  const { supported, config, os, actions } = state
  const [name, setName] = useState(config.displayName)
  const [path, setPath] = useState(config.executablePath)
  const [args, setArgs] = useState(config.arguments.join(' ') || (os === 'macos' ? '' : SAMPLE_ARGUMENTS))
  const mac = os === 'macos'

  return (
    <Panel>
      <PanelPreferences>
        <PreferenceSection>
          <PreferenceGroup
            title="Display name"
            description={
              mac
                ? 'Kept locally; Login Items shows the bundle’s own name.'
                : os === 'windows'
                  ? 'Kept locally; Task Manager shows the executable’s description.'
                  : 'Written to the .desktop file’s Name and Comment.'
            }
          >
            <form
              className="lal-panel__inline"
              onSubmit={e => {
                e.preventDefault()
                actions.setDisplayName(name)
              }}
            >
              <TextField size="small" value={name} disabled={!supported} placeholder="My Application" onChange={e => setName(e.target.value)} />
              <Button size="small" variant="normal" type="submit" disabled={!supported}>
                Set
              </Button>
            </form>
          </PreferenceGroup>
          <PreferenceGroup
            title="Program"
            action={
              mac ? (
                <Badge size="small" variant="outlined">
                  Bundle only
                </Badge>
              ) : undefined
            }
          >
            {mac && (
              <Callout size="small" tint="info" title="SMAppService registers the app bundle">
                It starts this app (or a bundled login-item helper, by its identifier) and never delivers arguments. A
                custom path makes enable() fail, so setProgram is off here.
              </Callout>
            )}
            <FormField label="Executable path">
              <TextField size="small" mono value={path} disabled={mac || !supported} onChange={e => setPath(e.target.value)} />
            </FormField>
            <FormField label="Arguments" hint="Space-separated; order is kept">
              <TextField
                size="small"
                mono
                value={args}
                disabled={mac || !supported}
                placeholder="--minimized"
                onChange={e => setArgs(e.target.value)}
              />
            </FormField>
            <div className="example-panel__actions lal-panel__end">
              <Button
                size="small"
                variant="filled"
                disabled={mac || !supported}
                onClick={() => actions.setProgram(path, args.trim() ? args.trim().split(/\s+/) : [])}
              >
                Set program
              </Button>
            </div>
          </PreferenceGroup>
        </PreferenceSection>
      </PanelPreferences>
    </Panel>
  )
}
