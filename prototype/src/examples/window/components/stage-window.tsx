import { type CSSProperties, type PointerEvent as ReactPointerEvent, type ReactNode, useRef } from 'react'

import type { WindowFramePlatform } from '@dazzlabs/dazzui'

import { DesktopWindow, usePointerDrag } from '../../../components/desktop-stage'
import { backgroundOf } from '../data'
import type { Area, Rect, SimWindow } from '../types'
import { frameOf, type WindowActions } from '../use-windows'
import './stage-window.css'

export interface StageWindowProps {
  w: SimWindow
  platform: WindowFramePlatform
  area: Area
  /** The stacking position among the windows; the stage turns it into a z-index. */
  z: number
  actions: WindowActions
  children: ReactNode
}

/** Where the window manager takes a press as a press on the title bar. */
const TITLE_REGION = '.dz-window-frame__titlebar, .dz-window-frame__chrome, .dz-sidebar__header'
/** What in a title bar is a control rather than the bar. */
const INTERACTIVE = 'button, input, textarea, select, a, [role="slider"], [role="switch"], [role="combobox"], [role="menuitem"]'

/**
 * One native window on the desktop, with what the window manager does to it
 * rather than the app: its title bar moves it and a double-click there
 * maximizes it, its corner resizes it, and the frame shows the getters'
 * state — opacity, shadow, material, background, stacking, mouse
 * pass-through. On Hyprland, which draws no title bars, Super-drag moves it
 * (Alt or ⌘ here).
 */
export function StageWindow({ w, platform, area, z, actions, children }: StageWindowProps) {
  const frame = frameOf(w, area)
  const start = useRef<Rect>(frame)
  const onTitle = useRef(false)

  // A maximized window restores under the pointer once it is actually
  // dragged, not on the press: a double-click there must not restore it twice.
  const restoreOnMove = useRef(false)
  const drag = usePointerDrag({
    onStart: () => {
      start.current = { ...w.frame }
      restoreOnMove.current = w.maximized
    },
    onMove: ({ dx, dy, x }) => {
      if (restoreOnMove.current) {
        if (Math.abs(dx) + Math.abs(dy) < 6) return
        restoreOnMove.current = false
        actions.unmaximizeUnder(w.id, x)
        start.current = { ...w.frame, x: x - area.left - w.frame.width / 2 - dx, y: -dy }
      }
      actions.moveTo(w.id, start.current.x + dx, start.current.y + dy)
    },
  })

  const resize = usePointerDrag({
    onStart: () => (start.current = { ...w.frame }),
    onMove: ({ dx, dy }) => actions.resizeFrom(w.id, start.current, 'bottomRight', dx, dy),
  })

  const pointerDown = (event: ReactPointerEvent) => {
    const target = event.target as Element
    onTitle.current = Boolean(target.closest(TITLE_REGION)) && !target.closest(INTERACTIVE)
    const superDrag = platform === 'omarchy' && (event.altKey || event.metaKey)
    if (!w.movable || w.fullScreen) return
    if (onTitle.current || superDrag) drag(event)
  }

  const background = backgroundOf(w.background)

  return (
    <DesktopWindow
      x={frame.x}
      y={frame.y}
      z={z}
      hidden={!w.visible || w.minimized}
      onPress={() => actions.press(w.id)}
      className="stage-window__placed"
      style={w.ignoreMouseEvents ? { pointerEvents: 'none' } : undefined}
    >
      <div
        className="stage-window"
        data-kind={w.kind}
        data-shadow={w.hasShadow ? undefined : 'off'}
        data-effect={w.visualEffect === 'none' ? undefined : w.visualEffect}
        data-fullscreen={w.fullScreen ? '' : undefined}
        data-maximized={w.maximized ? '' : undefined}
        data-tone={w.visualEffect === 'none' ? background.tone : undefined}
        data-title-colors={w.titleBarColors ? '' : undefined}
        data-buttons={w.kind === 'example' && platform.startsWith('macos') && !w.controlButtonsVisible ? 'hidden' : undefined}
        style={{ opacity: w.opacity, '--stage-window-background': background.token } as CSSProperties}
        onPointerDown={pointerDown}
        onDoubleClick={() => onTitle.current && actions.userToggleMaximize(w.id)}
      >
        {children}
        {w.resizable && !w.maximized && !w.fullScreen && (
          <span className="stage-window__grip" aria-label="Resize from the corner" onPointerDown={resize} />
        )}
      </div>
    </DesktopWindow>
  )
}
