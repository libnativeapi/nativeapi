import {
  CheckmarkCircle16Filled,
  Circle16Regular,
  ErrorCircle16Filled,
  Info16Regular,
  Play16Regular,
  Warning16Regular,
} from '@fluentui/react-icons'
import { useState } from 'react'

import {
  Badge,
  Button,
  Callout,
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

import { DesktopStage } from '../../components/desktop-stage'
import { EventBar } from '../../components/event-bar'
import { ExampleWindow } from '../../components/example-window'
import { osOf, PLATFORM_NAMES } from '../../components/platform'
import { BuildPanel } from './components/build-panel'
import { CheckCard } from './components/check-card'
import { CHECKS } from './data'
import type { CheckId, Tab } from './types'
import { useSmokeTest } from './use-smoke-test'
import './cocoapods-view.css'

export interface CocoapodsViewProps {
  platform: WindowFramePlatform
  /** Checks that throw, as when a module's symbols are missing from the build. */
  failing?: CheckId[]
  /** Stop the run partway: this check stays running, the rest wait. */
  freezeAfter?: number
  initialTab?: Tab
}

const SIDEBAR_STATUS = {
  idle: <Icon icon={Circle16Regular} className="cocoapods-view__idle" />,
  running: <Spinner size="small" />,
  pass: <Icon icon={CheckmarkCircle16Filled} className="cocoapods-view__pass" />,
  fail: <Icon icon={ErrorCircle16Filled} className="cocoapods-view__fail" />,
}

/**
 * The CocoaPods example: a build smoke test. Swift Package Manager is off
 * for the project, so it proves nativeapi still builds and links the
 * CocoaPods way — six checks, one per module, run as the app starts. Only
 * Apple platforms build it; on any other desktop the window says so.
 */
export function CocoapodsView({ platform, failing, freezeAfter, initialTab = 'checks' }: CocoapodsViewProps) {
  const buildable = osOf(platform) === 'macos'
  const { results, running, failed, done, log, run } = useSmokeTest({ buildable, failing, freezeAfter })
  const [tab, setTab] = useState<Tab>(initialTab)
  // A failing check starts picked, so its whole error shows.
  const [selected, setSelected] = useState<CheckId | null>(failing?.[0] ?? null)

  const appWindow = (
    <ExampleWindow
      platform={platform}
      appName="CocoaPods"
      width={640}
      height={480}
      title="Smoke test"
      toolbar={
        <SegmentedControl<Tab>
          size="small"
          tint={failed ? 'danger' : 'primary'}
          items={[
            { value: 'checks', label: 'Checks' },
            { value: 'build', label: 'Build' },
          ]}
          value={tab}
          onValueChange={setTab}
        />
      }
      trailing={
        running ? (
          <Spinner size="small" label="Running checks" />
        ) : done ? (
          <Badge size="small" variant="tinted" tint={failed ? 'danger' : 'success'}>
            {failed ? `${failed} failed` : `${CHECKS.length}/${CHECKS.length} passed`}
          </Badge>
        ) : undefined
      }
      sidebar={
        <SidebarGroup>
          <SidebarGroupLabel>Checks</SidebarGroupLabel>
          <SidebarGroupContent>
            <SidebarMenu>
              {CHECKS.map(check => (
                <SidebarMenuItem key={check.id}>
                  <SidebarMenuButton
                    active={check.id === selected}
                    icon={SIDEBAR_STATUS[results[check.id].status]}
                    onClick={() => {
                      setSelected(s => (s === check.id ? null : check.id))
                      setTab('checks')
                    }}
                  >
                    {check.name.split(' ')[0]}
                  </SidebarMenuButton>
                </SidebarMenuItem>
              ))}
            </SidebarMenu>
          </SidebarGroupContent>
        </SidebarGroup>
      }
      sidebarFooter={
        <Button size="small" variant="filled" fullWidth disabled={!buildable || running} onClick={run}>
          {running ? <Spinner size="small" onAccent /> : <Icon icon={Play16Regular} />}
          {running ? 'Running…' : 'Run checks'}
        </Button>
      }
      footer={<EventBar lastEvent={log.lastEvent} log={log.log} onClear={log.clear} />}
    >
      <div className="example-window__scroll">
        {tab === 'checks' ? (
          <div className="cocoapods-view__checks">
            <h2 className="cocoapods-view__heading">CocoaPods nativeapi smoke test</h2>
            {buildable ? (
              <Callout size="small" tint="info" icon={<Icon icon={Info16Regular} />} title="Built through CocoaPods">
                Swift Package Manager is off for this project, so this checks that nativeapi still builds the CocoaPods
                way. Each card calls one nativeapi module.
              </Callout>
            ) : (
              <Callout
                size="small"
                tint="warning"
                icon={<Icon icon={Warning16Regular} />}
                title={`Not built for ${PLATFORM_NAMES[platform]}`}
              >
                CocoaPods is Apple tooling: this example builds only for macOS and iOS. On this desktop nothing runs; see
                Build for the setup.
              </Callout>
            )}
            <div className="cocoapods-view__grid">
              {CHECKS.map(check => (
                <CheckCard
                  key={check.id}
                  name={check.name}
                  icon={check.icon}
                  result={results[check.id]}
                  idleDetail={buildable ? 'Waiting…' : 'Not run'}
                  selected={check.id === selected}
                  onPress={() => setSelected(s => (s === check.id ? null : check.id))}
                />
              ))}
            </div>
          </div>
        ) : (
          <BuildPanel />
        )}
      </div>
    </ExampleWindow>
  )

  return (
    <DesktopStage platform={platform} appName="CocoaPods Example">
      {appWindow}
    </DesktopStage>
  )
}
