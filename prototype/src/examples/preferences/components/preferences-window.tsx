import { ArrowClockwise16Regular, Delete16Regular, History20Regular, Rocket20Regular } from '@fluentui/react-icons'
import { useState } from 'react'

import {
  Badge,
  Button,
  Card,
  Checkbox,
  Icon,
  PreferenceGroup,
  PreferenceRow,
  PreferenceSection,
  SegmentedControl,
  SidebarGroup,
  SidebarGroupContent,
  SidebarGroupLabel,
  SidebarMenu,
  SidebarMenuButton,
  SidebarMenuItem,
  TextField,
  type WindowFramePlatform,
} from '@dazzlabs/dazzui'

import { EventBar, useEventLog } from '../../../components/event-bar'
import { ExampleWindow, SidebarRowLabel } from '../../../components/example-window'
import { Panel, PanelPreferences, PanelValue } from '../../../components/panel'
import { ReadBack } from '../../../components/read-back'
import { DEFAULTS, type Key, SCOPE, THEMES } from '../data'
import type { Launch, Store, Tab, Theme } from '../types'
import './preferences-window.css'

export interface PreferencesWindowProps {
  platform: WindowFramePlatform
  /** This run: the window is keyed by it, so a relaunch starts with a fresh log and form. */
  launch: Launch
  /** This session's runs, newest first. */
  launches: readonly Launch[]
  store: Store
  initialTab?: Tab
  onSet: (key: Key, value: string) => void
  onClear: () => void
  onRelaunch: () => void
  onQuit: () => void
}

/**
 * One run of the preferences app: a small settings form bound to
 * `Preferences.createWithScope("nativeapi-example")`. Every change is a
 * `set`; what the form shows is what `get` returns, so after a relaunch it
 * is what came back from the store.
 */
export function PreferencesWindow({
  platform,
  launch,
  launches,
  store,
  initialTab = 'settings',
  onSet,
  onClear,
  onRelaunch,
  onQuit,
}: PreferencesWindowProps) {
  const first = launch.number === 1
  const { lastEvent, log, event, call, clear } = useEventLog(
    launch.startup,
    launch.startup.includes('clear() → true')
      ? 'Store cleared'
      : first
        ? 'First launch: nothing stored yet'
        : `Launch #${launch.number}: ${launch.restored} values restored`,
  )
  const [tab, setTab] = useState<Tab>(initialTab)

  const get = (key: Key) => store[key] ?? DEFAULTS[key]
  const [greeting, setGreeting] = useState(get('greeting'))
  const theme = get('theme') as Theme
  const showTips = get('show-tips') === 'true'
  const size = Object.keys(store).length

  const set = (key: Key, value: string) => {
    if (store[key] === value) return
    onSet(key, value)
    event(`Saved ${key}`, `set("${key}", "${value}") → true`)
  }

  const clearStore = () => {
    onClear()
    setGreeting(DEFAULTS.greeting)
    event('Store cleared', 'clear() → true')
    call('getAll() → 0 entries')
  }

  return (
    <ExampleWindow
      platform={platform}
      appName="Preferences"
      width={640}
      height={460}
      className="preferences-window"
      title="Preferences"
      onClose={onQuit}
      trailing={
        <Button size="small" variant="plain" tint="danger" disabled={size === 0} onClick={clearStore}>
          Clear
        </Button>
      }
      toolbar={
        <SegmentedControl<Tab>
          size="small"
          items={[
            { value: 'settings', label: 'Settings' },
            { value: 'readback', label: 'Read back' },
          ]}
          value={tab}
          onValueChange={setTab}
        />
      }
      sidebar={
        <SidebarGroup>
          <SidebarGroupLabel>Launches</SidebarGroupLabel>
          <SidebarGroupContent>
            <SidebarMenu>
              {launches.map((l, i) => (
                <SidebarMenuItem key={l.number}>
                  <SidebarMenuButton
                    size="large"
                    active={i === 0}
                    icon={<Icon icon={i === 0 ? Rocket20Regular : History20Regular} />}
                  >
                    <SidebarRowLabel
                      label={`Launch #${l.number}`}
                      detail={i === 0 ? 'This run' : l.number === 1 ? 'First run · quit' : `${l.restored} restored · quit`}
                    />
                  </SidebarMenuButton>
                </SidebarMenuItem>
              ))}
            </SidebarMenu>
          </SidebarGroupContent>
        </SidebarGroup>
      }
      sidebarFooter={
        <Button size="small" variant="normal" fullWidth onClick={onRelaunch}>
          <Icon icon={ArrowClockwise16Regular} />
          Relaunch app
        </Button>
      }
      footer={<EventBar lastEvent={lastEvent} log={log} onClear={clear} />}
    >
      <div className="example-window__scroll">
        {tab === 'settings' ? (
          <Panel>
            {/* The greeting as the app would greet with it: proof the value came back. */}
            <Card variant="tinted" tint={first ? 'info' : 'success'} size="small" className="preferences-window__hello">
              <span className="preferences-window__greeting">{get('greeting')}</span>
              <span className="preferences-window__launch">
                {first
                  ? 'First launch — every get() returned its default.'
                  : launch.restored > 0
                    ? `Launch #${launch.number} — ${launch.restored} of 3 settings came back from the store.`
                    : `Launch #${launch.number} — the store was empty, so the defaults are back.`}
              </span>
            </Card>
            <PanelPreferences>
              <PreferenceSection>
                <PreferenceGroup title="Settings">
                  <PreferenceRow title="Greeting" subtitle="Saved on Enter or blur">
                    <TextField
                      size="small"
                      className="preferences-window__field"
                      value={greeting}
                      onChange={e => setGreeting(e.target.value)}
                      onBlur={() => set('greeting', greeting)}
                      onKeyDown={e => e.key === 'Enter' && set('greeting', greeting)}
                    />
                  </PreferenceRow>
                  <PreferenceRow title="Theme" subtitle="Applied now and at every start">
                    <SegmentedControl<Theme>
                      size="small"
                      items={THEMES}
                      value={theme}
                      onValueChange={value => set('theme', value)}
                    />
                  </PreferenceRow>
                  <PreferenceRow title="Show tips at start">
                    <Checkbox checked={showTips} onCheckedChange={checked => set('show-tips', String(checked))} />
                  </PreferenceRow>
                  <PreferenceRow title="Launch count" subtitle="Bumped as the app starts">
                    <PanelValue>{get('launch-count')}</PanelValue>
                  </PreferenceRow>
                </PreferenceGroup>
              </PreferenceSection>
            </PanelPreferences>
          </Panel>
        ) : (
          <Panel>
            <ReadBack
              columns={1}
              keyWidth="10rem"
              action={
                <Badge size="small" variant="outlined">
                  {size} {size === 1 ? 'entry' : 'entries'}
                </Badge>
              }
              rows={[
                ['getScope()', `"${SCOPE}"`],
                ['getSize()', String(size)],
                ['contains("greeting")', String('greeting' in store)],
                ['contains("foo")', 'false'],
              ]}
            />
            <ReadBack
              label="getAll()"
              columns={1}
              keyWidth="7rem"
              rows={size === 0 ? [['(empty)', '{}']] : Object.entries(store).map(([k, v]) => [k, `"${v}"`] as const)}
            />
            <div className="example-panel__actions">
              <Button size="small" variant="normal" tint="danger" disabled={size === 0} onClick={clearStore}>
                <Icon icon={Delete16Regular} />
                Clear
              </Button>
              <span className="example-panel__note">
                Removes every key in the scope. The form falls back to the defaults, and the next launch counts from 1.
              </span>
            </div>
          </Panel>
        )}
      </div>
    </ExampleWindow>
  )
}
