import {
  ArrowRotateClockwise20Regular,
  ArrowSync16Regular,
  Cursor16Regular,
  Desktop20Regular,
  DesktopArrowDown20Regular,
  Laptop20Regular,
  PlugDisconnected20Regular,
  Window16Regular,
  ZoomIn20Regular,
} from '@fluentui/react-icons'
import { type PointerEvent as ReactPointerEvent, useEffect, useRef, useState } from 'react'

import {
  Badge,
  Card,
  Icon,
  IconButton,
  SegmentedControl,
  SidebarGroup,
  SidebarGroupContent,
  SidebarGroupLabel,
  SidebarMenu,
  SidebarMenuButton,
  SidebarMenuItem,
  type WindowFramePlatform,
} from '@dazzlabs/dazzui'

import { DesktopStage, DesktopWindow, usePointerDrag } from '../../components/desktop-stage'
import { EventBar } from '../../components/event-bar'
import { ExampleWindow, SidebarRowLabel } from '../../components/example-window'
import { ArrangementCanvas, ArrangementLegend } from './components/arrangement-canvas'
import { DisplayDetails, DisplayTable } from './components/display-details'
import { STRIPS } from './data'
import type { Point, Rect, Scene, Tab } from './types'
import { scaleText, useDisplays } from './use-displays'
import './display-view.css'

export interface DisplayViewProps {
  platform: WindowFramePlatform
  /** The displays connected when the example starts. */
  scene?: Scene
  initialTab?: Tab
}

const WIDTH = 820
const HEIGHT = 560
const TITLE = 'Display Example'

/**
 * The display example: every display `DisplayManager` reports, arranged as
 * the system arranges them, with this window and the cursor drawn over them
 * live; the selected display's getters; and all of them in a table. The
 * example listens to `DisplayManager`, so plugging a display in, unplugging
 * one, turning or rescaling it updates the window as it happens.
 *
 * The desktop the window stands on is the primary display: moving the window
 * by its band moves it on the canvas, and the pointer anywhere on the
 * desktop — or over a display on the canvas — is the cursor.
 */
export function DisplayView({ platform, scene = 'two', initialTab = 'arrangement' }: DisplayViewProps) {
  const { displays, primary, selected, log, plugged, pluggableName, unplugTarget, nextScale, actions } = useDisplays(
    platform,
    scene,
  )
  const [tab, setTab] = useState<Tab>(initialTab)
  // Wayland tells a client neither the global pointer nor where its window is.
  const wayland = platform === 'omarchy'

  const [place, setPlace] = useState({ x: 64, y: 36 })
  const probe = useRef<HTMLDivElement>(null)
  const frame = useRef<HTMLDivElement>(null)
  const pointer = useRef<Point>({ x: 0, y: 0 })
  const [cursor, setCursor] = useState<Point>({ x: 0, y: 0 })
  const [windowRect, setWindowRect] = useState<Rect | null>(null)

  /**
   * Screen pixels per CSS pixel. The stage is the primary display, drawn so
   * that it fits: one scale both ways, so the window keeps its shape, and the
   * stage covers the display's top-left as far as its proportions allow.
   */
  const scaleOf = (stage: DOMRect) => Math.min(primary.width / stage.width, primary.height / stage.height)

  /** This window's bounds, as `getBounds()` reads them. */
  const measure = (): Rect | null => {
    const stage = probe.current?.closest('.desktop-stage')
    const win = frame.current
    if (!stage || !win) return null
    const s = stage.getBoundingClientRect()
    const w = win.getBoundingClientRect()
    const k = scaleOf(s)
    return {
      x: wayland ? 0 : Math.round(primary.x + (w.left - s.left) * k),
      y: wayland ? 0 : Math.round(primary.y + (w.top - s.top) * k),
      width: Math.round(w.width * k),
      height: Math.round(w.height * k),
    }
  }

  // The pointer on the stage is the cursor on the primary display.
  useEffect(() => {
    const stage = probe.current?.closest('.desktop-stage') as HTMLElement | null
    if (!stage) return
    const move = (event: PointerEvent) => {
      if ((event.target as Element).closest('.display-canvas')) return
      const rect = stage.getBoundingClientRect()
      const k = scaleOf(rect)
      pointer.current = {
        x: Math.round(primary.x + (event.clientX - rect.left) * k),
        y: Math.round(primary.y + (event.clientY - rect.top) * k),
      }
    }
    stage.addEventListener('pointermove', move)
    return () => stage.removeEventListener('pointermove', move)
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [primary.width, primary.x, primary.y])

  // As the example does: read the cursor and this window's bounds every 100 ms.
  useEffect(() => {
    const tick = () => {
      setCursor(wayland ? { x: 0, y: 0 } : { ...pointer.current })
      setWindowRect(measure())
    }
    tick()
    const id = window.setInterval(tick, 100)
    return () => clearInterval(id)
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [primary.width, primary.x, primary.y, wayland])

  // The window manager moves the window by its band, as on the desktop.
  const start = useRef(place)
  const drag = usePointerDrag({
    onStart: () => (start.current = place),
    onMove: ({ dx, dy }) => setPlace({ x: start.current.x + dx, y: Math.max(0, start.current.y + dy) }),
    onEnd: () => {
      const r = measure()
      if (r) log.event('Window moved', `WindowMovedEvent → ${r.x}, ${r.y}${wayland ? ' · Wayland reports no position' : ''}`)
    },
  })
  const pressFrame = (event: ReactPointerEvent) => {
    const target = event.target as Element
    if (!target.closest('.dz-window-frame__titlebar, .dz-sidebar__header')) return
    if (target.closest('button, [role="radio"], [role="tab"], input')) return
    drag(event)
  }

  const count = `${displays.length} display${displays.length === 1 ? '' : 's'}`
  const turned = selected.orientation === 'portrait' || selected.orientation === 'portraitFlipped'

  const exampleWindow = (
    <ExampleWindow
      platform={platform}
      appName={TITLE}
      width={WIDTH}
      height={HEIGHT}
      title="Displays"
      toolbar={
        <SegmentedControl<Tab>
          size="small"
          items={[
            { value: 'arrangement', label: 'Arrangement' },
            { value: 'details', label: 'Details' },
            { value: 'table', label: 'Table' },
          ]}
          value={tab}
          onValueChange={setTab}
        />
      }
      trailing={
        <span className="display-view__trailing">
          <Badge size="small" variant="tinted" tint="primary">
            {count}
          </Badge>
          <IconButton label="Refresh" size="small" variant="plain" onClick={actions.refresh}>
            <Icon icon={ArrowSync16Regular} />
          </IconButton>
        </span>
      }
      sidebar={
        <>
          <SidebarGroup>
            <SidebarGroupLabel>Displays</SidebarGroupLabel>
            <SidebarGroupContent>
              <SidebarMenu>
                {displays.map(d => (
                  <SidebarMenuItem key={d.id}>
                    <SidebarMenuButton
                      size="large"
                      active={d.id === selected.id}
                      icon={<Icon icon={d.name.startsWith('Built-in') ? Laptop20Regular : Desktop20Regular} />}
                      onClick={() => actions.select(d.id)}
                    >
                      <SidebarRowLabel
                        label={d.name}
                        detail={`${d.width} × ${d.height} @${scaleText(d.scale)}x${d.primary ? ' · Primary' : ''}`}
                      />
                    </SidebarMenuButton>
                  </SidebarMenuItem>
                ))}
              </SidebarMenu>
            </SidebarGroupContent>
          </SidebarGroup>
          {/* What the system does to the displays, so the events can be seen arriving. */}
          <SidebarGroup>
            <SidebarGroupLabel>Simulate</SidebarGroupLabel>
            <SidebarGroupContent>
              <SidebarMenu>
                <SidebarMenuItem>
                  <SidebarMenuButton
                    disabled={plugged}
                    icon={<Icon icon={DesktopArrowDown20Regular} />}
                    onClick={actions.plugIn}
                  >
                    <SidebarRowLabel label="Plug in" detail={pluggableName} />
                  </SidebarMenuButton>
                </SidebarMenuItem>
                <SidebarMenuItem>
                  <SidebarMenuButton
                    disabled={!unplugTarget}
                    icon={<Icon icon={PlugDisconnected20Regular} />}
                    onClick={actions.unplug}
                  >
                    <SidebarRowLabel label="Unplug" detail={unplugTarget?.name ?? 'Only the primary is left'} />
                  </SidebarMenuButton>
                </SidebarMenuItem>
                <SidebarMenuItem>
                  <SidebarMenuButton icon={<Icon icon={ArrowRotateClockwise20Regular} />} onClick={actions.rotate}>
                    <SidebarRowLabel
                      label="Rotate"
                      detail={`to ${turned ? 'landscape' : 'portrait'} · ${selected.name}`}
                    />
                  </SidebarMenuButton>
                </SidebarMenuItem>
                <SidebarMenuItem>
                  <SidebarMenuButton icon={<Icon icon={ZoomIn20Regular} />} onClick={actions.changeScale}>
                    <SidebarRowLabel label="Change scale" detail={`to ${scaleText(nextScale)}x · ${selected.name}`} />
                  </SidebarMenuButton>
                </SidebarMenuItem>
              </SidebarMenu>
            </SidebarGroupContent>
          </SidebarGroup>
        </>
      }
      footer={<EventBar lastEvent={log.lastEvent} log={log.log} onClear={log.clear} />}
    >
      {tab === 'arrangement' && (
        <div className="display-view__arrangement">
          <Card variant="sunken" size="small" className="display-view__canvas-card">
            <ArrangementCanvas
              displays={displays}
              selectedId={selected.id}
              onSelect={actions.select}
              windowRect={windowRect}
              windowTitle={TITLE}
              cursor={wayland ? null : cursor}
              onPointer={point => (pointer.current = point)}
            />
            <ArrangementLegend strips={STRIPS[platform]} cursor={!wayland} />
          </Card>
        </div>
      )}
      {tab === 'details' && (
        <div className="example-window__scroll">
          <DisplayDetails display={selected} />
        </div>
      )}
      {tab === 'table' && (
        <div className="example-window__scroll">
          <DisplayTable displays={displays} selectedId={selected.id} onSelect={actions.select} />
        </div>
      )}
      {/* The two live read-outs the canvas draws, as numbers. */}
      <div className="display-view__status">
        <Icon icon={Cursor16Regular} />
        <span>
          Cursor ({cursor.x}, {cursor.y})
        </span>
        <Icon icon={Window16Regular} />
        <span className="display-view__status-window">
          This window{' '}
          {windowRect ? `(${windowRect.x}, ${windowRect.y}) ${windowRect.width} × ${windowRect.height}` : 'none'}
        </span>
        <span className="display-view__status-note">{wayland ? 'Wayland: no global positions' : 'every 100 ms'}</span>
      </div>
    </ExampleWindow>
  )

  return (
    <DesktopStage
      platform={platform}
      appName={TITLE}
      layout="free"
      hint="This desktop is the primary display: move the window by its band, and the pointer is the cursor."
    >
      <div ref={probe} className="display-view__probe" />
      <DesktopWindow x={place.x} y={place.y}>
        <div ref={frame} className="display-view__frame" onPointerDown={pressFrame}>
          {exampleWindow}
        </div>
      </DesktopWindow>
    </DesktopStage>
  )
}
