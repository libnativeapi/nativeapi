import { Eye20Regular, EyeOff20Regular, Link20Regular, LinkDismiss20Regular, Warning20Regular } from '@fluentui/react-icons'
import type { PointerEvent as ReactPointerEvent } from 'react'

import { Badge, Button, Callout, Card, Icon, SectionLabel, WindowFooter, WindowFrame, type WindowFramePlatform } from '@dazzlabs/dazzui'

import { EventBar } from '../../../components/event-bar'
import { ReadBack } from '../../../components/read-back'
import { MAIN_SIZE, swatchOf } from '../data'
import type { FloatingToolbar } from '../use-floating-toolbar'
import './main-window.css'

export interface MainWindowProps {
  platform: WindowFramePlatform
  ft: FloatingToolbar
  inactive: boolean
  /** A press on the title bar (or, on Hyprland, Super + press anywhere): the window manager moves it. */
  onMovePress: (event: ReactPointerEvent) => void
}

const at = (p: { x: number; y: number }) => `${Math.round(p.x)}, ${Math.round(p.y)}`

function Status({ on, yes, no }: { on: boolean; yes: string; no: string }) {
  return (
    <Badge size="small" variant={on ? 'tinted' : 'outlined'} tint={on ? 'success' : 'neutral'}>
      {on ? yes : no}
    </Badge>
  )
}

/**
 * The main window: what the toolbar sets, read back here; the toolbar
 * window's controls — attach or detach it, show or hide it — and what its
 * getters return; and the log of window events at the foot.
 */
export function MainWindow({ platform, ft, inactive, onMovePress }: MainWindowProps) {
  const swatch = swatchOf(ft.color)
  return (
    <div
      className="main-window"
      onPointerDown={event => {
        const target = event.target as HTMLElement
        if (target.closest('button, [role="switch"]')) return
        const superDrag = platform === 'omarchy' && (event.metaKey || event.altKey)
        if (superDrag || target.closest('.dz-window-frame__titlebar')) onMovePress(event)
      }}
    >
      <WindowFrame
        platform={platform}
        title="Floating toolbar"
        width={MAIN_SIZE.width}
        height={MAIN_SIZE.height}
        inactive={inactive}
        className="main-window__frame"
        onClose={ft.close}
        onMinimize={ft.minimize}
      >
        <div className="main-window__scroll">
          <p className="main-window__intro">
            The pill above this window is a second window: transparent, frameless, and a child of this one.
            {ft.canPlaceWindows && ' Move or minimize this window and it comes along.'}
          </p>
          {!ft.canPlaceWindows && (
            <Callout size="small" tint="warning" icon={<Icon icon={Warning20Regular} />} title="Wayland">
              Applications cannot place their windows here, so the pill cannot follow this window. It stays above it and
              shares its state; drag the pill to put it where you want it. Super + drag moves this window.
            </Callout>
          )}

          <Card variant="raised" className="main-window__shared">
            <span className="main-window__swatch" style={{ background: swatch.color }} />
            <div className="main-window__shared-text">
              <SectionLabel>Set from the toolbar</SectionLabel>
              <span className="main-window__stamps">Stamps: {ft.stamps}</span>
              <span className="main-window__muted">Colour: {swatch.label}</span>
            </div>
          </Card>

          <div className="main-window__section">
            <SectionLabel>Toolbar window</SectionLabel>
            <div className="main-window__controls">
              <Button size="small" variant="normal" onClick={ft.attached ? ft.detach : ft.attach}>
                <Icon icon={ft.attached ? LinkDismiss20Regular : Link20Regular} />
                {ft.attached ? 'Detach toolbar' : 'Attach toolbar'}
              </Button>
              <Button size="small" variant="normal" onClick={() => ft.showToolbar(!ft.toolbarVisible)}>
                <Icon icon={ft.toolbarVisible ? EyeOff20Regular : Eye20Regular} />
                {ft.toolbarVisible ? 'Hide toolbar' : 'Show toolbar'}
              </Button>
              <span className="main-window__spacer" />
              <Status on={ft.attached} yes="attached" no="detached" />
              <Status on={ft.toolbarVisible} yes="shown" no="hidden" />
            </div>
          </div>

          <ReadBack
            columns={2}
            keyWidth="9.5rem"
            rows={[
              ['parentWindow', ft.attached ? 'main' : 'null'],
              ['isVisible', String(ft.toolbarVisible && !(ft.attached && ft.minimized))],
              ['toolbar.position', ft.canPlaceWindows ? at(ft.toolbarAt) : 'unknown on Wayland'],
              ['main.position', ft.canPlaceWindows ? at(ft.mainAt) : 'unknown on Wayland'],
            ]}
          />
        </div>
        <WindowFooter className="main-window__foot">
          <EventBar lastEvent={ft.log.lastEvent} log={ft.log.log} onClear={ft.log.clear} />
        </WindowFooter>
      </WindowFrame>
    </div>
  )
}
