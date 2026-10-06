import { useRef, useState } from 'react'

import { Button, Card, type WindowFramePlatform } from '@dazzlabs/dazzui'

import { DesktopStage, DesktopWindow, usePointerDrag } from '../../components/desktop-stage'
import { MainWindow } from './components/main-window'
import { ToolbarWindow } from './components/toolbar-window'
import type { Point } from './types'
import { type FloatingToolbarOptions, useFloatingToolbar } from './use-floating-toolbar'
import './floating-toolbar-view.css'

export interface FloatingToolbarViewProps extends FloatingToolbarOptions {
  platform: WindowFramePlatform
}

/**
 * The floating toolbar example: the main window, and a pill-shaped toolbar
 * window centred 10 px above it. Drag the main window by its title bar and an
 * attached toolbar follows, re-placed on every WindowMovedEvent; detach it
 * and it stays put; minimize the main window and the toolbar goes with it.
 * On Wayland nothing can place the toolbar, so the pill drags itself.
 */
export function FloatingToolbarView({ platform, ...options }: FloatingToolbarViewProps) {
  const canPlaceWindows = platform !== 'omarchy'
  const ft = useFloatingToolbar(canPlaceWindows, options)
  const [active, setActive] = useState(true)

  const from = useRef<{ main: Point; toolbar: Point; moves: number }>({ main: ft.mainAt, toolbar: ft.toolbarAt, moves: 0 })
  const clampY = (y: number) => Math.max(0, y)
  const dragMain = usePointerDrag({
    onStart: () => {
      from.current = { main: ft.mainAt, toolbar: ft.toolbarAt, moves: 0 }
      setActive(true)
    },
    onMove: ({ dx, dy }) => {
      from.current.moves++
      ft.moveMain({ x: from.current.main.x + dx, y: clampY(from.current.main.y + dy) })
    },
    onEnd: ({ dx, dy }) => ft.endMainDrag({ x: from.current.main.x + dx, y: clampY(from.current.main.y + dy) }, from.current.moves),
  })
  const dragToolbar = usePointerDrag({
    onStart: () => {
      from.current = { main: ft.mainAt, toolbar: ft.toolbarAt, moves: 0 }
      ft.log.call('toolbar.startDragging()')
    },
    onMove: ({ dx, dy }) => ft.moveToolbar({ x: from.current.toolbar.x + dx, y: clampY(from.current.toolbar.y + dy) }),
  })

  // A child is hidden while its parent is minimized.
  const toolbarShown = ft.toolbarVisible && !(ft.attached && ft.minimized) && !ft.closed
  const taskbar = platform === 'windows' || platform === 'kde'
  // Restoring brings the window back as the key window.
  const restore = () => {
    ft.restore()
    setActive(true)
  }

  return (
    <DesktopStage
      platform={platform}
      appName="Floating Toolbar"
      layout="free"
      onPress={() => setActive(false)}
      hint={ft.closed ? 'The example closed its toolbar, then its main window, and quit.' : undefined}
      taskbarApp={
        taskbar && !ft.closed ? (
          <button
            type="button"
            className="floating-toolbar-view__task desktop-stage__app desktop-stage__app--running"
            title={ft.minimized ? 'Restore Floating toolbar' : 'Floating toolbar'}
            onClick={() => (ft.minimized ? restore() : ft.minimize())}
          />
        ) : undefined
      }
      overlay={
        ft.minimized &&
        !ft.closed && (
          <Card variant="raised" size="small" className="floating-toolbar-view__minimized">
            <span>Floating toolbar is minimized</span>
            <Button
              size="small"
              variant="filled"
              onClick={event => {
                event.stopPropagation()
                restore()
              }}
            >
              Restore
            </Button>
          </Card>
        )
      }
    >
      <DesktopWindow x={ft.mainAt.x} y={ft.mainAt.y} z={1} hidden={ft.minimized || ft.closed} onPress={() => setActive(true)}>
        <MainWindow platform={platform} ft={ft} inactive={!active} onMovePress={dragMain} />
      </DesktopWindow>
      {/* Not in the taskbar, never activated by showing: it stays above its parent. */}
      <DesktopWindow x={ft.toolbarAt.x} y={ft.toolbarAt.y} z={2} hidden={!toolbarShown}>
        <ToolbarWindow
          color={ft.color}
          onPick={ft.pick}
          onStamp={ft.stamp}
          onDragPress={canPlaceWindows ? undefined : dragToolbar}
        />
      </DesktopWindow>
    </DesktopStage>
  )
}
