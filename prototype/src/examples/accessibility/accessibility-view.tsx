import {
  ArrowClockwise16Regular,
  Keyboard20Regular,
  KeyMultiple20Regular,
  ShieldCheckmark20Regular,
  ShieldError20Regular,
} from '@fluentui/react-icons'
import { useState } from 'react'

import {
  Badge,
  Button,
  Callout,
  Card,
  Icon,
  PreferenceRow,
  PreferenceSection,
  SegmentedControl,
  SidebarGroup,
  SidebarGroupContent,
  SidebarGroupLabel,
  SidebarMenu,
  SidebarMenuButton,
  SidebarMenuItem,
  type WindowFramePlatform,
} from '@dazzlabs/dazzui'

import { DesktopStage, DesktopWindow } from '../../components/desktop-stage'
import { EventBar } from '../../components/event-bar'
import { ExampleWindow, SidebarRowLabel } from '../../components/example-window'
import { Panel, PanelPreferences } from '../../components/panel'
import { osOf } from '../../components/platform'
import { ReadBack } from '../../components/read-back'
import { SettingsWindow } from './components/settings-window'
import { SystemPrompt } from './components/system-prompt'
import { dependentsOf, PLATFORM_BEHAVIOUR } from './data'
import type { Dependent, Tab } from './types'
import { type AccessibilityOptions, useAccessibility } from './use-accessibility'
import './accessibility-view.css'

export interface AccessibilityViewProps {
  platform: WindowFramePlatform
  options?: AccessibilityOptions
  initialTab?: Tab
}

/**
 * The accessibility example: is this process trusted to use the
 * accessibility APIs, and if not, ask. On macOS the answer lives in System
 * Settings — `enable()` raises the system's prompt and lists the example
 * there, switched off; the user switches it on, and the example sees it the
 * next time it checks (on activation, or Check again). Elsewhere there is
 * nothing to grant.
 */
export function AccessibilityView({ platform, options, initialTab = 'status' }: AccessibilityViewProps) {
  const os = osOf(platform)
  const mac = os === 'macos'
  const { state, actions, lastEvent, log } = useAccessibility(os, options)
  const [tab, setTab] = useState<Tab>(initialTab)
  const dependents = dependentsOf(os)

  const featureState = (dep: Dependent) => {
    if (!dep.needs) return { label: 'Not needed', tint: 'neutral' as const }
    if (dep.id === 'keyboard' && state.monitorStarted === false && !state.trusted) return { label: 'Blocked', tint: 'danger' as const }
    return state.trusted ? { label: 'Ready', tint: 'success' as const } : { label: 'Needs access', tint: 'warning' as const }
  }

  const statusPanel = (
    <Panel>
      <Card variant="outlined" size="small" className="accessibility-view__hero" data-trusted={state.trusted ? '' : undefined}>
        <span className="accessibility-view__shield">
          <Icon icon={state.trusted ? ShieldCheckmark20Regular : ShieldError20Regular} size={30} />
        </span>
        <div className="accessibility-view__hero-text">
          <strong>{state.trusted ? 'Trusted' : 'Not trusted'}</strong>
          <span>isEnabled → {String(state.trusted)}</span>
          <span className="accessibility-view__source">
            {mac ? 'AXIsProcessTrustedWithOptions' : os === 'windows' ? 'Always true on Windows' : 'An AT-SPI bus is running'}
          </span>
        </div>
        <div className="accessibility-view__hero-actions">
          <Button size="small" variant={state.trusted ? 'normal' : 'filled'} onClick={actions.enable}>
            Request access
          </Button>
          <Button size="small" variant="plain" onClick={actions.check}>
            <Icon icon={ArrowClockwise16Regular} />
            Check again
          </Button>
        </div>
      </Card>
      {mac ? (
        !state.trusted &&
        (state.allowed ? (
          <Callout size="small" tint="info" title="Switched on in System Settings">
            The example reads it on its next check: click its window, or Check again.
          </Callout>
        ) : (
          <p className="example-panel__note">
            Request access asks the system, which lists the example in Privacy & Security ▸ Accessibility, switched off.
          </p>
        ))
      ) : (
        <Callout size="small" tint="info" title="Nothing to grant here">
          {os === 'windows'
            ? 'isEnabled is always true on Windows, and enable() does nothing.'
            : 'Linux has no permission for it: isEnabled reports whether assistive technology is active, and enable() only sets up GTK’s bridge.'}
        </Callout>
      )}
      <PanelPreferences>
        <PreferenceSection label="What needs it">
          {dependents.map(dep => {
            const feature = featureState(dep)
            return (
              <PreferenceRow
                key={dep.id}
                icon={<Icon icon={dep.id === 'keyboard' ? Keyboard20Regular : KeyMultiple20Regular} />}
                title={dep.name}
                subtitle={dep.why}
              >
                <Badge size="small" variant="tinted" tint={feature.tint}>
                  {feature.label}
                </Badge>
                {dep.id === 'keyboard' && (
                  <Button size="small" variant="normal" onClick={actions.tryKeyboard}>
                    Try start
                  </Button>
                )}
              </PreferenceRow>
            )
          })}
        </PreferenceSection>
      </PanelPreferences>
      <ReadBack
        keyWidth="6.5rem"
        action={
          <span className="accessibility-view__calls">
            enable ×{state.enableCalls} · isEnabled ×{state.checks}
          </span>
        }
        rows={[
          ['isEnabled', String(state.trusted)],
          ['isMonitoring', state.monitorStarted === null ? '—' : String(state.monitorStarted)],
        ]}
      />
    </Panel>
  )

  const platformsPanel = (
    <Panel>
      <PanelPreferences>
        <PreferenceSection label="AccessibilityManager per platform">
          {PLATFORM_BEHAVIOUR.map(entry => (
            <PreferenceRow
              key={entry.os}
              title={
                <>
                  {entry.name}
                  {entry.os === os && (
                    <Badge size="small" variant="tinted" tint="primary" className="accessibility-view__here">
                      Here
                    </Badge>
                  )}
                </>
              }
              subtitle={
                <span className="accessibility-view__behaviour">
                  <span>
                    <b>isEnabled</b> {entry.isEnabled}
                  </span>
                  <span>
                    <b>enable</b> {entry.enable}
                  </span>
                </span>
              }
            />
          ))}
        </PreferenceSection>
      </PanelPreferences>
    </Panel>
  )

  const exampleWindow = (
    <ExampleWindow
      platform={platform}
      appName="Accessibility"
      width={660}
      height={480}
      inactive={state.front !== 'example' || state.prompt}
      title="Accessibility"
      subtitle={mac ? 'AccessibilityManager' : undefined}
      toolbar={
        <SegmentedControl<Tab>
          size="small"
          items={[
            { value: 'status', label: 'Status' },
            { value: 'platforms', label: 'Platforms' },
          ]}
          value={tab}
          onValueChange={setTab}
        />
      }
      sidebar={
        <>
          <SidebarGroup>
            <SidebarGroupLabel>This process</SidebarGroupLabel>
            <SidebarGroupContent>
              <SidebarMenu>
                <SidebarMenuItem>
                  <SidebarMenuButton
                    size="large"
                    active={tab === 'status'}
                    icon={
                      <Icon
                        icon={state.trusted ? ShieldCheckmark20Regular : ShieldError20Regular}
                        className={state.trusted ? 'accessibility-view__ok' : 'accessibility-view__warn'}
                      />
                    }
                    onClick={() => setTab('status')}
                  >
                    <SidebarRowLabel label="Accessibility" detail={state.trusted ? 'Trusted' : 'Not trusted'} />
                  </SidebarMenuButton>
                </SidebarMenuItem>
              </SidebarMenu>
            </SidebarGroupContent>
          </SidebarGroup>
          <SidebarGroup>
            <SidebarGroupLabel>Needs it</SidebarGroupLabel>
            <SidebarGroupContent>
              <SidebarMenu>
                {dependents.map(dep => (
                  <SidebarMenuItem key={dep.id}>
                    <SidebarMenuButton
                      icon={<Icon icon={dep.id === 'keyboard' ? Keyboard20Regular : KeyMultiple20Regular} />}
                      onClick={() => setTab('status')}
                    >
                      <SidebarRowLabel label={dep.name} detail={featureState(dep).label} />
                    </SidebarMenuButton>
                  </SidebarMenuItem>
                ))}
              </SidebarMenu>
            </SidebarGroupContent>
          </SidebarGroup>
        </>
      }
      footer={<EventBar lastEvent={lastEvent} log={log} onClear={actions.clearLog} />}
    >
      <div className="example-window__scroll">{tab === 'status' ? statusPanel : platformsPanel}</div>
    </ExampleWindow>
  )

  return (
    <DesktopStage
      platform={platform}
      appName={state.front === 'settings' ? 'System Settings' : 'Accessibility Example'}
      layout="free"
      menus={
        state.front === 'settings' ? (
          <>
            <span>Edit</span>
            <span>View</span>
            <span>Window</span>
            <span>Help</span>
          </>
        ) : undefined
      }
      overlay={
        state.prompt ? (
          <div className="accessibility-view__prompt">
            <SystemPrompt onOpenSettings={actions.openSettings} onDeny={actions.deny} />
          </div>
        ) : undefined
      }
    >
      <DesktopWindow x={56} y={48} z={state.front === 'example' ? 2 : 1} onPress={actions.activate}>
        {exampleWindow}
      </DesktopWindow>
      {state.settingsOpen && (
        <DesktopWindow x={600} y={120} z={state.front === 'settings' ? 2 : 1} onPress={actions.frontSettings}>
          <SettingsWindow
            platform={platform}
            apps={state.apps}
            inactive={state.front !== 'settings'}
            onAllowedChange={actions.setAllowed}
            onClose={actions.closeSettings}
          />
        </DesktopWindow>
      )}
    </DesktopStage>
  )
}
