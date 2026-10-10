import {
  ArrowDown20Regular,
  ArrowUp20Regular,
  Keyboard20Regular,
  KeyboardShift20Regular,
  Play16Regular,
  ShieldError20Regular,
  Stop16Regular,
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
  SidebarMenuBadge,
  SidebarMenuButton,
  SidebarMenuItem,
  Switch,
  Table,
  TableCell,
  TableHead,
  TableRow,
  type WindowFramePlatform,
} from '@dazzlabs/dazzui'

import { DesktopStage, DesktopWindow } from '../../components/desktop-stage'
import { EventBar } from '../../components/event-bar'
import { ExampleWindow, SidebarRowLabel } from '../../components/example-window'
import { Panel } from '../../components/panel'
import { osOf } from '../../components/platform'
import { ReadBack } from '../../components/read-back'
import { EventStream } from './components/event-stream'
import { KeyboardVisualizer } from './components/keyboard-visualizer'
import { ModifierLights } from './components/modifier-lights'
import { NotesWindow } from './components/notes-window'
import { describeMask, formatKeycode, KEYCODES, monitorNoteOf } from './data'
import type { KeyEventType, Tab } from './types'
import { type KeyboardOptions, useKeyboardMonitor } from './use-keyboard-monitor'
import './keyboard-view.css'

export interface KeyboardViewProps {
  platform: WindowFramePlatform
  options?: KeyboardOptions
  initialTab?: Tab
}

const EVENT_ROWS: readonly { type: KeyEventType; label: string; name: string; icon: typeof Keyboard20Regular }[] = [
  { type: 'pressed', label: 'Pressed', name: 'KeyPressedEvent', icon: ArrowDown20Regular },
  { type: 'released', label: 'Released', name: 'KeyReleasedEvent', icon: ArrowUp20Regular },
  { type: 'modifiers', label: 'Modifiers', name: 'ModifierKeysChangedEvent', icon: KeyboardShift20Regular },
]

const OS_CODES = { macos: 'macOS virtual keycodes', windows: 'Windows virtual-key codes', linux: 'X11 keycodes' } as const

/**
 * The keyboard example: a global `KeyboardMonitor`. Start it and press keys —
 * the real keys on this page arrive as the platform's monitor delivers them:
 * the keyboard lights up, each event carries the raw keycode, the modifier
 * mask lights its flags. Give the Notes window the focus and keep typing:
 * the keys still arrive, which is what a global monitor is for.
 */
export function KeyboardView({ platform, options, initialTab = 'live' }: KeyboardViewProps) {
  const os = osOf(platform)
  const { state, actions, lastEvent, log } = useKeyboardMonitor(platform, options)
  const [tab, setTab] = useState<Tab>(initialTab)
  const wayland = platform === 'omarchy'
  const last = state.events.find(r => r.type !== 'modifiers')
  const status = state.monitoring ? 'Monitoring' : state.blocked ? 'Blocked' : 'Stopped'

  const livePanel = (
    <Panel>
      {state.blocked === 'permission' && (
        <Callout
          size="small"
          tint="danger"
          icon={<Icon icon={ShieldError20Regular} />}
          title="start() did nothing: isMonitoring → false"
          action={
            state.trusted ? (
              <Button size="small" variant="filled" onClick={actions.start}>
                Start again
              </Button>
            ) : (
              <Button size="small" variant="filled" onClick={actions.grant}>
                Grant
              </Button>
            )
          }
        >
          {state.trusted
            ? 'Accessibility is granted now: start the monitor again.'
            : 'The process is not trusted for accessibility, so macOS refuses the event tap. Grant switches it on in System Settings.'}
        </Callout>
      )}
      {wayland && (
        <Callout size="small" tint="warning" title="Hyprland is a Wayland compositor">
          The monitor runs on XWayland: it sees keys while an X11 window (this example) has the focus, and nothing typed into a Wayland window.
        </Callout>
      )}
      <div className="keyboard-view__strip">
        <span className="keyboard-view__prompt">
          {state.monitoring
            ? state.focus === 'example'
              ? 'Press keys: this page’s keyboard is the system’s'
              : 'The Notes window has the focus: keep typing'
            : 'Start the monitor, then press keys'}
        </span>
        <label className="keyboard-view__focus">
          Example window focused
          <Switch
            size="small"
            checked={state.focus === 'example'}
            onCheckedChange={focused => actions.setFocus(focused ? 'example' : 'notes')}
          />
        </label>
      </div>
      <KeyboardVisualizer os={os} pressed={state.pressed} idle={!state.monitoring} />
      <ModifierLights os={os} mask={state.mask} />
      <ReadBack
        keyWidth="7rem"
        rows={[
          ['isMonitoring', String(state.monitoring)],
          ['last key', last ? `${last.key} · ${last.type}` : '—'],
          ['getKeycode', last ? formatKeycode(last.keycode, os) : '—'],
          ['modifiers', describeMask(state.mask, os)],
        ]}
      />
      <EventStream os={os} events={state.events} limit={5} empty={state.monitoring ? 'Press a key.' : 'Not monitoring.'} />
    </Panel>
  )

  const streamPanel = (
    <Panel>
      <div className="keyboard-view__strip">
        <span className="keyboard-view__prompt">
          The last {state.events.length} events, newest first, as the example prints them
        </span>
        <Button size="small" variant="plain" disabled={state.events.length === 0} onClick={actions.clearEvents}>
          Clear
        </Button>
      </div>
      <EventStream os={os} events={state.events} empty={state.monitoring ? 'Press a key.' : 'Start the monitor to see events.'} />
    </Panel>
  )

  const column = os === 'macos' ? 'mac' : os === 'windows' ? 'win' : 'x11'
  const keycodesPanel = (
    <Panel>
      <p className="example-panel__note">
        getKeycode is the platform’s raw code, not a character: {OS_CODES[os]} here. The same key, three numbers.
      </p>
      <Table className="keyboard-view__table">
        <TableHead>
          <TableCell head>Key</TableCell>
          <TableCell head data-here={column === 'mac' ? '' : undefined}>
            macOS
          </TableCell>
          <TableCell head data-here={column === 'win' ? '' : undefined}>
            Windows
          </TableCell>
          <TableCell head data-here={column === 'x11' ? '' : undefined}>
            X11
          </TableCell>
        </TableHead>
        {Object.entries(KEYCODES).map(([code, entry]) => (
          <TableRow key={code} active={state.pressed.has(code)}>
            <TableCell>
              {entry.label} <span className="keyboard-view__code-name">{code}</span>
            </TableCell>
            <TableCell data-here={column === 'mac' ? '' : undefined}>{entry.mac < 0 ? '—' : entry.mac}</TableCell>
            <TableCell data-here={column === 'win' ? '' : undefined}>{formatKeycode(entry.win, 'windows')}</TableCell>
            <TableCell data-here={column === 'x11' ? '' : undefined}>{entry.x11}</TableCell>
          </TableRow>
        ))}
      </Table>
    </Panel>
  )

  const exampleWindow = (
    <ExampleWindow
      platform={platform}
      appName="Keyboard"
      width={800}
      height={560}
      inactive={state.focus !== 'example'}
      title="Keyboard"
      subtitle={os === 'macos' ? 'KeyboardMonitor' : undefined}
      toolbar={
        <SegmentedControl<Tab>
          size="small"
          items={[
            { value: 'live', label: 'Live' },
            { value: 'stream', label: 'Stream' },
            { value: 'keycodes', label: 'Keycodes' },
          ]}
          value={tab}
          onValueChange={setTab}
        />
      }
      trailing={
        state.monitoring ? (
          <Button size="small" variant="normal" onClick={actions.stop}>
            <Icon icon={Stop16Regular} />
            Stop
          </Button>
        ) : (
          <Button size="small" variant="filled" onClick={actions.start}>
            <Icon icon={Play16Regular} />
            Start
          </Button>
        )
      }
      sidebar={
        <>
          <SidebarGroup>
            <SidebarGroupLabel>Monitor</SidebarGroupLabel>
            <SidebarGroupContent>
              <SidebarMenu>
                <SidebarMenuItem>
                  <SidebarMenuButton
                    size="large"
                    active={tab === 'live'}
                    icon={<Icon icon={Keyboard20Regular} />}
                    onClick={() => setTab('live')}
                  >
                    <SidebarRowLabel label="Monitor" detail={status} />
                  </SidebarMenuButton>
                  <SidebarMenuBadge>
                    <span className="keyboard-view__state" data-state={status.toLowerCase()} />
                  </SidebarMenuBadge>
                </SidebarMenuItem>
              </SidebarMenu>
            </SidebarGroupContent>
          </SidebarGroup>
          <SidebarGroup>
            <SidebarGroupLabel>Events</SidebarGroupLabel>
            <SidebarGroupContent>
              <SidebarMenu>
                {EVENT_ROWS.map(row => (
                  <SidebarMenuItem key={row.type}>
                    <SidebarMenuButton
                      icon={<Icon icon={row.icon} />}
                      active={tab === 'stream'}
                      onClick={() => setTab('stream')}
                    >
                      <SidebarRowLabel label={row.label} detail={row.name} />
                    </SidebarMenuButton>
                    <SidebarMenuBadge>{state.counts[row.type]}</SidebarMenuBadge>
                  </SidebarMenuItem>
                ))}
              </SidebarMenu>
            </SidebarGroupContent>
          </SidebarGroup>
        </>
      }
      footer={<EventBar lastEvent={lastEvent} log={log} onClear={actions.clearEvents} />}
    >
      <div className="example-window__scroll">
        {tab === 'live' && livePanel}
        {tab === 'stream' && streamPanel}
        {tab === 'keycodes' && keycodesPanel}
      </div>
    </ExampleWindow>
  )

  return (
    <DesktopStage
      platform={platform}
      appName={state.focus === 'notes' ? 'Notes' : 'Keyboard Example'}
      layout="free"
      hint={monitorNoteOf(platform, os)}
    >
      <DesktopWindow x={40} y={28} z={state.focus === 'example' ? 2 : 1} onPress={() => actions.setFocus('example')}>
        {exampleWindow}
      </DesktopWindow>
      <DesktopWindow x={880} y={110} z={state.focus === 'notes' ? 2 : 1} onPress={() => actions.setFocus('notes')}>
        <NotesWindow platform={platform} text={state.notes} focused={state.focus === 'notes'} wayland={wayland} />
      </DesktopWindow>
      {state.monitoring && (
        <Badge size="small" variant="filled" tint="success" className="keyboard-view__live">
          KeyboardMonitor · monitoring
        </Badge>
      )}
    </DesktopStage>
  )
}
