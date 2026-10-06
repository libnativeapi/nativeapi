import { useLayoutEffect, useRef } from 'react'

import type { WindowFramePlatform } from '@dazzlabs/dazzui'

import { DesktopStage, DesktopWindow } from '../../components/desktop-stage'
import { osOf } from '../../components/platform'
import { HookPanel } from './components/hook-panel'
import { HostWindow } from './components/host-window'
import { CREATION_ORDER } from './data'
import type { HooksOptions } from './simulate-hooks'
import { blockOf, rectText } from './simulate-layout'
import { useHooks } from './use-hooks'
import './multiple-window-view.css'

export interface MultipleWindowViewProps extends HooksOptions {
  platform: WindowFramePlatform
}

/**
 * The multiple window example: three windows laid out by a will-show hook.
 * `WindowManager.setWillShowHook` catches each window just before it shows,
 * sizes and places it against the primary display's work area — the primary
 * window the top half of a centred block 60% of the work area, the other two
 * the bottom half — and lets the show through with `callOriginalShow`.
 */
export function MultipleWindowView({ platform, ...options }: MultipleWindowViewProps) {
  const hooks = useHooks(platform, options)
  const probe = useRef<HTMLSpanElement>(null)
  const workArea = hooks.workArea
  const front = hooks.frontKey

  // The desktop under the bar is the primary display's work area; its top is
  // where the bar ends.
  useLayoutEffect(() => {
    const desktop = probe.current?.parentElement
    if (!desktop) return
    const read = () =>
      hooks.setWorkArea({ x: 0, y: desktop.offsetTop, width: desktop.clientWidth, height: desktop.clientHeight })
    read()
    const observer = new ResizeObserver(read)
    observer.observe(desktop)
    return () => observer.disconnect()
  }, [hooks])

  const block = workArea && blockOf(workArea)
  const order = [...CREATION_ORDER].sort((a, b) => hooks.windows[a].z - hooks.windows[b].z)

  return (
    <DesktopStage
      platform={platform}
      appName="Multiple Window"
      layout="free"
      overlay={<HookPanel hooks={hooks} macos={osOf(platform) === 'macos'} />}
    >
      <span ref={probe} className="multiple-window-view__probe" />
      {workArea && (
        <span className="multiple-window-view__work-area">Work area {rectText(workArea)}</span>
      )}
      {workArea && block && (
        <div
          className="multiple-window-view__block"
          style={{ left: block.x - workArea.x, top: block.y - workArea.y, width: block.width, height: block.height }}
        >
          <span>60% of the work area</span>
        </div>
      )}
      {workArea &&
        CREATION_ORDER.map(key => {
          const window = hooks.windows[key]
          return (
            <DesktopWindow
              key={key}
              x={window.frame.x - workArea.x}
              y={window.frame.y - workArea.y}
              z={order.indexOf(key) + 1}
              hidden={!window.visible}
              className="multiple-window-view__placed"
              style={hooks.moving === key ? { transition: 'none' } : undefined}
              onPress={() => hooks.focus(key)}
            >
              <div className="multiple-window-view__grab" onPointerDown={event => hooks.pressWindow(event, key)}>
                <HostWindow platform={platform} hooks={hooks} windowKey={key} inactive={key !== front} />
              </div>
            </DesktopWindow>
          )
        })}
    </DesktopStage>
  )
}
