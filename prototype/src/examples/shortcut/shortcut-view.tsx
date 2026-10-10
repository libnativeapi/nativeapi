import { Add20Regular, KeyMultiple20Regular, Settings20Regular } from '@fluentui/react-icons'
import { useState } from 'react'

import {
  Badge,
  Button,
  EmptyState,
  Icon,
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
import { osOf } from '../../components/platform'
import { Chord } from './components/chord'
import { ManagerPane } from './components/manager-pane'
import { RegisterPane } from './components/register-pane'
import { ShortcutPane } from './components/shortcut-pane'
import type { ShortcutDraft, Tab } from './types'
import { type ShortcutOptions, useShortcuts } from './use-shortcuts'
import './shortcut-view.css'

export interface ShortcutViewProps {
  platform: WindowFramePlatform
  options?: ShortcutOptions
  initialTab?: Tab
  /** What the Register tab's form starts with. */
  draft?: Partial<ShortcutDraft>
}

/**
 * The shortcut example: `ShortcutManager` with the shortcuts it registered.
 * Press a registered accelerator on the page and it fires — the counter in
 * the sidebar bumps, the chord flashes, the event bar shows the
 * ShortcutActivatedEvent. Click the desktop to take the focus away: Global
 * shortcuts still fire, Application ones wait.
 */
export function ShortcutView({ platform, options, initialTab = 'shortcut', draft }: ShortcutViewProps) {
  const os = osOf(platform)
  const wayland = platform === 'omarchy'
  const { state, actions, lastEvent, log } = useShortcuts(platform, options)
  const [tab, setTab] = useState<Tab>(initialTab)
  const selected = state.shortcuts.find(s => s.id === state.selected) ?? null

  const exampleWindow = (
    <ExampleWindow
      platform={platform}
      appName="Shortcuts"
      width={800}
      height={540}
      inactive={!state.focused}
      title="Shortcuts"
      subtitle={os === 'macos' ? 'ShortcutManager' : undefined}
      toolbar={
        <SegmentedControl<Tab>
          size="small"
          items={[
            { value: 'shortcut', label: 'Shortcut', disabled: !selected },
            { value: 'register', label: 'Register' },
            { value: 'manager', label: 'Manager' },
          ]}
          value={tab === 'shortcut' && !selected ? 'register' : tab}
          onValueChange={setTab}
        />
      }
      trailing={
        !state.managerEnabled ? (
          <Badge size="small" variant="tinted" tint="warning">
            Disabled
          </Badge>
        ) : (
          <Badge size="small" variant="tinted" tint={state.focused ? 'success' : 'neutral'}>
            {state.focused ? 'Focused' : 'Background'}
          </Badge>
        )
      }
      sidebar={
        <>
          <SidebarGroup>
            <SidebarGroupLabel>Registered</SidebarGroupLabel>
            <SidebarGroupContent>
              <SidebarMenu>
                {state.shortcuts.map(entry => (
                  <SidebarMenuItem key={entry.id} className="shortcut-view__row" data-disabled={entry.enabled ? undefined : ''}>
                    <SidebarMenuButton
                      size="large"
                      active={tab === 'shortcut' && entry.id === state.selected}
                      icon={
                        <Icon
                          key={state.flash[entry.id] ?? 0}
                          icon={KeyMultiple20Regular}
                          className="shortcut-view__icon"
                          data-flash={state.flash[entry.id] ? '' : undefined}
                        />
                      }
                      onClick={() => {
                        actions.select(entry.id)
                        setTab('shortcut')
                      }}
                    >
                      <SidebarRowLabel
                        label={<span className="shortcut-view__accelerator">{entry.accelerator}</span>}
                        detail={`${entry.scope === 'global' ? 'Global' : 'App'} · fired ${entry.activations}×`}
                      />
                    </SidebarMenuButton>
                  </SidebarMenuItem>
                ))}
              </SidebarMenu>
              {state.shortcuts.length === 0 && <p className="shortcut-view__none">No shortcuts registered.</p>}
            </SidebarGroupContent>
          </SidebarGroup>
          <SidebarGroup>
            <SidebarGroupContent>
              <SidebarMenu>
                <SidebarMenuItem>
                  <SidebarMenuButton active={tab === 'manager'} icon={<Icon icon={Settings20Regular} />} onClick={() => setTab('manager')}>
                    <SidebarRowLabel label="Manager" detail={`getAll → ${state.shortcuts.length}`} />
                  </SidebarMenuButton>
                </SidebarMenuItem>
              </SidebarMenu>
            </SidebarGroupContent>
          </SidebarGroup>
        </>
      }
      sidebarFooter={
        <Button size="small" variant="normal" fullWidth onClick={() => setTab('register')}>
          <Icon icon={Add20Regular} />
          Register
        </Button>
      }
      footer={<EventBar lastEvent={lastEvent} log={log} onClear={actions.clearLog} />}
    >
      <div className="example-window__scroll">
        {tab === 'shortcut' &&
          (selected ? (
            <ShortcutPane
              key={selected.id}
              os={os}
              entry={selected}
              flash={state.flash[selected.id] ?? 0}
              managerEnabled={state.managerEnabled}
              focused={state.focused}
              wayland={wayland}
              actions={actions}
            />
          ) : (
            <EmptyState
              title="No shortcut selected."
              action={
                <Button size="small" variant="filled" onClick={() => setTab('register')}>
                  Register one
                </Button>
              }
            />
          ))}
        {tab === 'register' && (
          <RegisterPane
            os={os}
            shortcuts={state.shortcuts}
            failure={state.failure}
            initial={draft ?? options?.attempt}
            actions={{
              ...actions,
              register: d => {
                const result = actions.register(d)
                if (result.ok) setTab('shortcut')
                return result
              },
            }}
          />
        )}
        {tab === 'manager' && (
          <ManagerPane
            os={os}
            shortcuts={state.shortcuts}
            managerEnabled={state.managerEnabled}
            focused={state.focused}
            wayland={wayland}
            actions={actions}
          />
        )}
      </div>
    </ExampleWindow>
  )

  return (
    <DesktopStage
      platform={platform}
      appName={state.focused ? 'Shortcut Example' : 'Finder'}
      layout="free"
      hint={
        state.focused
          ? 'Click the desktop to take the focus away from the example'
          : wayland
            ? 'In the background on Hyprland: the X11 grab hears nothing now'
            : 'In the background: Global shortcuts still fire, Application ones wait'
      }
      onPress={() => actions.setFocused(false)}
      overlay={
        state.firing ? (
          <div key={state.firing.key} className="shortcut-view__hud">
            <Chord accelerator={state.firing.accelerator} os={os} size="large" />
            <span>{state.firing.description || 'ShortcutActivatedEvent'}</span>
          </div>
        ) : undefined
      }
    >
      <DesktopWindow x={40} y={28} onPress={() => actions.setFocused(true)}>
        {exampleWindow}
      </DesktopWindow>
    </DesktopStage>
  )
}
