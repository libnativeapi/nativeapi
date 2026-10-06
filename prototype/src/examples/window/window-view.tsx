import { Add20Regular, AppGeneric24Filled, Map20Regular, MoreHorizontal20Regular, Window20Regular } from '@fluentui/react-icons'
import { useLayoutEffect, useMemo, useRef } from 'react'

import {
  Button,
  Icon,
  IconButton,
  Menu,
  SegmentedControl,
  SidebarGroup,
  SidebarGroupContent,
  SidebarGroupLabel,
  SidebarMenu,
  SidebarMenuButton,
  SidebarMenuItem,
  type WindowFramePlatform,
} from '@dazzlabs/dazzui'

import { DesktopStage } from '../../components/desktop-stage'
import { EventBar } from '../../components/event-bar'
import { ExampleWindow, SidebarRowLabel } from '../../components/example-window'
import { AppearancePanel } from './components/appearance-panel'
import { BehaviourPanel } from './components/behaviour-panel'
import { GeometryPanel } from './components/geometry-panel'
import { MapPanel } from './components/map-panel'
import { PlainWindow } from './components/plain-window'
import { StageWindow } from './components/stage-window'
import { StateBadges } from './components/state-badges'
import { StatePanel } from './components/state-panel'
import { TaskbarButtons, WindowDock } from './components/window-dock'
import type { SimWindow, Tab } from './types'
import { boundsOf, frameOf, rectText, sizeText, useWindows, type WindowsOptions } from './use-windows'
import './window-view.css'

export interface WindowViewProps {
  platform: WindowFramePlatform
  /** The windows' starting state, the selection, the view and the tab. */
  options?: WindowsOptions
}

const TABS: readonly { value: Tab; label: string }[] = [
  { value: 'state', label: 'State' },
  { value: 'geometry', label: 'Geometry' },
  { value: 'appearance', label: 'Appearance' },
  { value: 'behaviour', label: 'Behaviour' },
]

/** The one state a sidebar row calls out, the most unusual first. */
function badgeOf(w: SimWindow) {
  if (w.fullScreen) return 'Full screen'
  if (w.minimized) return 'Minimized'
  if (!w.visible) return 'Hidden'
  if (w.maximized) return 'Maximized'
  if (w.alwaysOnTop) return 'On top'
  return null
}

/** The stacking level the window manager keeps a window in. */
const levelOf = (w: SimWindow) => (w.fullScreen ? 3 : w.alwaysOnTop ? 2 : w.alwaysOnBottom ? 0 : 1)

/**
 * The window example: a playground for `Window` and `WindowManager` on the
 * desktop it runs on. The sidebar lists every window the manager tracks —
 * the example's own and the two it created — and the pane acts on the
 * selected one over four tabs, or draws them all on the Map. Every call is
 * visible on the desktop: windows move, resize, maximize, minimize into the
 * Dock or the taskbar, hide, go translucent, lose their shadow, stay on top.
 * The windows answer the mouse too — drag a title bar, pull a corner,
 * double-click to maximize — and the events come back at the foot.
 */
export function WindowView({ platform, options }: WindowViewProps) {
  const { state, selected, actions } = useWindows(platform, options)
  const { windows, area, focusedId } = state
  const example = windows.find(w => w.kind === 'example')!
  const measure = useRef<HTMLDivElement>(null)

  // The desktop's size is the display's work area: maximize fills it, the
  // positions are measured from it.
  useLayoutEffect(() => {
    const el = measure.current
    const stage = el?.closest('.desktop-stage')
    if (!el || !stage) return
    const update = () => {
      const r = el.getBoundingClientRect()
      const s = stage.getBoundingClientRect()
      const barTop = r.top - s.top > 1
      actions.setArea({
        width: Math.round(r.width),
        height: Math.round(r.height),
        bar: Math.round(s.height - r.height),
        barTop,
        left: r.left,
        top: r.top,
      })
    }
    update()
    const observer = new ResizeObserver(update)
    observer.observe(el)
    return () => observer.disconnect()
  }, [actions])

  // The window manager's stacking: by level, then by the last raise.
  const zOf = useMemo(() => {
    const order = [...windows].sort((a, b) => levelOf(a) - levelOf(b) || a.z - b.z)
    return (w: SimWindow) => (w.fullScreen ? 60 : 1 + order.indexOf(w))
  }, [windows])

  const log = useMemo(() => state.log.map(entry => entry.text), [state.log])
  const { bounds, content } = boundsOf(selected, area, platform)

  const pane =
    state.view === 'map' ? (
      <div className="example-window__scroll">
        <MapPanel
          windows={windows}
          selected={selected}
          focusedId={focusedId}
          platform={platform}
          area={area}
          actions={actions}
        />
      </div>
    ) : (
      <>
        <div className="window-panel__summary">
          <StateBadges w={selected} focused={focusedId === selected.id} />
          <span className="window-panel__bounds">
            bounds {rectText(bounds)} · content {sizeText(content)}
          </span>
        </div>
        <div className="example-window__scroll">
          {(() => {
            const props = { w: selected, platform, area, focused: focusedId === selected.id, actions }
            switch (state.tab) {
              case 'state':
                return <StatePanel key={selected.id} {...props} />
              case 'geometry':
                return <GeometryPanel key={selected.id} {...props} />
              case 'appearance':
                return <AppearancePanel key={selected.id} {...props} />
              case 'behaviour':
                return <BehaviourPanel key={selected.id} {...props} />
            }
          })()}
        </div>
      </>
    )

  const frame = frameOf(example, area)
  const exampleWindow = (
    <ExampleWindow
      platform={platform}
      appName="Window"
      width={frame.width}
      height={frame.height}
      inactive={focusedId !== example.id}
      title={state.view === 'map' ? 'Map' : selected.title}
      subtitle={state.view === 'map' ? `${windows.length} windows` : undefined}
      toolbar={
        state.view === 'window' && (
          <SegmentedControl<Tab> size="small" items={TABS} value={state.tab} onValueChange={actions.setTab} />
        )
      }
      trailing={
        <Menu
          align="end"
          trigger={
            <IconButton label="All windows" size="small" variant="plain">
              <Icon icon={MoreHorizontal20Regular} />
            </IconButton>
          }
          items={[
            { label: 'Minimize all', onSelect: () => actions.forAll('minimize') },
            { label: 'Restore all', onSelect: () => actions.forAll('restore') },
            { label: 'Show all', onSelect: () => actions.forAll('show') },
            { label: 'Hide all', onSelect: () => actions.forAll('hide') },
          ]}
        />
      }
      sidebar={
        <>
          <SidebarGroup>
            <SidebarGroupLabel>Windows</SidebarGroupLabel>
            <SidebarGroupContent>
              <SidebarMenu>
                {windows.map(w => {
                  const badge = badgeOf(w)
                  return (
                    <SidebarMenuItem key={w.id}>
                      <SidebarMenuButton
                        size="large"
                        active={state.view === 'window' && w.id === selected.id}
                        icon={<Icon icon={w.kind === 'example' ? AppGeneric24Filled : Window20Regular} size={20} />}
                        onClick={() => actions.select(w.id)}
                      >
                        <SidebarRowLabel
                          label={w.title}
                          detail={`#${w.id} · ${badge ?? sizeText(frameOf(w, area))}${focusedId === w.id ? ' · focused' : ''}`}
                        />
                      </SidebarMenuButton>
                    </SidebarMenuItem>
                  )
                })}
              </SidebarMenu>
            </SidebarGroupContent>
          </SidebarGroup>
          <SidebarGroup>
            <SidebarGroupLabel>WindowManager</SidebarGroupLabel>
            <SidebarGroupContent>
              <SidebarMenu>
                <SidebarMenuItem>
                  <SidebarMenuButton
                    active={state.view === 'map'}
                    icon={<Icon icon={Map20Regular} />}
                    onClick={() => actions.setView('map')}
                  >
                    Map
                  </SidebarMenuButton>
                </SidebarMenuItem>
              </SidebarMenu>
            </SidebarGroupContent>
          </SidebarGroup>
        </>
      }
      sidebarFooter={
        <Button size="small" variant="normal" fullWidth onClick={actions.create}>
          <Icon icon={Add20Regular} />
          New window
        </Button>
      }
      footer={<EventBar lastEvent={state.lastEvent} log={log} onClear={actions.clearLog} />}
    >
      {pane}
    </ExampleWindow>
  )

  // The way back, which the real example does not have: a hidden window, or
  // one the mouse passes through, can no longer be reached from its own controls.
  const stranded = !example.visible
    ? 'The example’s window is hidden. Click the desktop to show it again.'
    : example.ignoreMouseEvents
      ? 'The example’s window lets the mouse through. Click the desktop to turn that off.'
      : undefined

  return (
    <DesktopStage
      platform={platform}
      appName="Window Example"
      layout="free"
      hint={stranded}
      onPress={() => {
        if (!example.visible) actions.show(example.id)
        else if (example.ignoreMouseEvents) actions.setFlag(example.id, 'ignoreMouseEvents', false)
        else actions.desktopPress()
      }}
      taskbarApp={
        platform === 'windows' || platform === 'kde' ? (
          <TaskbarButtons windows={windows} focusedId={focusedId} onWindow={actions.userTaskbar} />
        ) : undefined
      }
      overlay={
        platform === 'windows' || platform === 'kde' ? undefined : (
          <WindowDock
            platform={platform}
            windows={windows}
            focusedId={focusedId}
            onWindow={actions.userTaskbar}
            onApp={() =>
              // The Dock's reopen: the example shows its window again.
              example.minimized ? actions.userTaskbar(example.id) : actions.show(example.id)
            }
          />
        )
      }
    >
      <div ref={measure} className="window-view__measure" />
      {windows.map(w => (
        <StageWindow key={w.id} w={w} platform={platform} area={area} z={zOf(w)} actions={actions}>
          {w.kind === 'example' ? (
            exampleWindow
          ) : (
            <PlainWindow
              w={w}
              platform={platform}
              frame={frameOf(w, area)}
              focused={focusedId === w.id}
              actions={actions}
            />
          )}
        </StageWindow>
      ))}
    </DesktopStage>
  )
}
