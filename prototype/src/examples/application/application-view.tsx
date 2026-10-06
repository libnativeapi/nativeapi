import {
  ArrowEnter20Regular,
  ArrowExit20Regular,
  ArrowClockwise16Regular,
  Play20Regular,
  Power20Regular,
} from '@fluentui/react-icons'
import { type ReactNode, useState } from 'react'

import {
  Badge,
  Button,
  Icon,
  SegmentedControl,
  SidebarGroup,
  SidebarGroupContent,
  SidebarGroupLabel,
  SidebarMenu,
  SidebarMenuBadge,
  SidebarMenuButton,
  SidebarMenuItem,
  WindowFrame,
  type WindowFramePlatform,
} from '@dazzlabs/dazzui'

import { DesktopStage, DesktopWindow } from '../../components/desktop-stage'
import { EventBar } from '../../components/event-bar'
import { ExampleWindow, SidebarRowLabel } from '../../components/example-window'
import { osOf } from '../../components/platform'
import { AppearancePanel } from './components/appearance-panel'
import { AppDock, AppIcon, TaskbarButton } from './components/dock'
import { DockPanel } from './components/dock-panel'
import { LifecyclePanel } from './components/lifecycle-panel'
import { WindowsPanel } from './components/windows-panel'
import { APP_EVENTS, DOCK_SURFACE } from './data'
import type { AppEventType, Tab } from './types'
import { type ApplicationOptions, useApplication } from './use-application'
import { useForcedTheme } from './use-forced-theme'
import './application-view.css'

export interface ApplicationViewProps {
  platform: WindowFramePlatform
  options?: ApplicationOptions
  initialTab?: Tab
}

const EVENT_ICONS: Record<AppEventType, typeof Play20Regular> = {
  started: Play20Regular,
  activated: ArrowEnter20Regular,
  deactivated: ArrowExit20Regular,
  quitRequested: Power20Regular,
  exiting: Power20Regular,
}

const clockOf = (at: number) =>
  new Date(performance.timeOrigin + at).toLocaleTimeString('en-GB', { hour: '2-digit', minute: '2-digit', second: '2-digit' })

/**
 * The application example: `Application`'s run on the desktop it runs on.
 * The sidebar counts the lifecycle events as they arrive; the tabs drive the
 * run, the dock or taskbar icon, the forced appearance and the windows and
 * menu bar — and the desktop shows each change: the badge and progress on
 * the tile, the window flipping dark, the menu bar, the window gone on quit.
 */
export function ApplicationView({ platform, options, initialTab = 'lifecycle' }: ApplicationViewProps) {
  const os = osOf(platform)
  const { state, actions, lastEvent, log } = useApplication(os, options)
  const { themeOf, appearanceOf } = useForcedTheme()
  const [tab, setTab] = useState<Tab>(initialTab)
  const surface = DOCK_SURFACE[platform]

  const shown = state.running && !state.hidden
  const primaryMenu = os === 'windows' && state.menuBar && state.primaryId !== null

  const menuStrip = (
    <div className="application-view__menu-strip">
      <button type="button" onClick={() => actions.menuItem('About')}>
        About
      </button>
      <button type="button" onClick={() => actions.menuItem('Quit')}>
        Quit
      </button>
    </div>
  )

  const mainWindow = (
    <ExampleWindow
      platform={platform}
      appName="Application"
      width={820}
      height={520}
      inactive={!state.active || state.front !== null}
      onClose={() => actions.closeWindow(1)}
      title="Application"
      toolbar={
        <SegmentedControl<Tab>
          size="small"
          items={[
            { value: 'lifecycle', label: 'Lifecycle' },
            { value: 'dock', label: os === 'macos' ? 'Dock' : 'Taskbar' },
            { value: 'appearance', label: 'Appearance' },
            { value: 'windows', label: 'Windows' },
          ]}
          value={tab}
          onValueChange={setTab}
        />
      }
      trailing={
        <Badge size="small" variant="tinted" tint={state.quitting ? 'warning' : state.active ? 'success' : 'neutral'}>
          {state.quitting ? 'Quitting' : state.active ? 'Active' : 'Inactive'}
        </Badge>
      }
      sidebar={
        <SidebarGroup>
          <SidebarGroupLabel>Lifecycle events</SidebarGroupLabel>
          <SidebarGroupContent>
            <SidebarMenu>
              {APP_EVENTS.map(entry => {
                const tally = state.tally[entry.type]
                return (
                  <SidebarMenuItem key={entry.type}>
                    <SidebarMenuButton
                      size="large"
                      icon={
                        <Icon
                          key={tally.count}
                          icon={EVENT_ICONS[entry.type]}
                          className="application-view__event-icon"
                          data-fired={tally.count > 0 ? '' : undefined}
                        />
                      }
                      onClick={() => setTab('lifecycle')}
                    >
                      <SidebarRowLabel
                        label={entry.label}
                        detail={tally.count > 0 ? clockOf(tally.at) : 'Not yet'}
                      />
                    </SidebarMenuButton>
                    <SidebarMenuBadge>{tally.count}</SidebarMenuBadge>
                  </SidebarMenuItem>
                )
              })}
            </SidebarMenu>
          </SidebarGroupContent>
        </SidebarGroup>
      }
      footer={<EventBar lastEvent={lastEvent} log={log} onClear={actions.clearLog} />}
    >
      {primaryMenu && state.primaryId === 1 && menuStrip}
      <div className="example-window__scroll">
        {tab === 'lifecycle' && <LifecyclePanel os={os} state={state} actions={actions} />}
        {tab === 'dock' && <DockPanel platform={platform} state={state} actions={actions} />}
        {tab === 'appearance' && <AppearancePanel os={os} state={state} actions={actions} appearanceOf={appearanceOf} />}
        {tab === 'windows' && <WindowsPanel os={os} state={state} actions={actions} />}
      </div>
    </ExampleWindow>
  )

  // Hold a window in the theme its brightness forces; `System` follows the toolbar.
  const themed = (brightness: (typeof state.windows)[number]['brightness'], node: ReactNode) => (
    <div className="application-view__themed" data-theme={themeOf(brightness)}>
      {node}
    </div>
  )

  const mainBrightness = state.windows.find(w => w.id === 1)?.brightness ?? 'system'
  const left = surface === 'side-dock' ? 96 : 40

  const tile = {
    preset: state.icon,
    badge: state.badge,
    progress: state.progress,
    running: state.running,
  }

  const dock =
    surface === 'dock' || surface === 'side-dock' ? (
      <AppDock
        variant={surface === 'dock' ? 'dock' : 'side'}
        present={state.running && (surface !== 'dock' || state.dockIconVisible)}
        hidden={state.hidden}
        {...tile}
        onPress={() => (state.hidden ? actions.show() : actions.activate(null))}
      />
    ) : surface === 'none' && state.running ? (
      <div className="application-view__no-launcher">
        <AppIcon preset={state.icon} size={20} />
        Hyprland has no launcher: LauncherEntry badges and progress go unseen
      </div>
    ) : null

  // Where the window stood: what run() returned, or why the window is gone.
  const notice =
    state.exitCode !== null && !state.running ? (
      <div className="application-view__notice">
        <strong>Exited with {state.exitCode}</strong>
        <span>run() returned {state.exitCode} after ApplicationExitingEvent</span>
        <Button size="small" variant="filled" onClick={actions.relaunch}>
          <Icon icon={ArrowClockwise16Regular} />
          Relaunch
        </Button>
      </div>
    ) : state.hidden ? (
      <div className="application-view__notice">
        <strong>Hidden</strong>
        <span>Click its Dock tile, or Show, to bring the windows back</span>
      </div>
    ) : null

  return (
    <DesktopStage
      platform={platform}
      appName={state.active ? 'Application Example' : 'Finder'}
      layout="free"
      menus={
        state.active ? (
          state.menuBar ? (
            <>
              <button type="button" className="application-view__bar-menu" onClick={() => actions.menuItem('About')}>
                About
              </button>
              <button type="button" className="application-view__bar-menu" onClick={() => actions.menuItem('Quit')}>
                Quit
              </button>
            </>
          ) : (
            <></>
          )
        ) : (
          <>
            <span>File</span>
            <span>Edit</span>
            <span>View</span>
            <span>Go</span>
            <span>Window</span>
            <span>Help</span>
          </>
        )
      }
      taskbarApp={
        surface === 'taskbar' ? (
          state.running ? (
            <TaskbarButton
              variant={platform === 'kde' ? 'kde' : 'windows'}
              active={state.active}
              {...tile}
              onPress={() => (state.active ? actions.deactivate() : actions.activate(null))}
            />
          ) : (
            <></>
          )
        ) : undefined
      }
      overlay={
        <>
          {dock}
          {notice}
        </>
      }
      onPress={actions.deactivate}
    >
      {shown && (
        <DesktopWindow x={left} y={28} z={state.front === null ? 3 : 2} onPress={() => actions.activate(null)}>
          {themed(mainBrightness, mainWindow)}
        </DesktopWindow>
      )}
      {shown &&
        state.windows
          .filter(w => w.id !== 1)
          .map((w, i) => (
            <DesktopWindow
              key={w.id}
              x={left + 860 + (i % 3) * 24}
              y={60 + (i % 3) * 120}
              z={state.front === w.id ? 4 : 1}
              onPress={() => actions.activate(w.id)}
            >
              {themed(
                w.brightness,
                <WindowFrame
                  platform={platform}
                  title={w.title}
                  width={300}
                  height={190}
                  inactive={!state.active || state.front !== w.id}
                  onClose={() => actions.closeWindow(w.id)}
                >
                  <div className="application-view__extra">
                    {os === 'windows' && w.id === state.primaryId && state.menuBar && menuStrip}
                    <span className="application-view__extra-id">window #{w.id}</span>
                    <span>
                      {w.id === state.primaryId ? 'Primary window' : 'Secondary window'} · {appearanceOf(w.brightness)}
                      {w.brightness === 'system' && state.brightness !== 'system' ? ' (made after setBrightness)' : ''}
                    </span>
                  </div>
                </WindowFrame>,
              )}
            </DesktopWindow>
          ))}
    </DesktopStage>
  )
}
