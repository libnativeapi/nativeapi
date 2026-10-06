import { AppGeneric20Regular, SignOut20Regular } from '@fluentui/react-icons'
import { useState } from 'react'

import {
  Avatar,
  Badge,
  Button,
  Card,
  Icon,
  SegmentedControl,
  SidebarGroup,
  SidebarGroupContent,
  SidebarGroupLabel,
  SidebarMenu,
  SidebarMenuButton,
  SidebarMenuItem,
  Spinner,
  type WindowFramePlatform,
} from '@dazzlabs/dazzui'

import { DesktopStage, DesktopWindow } from '../../components/desktop-stage'
import { EventBar } from '../../components/event-bar'
import { ExampleWindow, SidebarRowLabel } from '../../components/example-window'
import { osOf } from '../../components/platform'
import { LoginItemPanel, ProgramPanel } from './components/panels'
import { SystemWindow } from './components/system-window'
import type { Tab, WindowsView } from './types'
import { useLaunchAtLogin } from './use-launch-at-login'
import './launch-at-login-view.css'

export interface LaunchAtLoginViewProps {
  platform: WindowFramePlatform
  /** What `LaunchAtLogin.isSupported()` returns. */
  supported?: boolean
  /** Registered already as the example starts. */
  initialEnabled?: boolean
  /** The arguments recorded with it, where the platform keeps them. */
  initialArguments?: string[]
  initialTab?: Tab
  /** On Windows: Task Manager's Startup apps, or the Run key in Registry Editor. */
  initialWindowsView?: WindowsView
}

/**
 * The launch-at-login example: one `LaunchAtLogin` manager — the switch,
 * what its getters return, and its setters — beside the system's own view
 * of the registration. Log out and back in plays the next login: the app
 * starts by itself when it is registered, and stays away when it is not.
 */
export function LaunchAtLoginView({
  platform,
  supported = true,
  initialEnabled,
  initialArguments,
  initialTab = 'login',
  initialWindowsView = 'task-manager',
}: LaunchAtLoginViewProps) {
  const os = osOf(platform)
  const state = useLaunchAtLogin({ os, supported, initialEnabled, initialArguments })
  const { config, enabled, session, run, log, actions } = state
  const [tab, setTab] = useState<Tab>(initialTab)
  const [windowsView, setWindowsView] = useState<WindowsView>(initialWindowsView)
  const [front, setFront] = useState<'app' | 'system'>('app')

  const appWindow = (
    <ExampleWindow
      key={run}
      platform={platform}
      appName="Launch at Login"
      width={680}
      height={520}
      inactive={front !== 'app'}
      title="LaunchAtLogin"
      onClose={actions.quit}
      toolbar={
        <SegmentedControl<Tab>
          size="small"
          items={[
            { value: 'login', label: 'Login item' },
            { value: 'program', label: 'Program' },
          ]}
          value={tab}
          onValueChange={setTab}
        />
      }
      trailing={
        <Badge size="small" variant={enabled ? 'tinted' : 'outlined'} tint={!supported ? 'danger' : enabled ? 'primary' : 'neutral'}>
          {!supported ? 'Not supported' : enabled ? 'Enabled' : 'Disabled'}
        </Badge>
      }
      sidebar={
        <SidebarGroup>
          <SidebarGroupLabel>Manager</SidebarGroupLabel>
          <SidebarGroupContent>
            <SidebarMenu>
              <SidebarMenuItem>
                <SidebarMenuButton size="large" active icon={<Icon icon={AppGeneric20Regular} />}>
                  <SidebarRowLabel label="This app" detail={config.id} />
                </SidebarMenuButton>
              </SidebarMenuItem>
            </SidebarMenu>
          </SidebarGroupContent>
        </SidebarGroup>
      }
      sidebarFooter={
        <Button size="small" variant="normal" fullWidth disabled={!supported} onClick={actions.logOutAndIn}>
          <Icon icon={SignOut20Regular} />
          Log out and back in
        </Button>
      }
      footer={<EventBar lastEvent={log.lastEvent} log={log.log} onClear={log.clear} />}
    >
      <div className="example-window__scroll">
        {tab === 'login' ? <LoginItemPanel state={state} /> : <ProgramPanel state={state} />}
      </div>
    </ExampleWindow>
  )

  // Logging out takes the desktop away; signing in brings it back with whatever starts at login.
  const overlay =
    session === 'logging-out' || session === 'signing-in' ? (
      <div className="launch-at-login-view__login">
        <Card variant="raised" size="medium" className="launch-at-login-view__login-card">
          <Avatar name="Ada Lovelace" size="large" />
          <strong>Ada Lovelace</strong>
          <span className="launch-at-login-view__login-state">
            <Spinner size="small" />
            {session === 'logging-out' ? 'Logging out…' : 'Signing in…'}
          </span>
        </Card>
      </div>
    ) : undefined

  const hint =
    session === 'not-running' ? (
      <span className="launch-at-login-view__hint">
        {enabled ? 'Launch at Login Example quit.' : 'Signed in — the example did not start: it is not registered.'}
        <Button size="small" variant="filled" onClick={actions.open}>
          Open it
        </Button>
      </span>
    ) : undefined

  return (
    <DesktopStage
      platform={platform}
      appName="Launch at Login Example"
      layout="free"
      overlay={overlay}
      hint={hint}
      onPress={() => setFront('app')}
    >
      {session === 'running' && (
        <DesktopWindow x={40} y={36} z={front === 'app' ? 2 : 1} onPress={() => setFront('app')}>
          {appWindow}
        </DesktopWindow>
      )}
      {supported && session !== 'logging-out' && session !== 'signing-in' && (
        <DesktopWindow x={748} y={84} z={front === 'system' ? 2 : 1} onPress={() => setFront('system')}>
          <div className="launch-at-login-view__system">
            {os === 'windows' && (
              <SegmentedControl<WindowsView>
                size="small"
                items={[
                  { value: 'task-manager', label: 'Task Manager' },
                  { value: 'registry', label: 'Registry Editor' },
                ]}
                value={windowsView}
                onValueChange={setWindowsView}
              />
            )}
            <SystemWindow platform={platform} state={state} windowsView={windowsView} inactive={front !== 'system'} />
          </div>
        </DesktopWindow>
      )}
    </DesktopStage>
  )
}
