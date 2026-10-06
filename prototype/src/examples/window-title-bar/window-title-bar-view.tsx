import { useLayoutEffect, useRef } from 'react'

import {
  SectionLabel,
  SegmentedControl,
  WindowFooter,
  WindowFrame,
  type WindowFrameControls,
  type WindowFramePlatform,
} from '@dazzlabs/dazzui'

import { DesktopStage, DesktopWindow, usePointerDrag } from '../../components/desktop-stage'
import { EventBar } from '../../components/event-bar'
import { osOf, PLATFORM_NAMES } from '../../components/platform'
import { ReadBack } from '../../components/read-back'
import { TitleBarStrip } from './components/title-bar-strip'
import { LOOK_FOR, WINDOW_TITLE } from './data'
import type { TitleBarState } from './types'
import { stateOf, type TitleBarOptions, useTitleBar } from './use-title-bar'
import './window-title-bar-view.css'

export interface WindowTitleBarViewProps {
  platform: WindowFramePlatform
  options?: TitleBarOptions
}

const ENABLED: WindowFrameControls = { close: true, minimize: true, maximize: true }

/**
 * The title bar example: one window whose title bar is the subject, in its
 * three states — Normal, Content under the title bar (macOS only) and Hidden
 * — with its buttons shown or hidden. The window draws a strip of its own
 * that is always there to move it by, and its panel reads back what the
 * getters return after each change: the frame stays, the content takes in
 * the title bar.
 */
export function WindowTitleBarView({ platform, options }: WindowTitleBarViewProps) {
  const { w, desk, running, supported, content, log, actions } = useTitleBar(platform, options)
  const mac = osOf(platform) === 'macos'
  const measure = useRef<HTMLDivElement>(null)
  const state = stateOf(w)

  useLayoutEffect(() => {
    const el = measure.current
    if (!el) return
    const update = () => actions.setDesk({ width: Math.round(el.clientWidth), height: Math.round(el.clientHeight) })
    update()
    const observer = new ResizeObserver(update)
    observer.observe(el)
    return () => observer.disconnect()
    // The observer is set up once; setDesk only reads the newest window.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [running])

  const start = useRef({ x: 0, y: 0 })
  const restoreOnMove = useRef(false)
  const drag = usePointerDrag({
    onStart: () => {
      start.current = { x: w.x, y: w.y }
      restoreOnMove.current = w.maximized
    },
    onMove: ({ dx, dy, x }) => {
      if (restoreOnMove.current) {
        // A maximized window restores under the pointer once it is dragged.
        if (Math.abs(dx) + Math.abs(dy) < 6) return
        restoreOnMove.current = false
        const left = (measure.current?.getBoundingClientRect().left ?? 0) + w.width / 2
        start.current = { x: x - left - dx, y: -dy }
      }
      actions.moveTo(start.current.x + dx, start.current.y + dy)
    },
  })

  const hidden = w.titleBarStyle === 'hidden'
  const under = mac && w.contentUnderTitleBar && !hidden
  // Hidden takes the buttons with the bar; on macOS they can come back over
  // the content. Hiding them under a bar is macOS only too.
  const controls: WindowFrameControls | false = hidden
    ? mac && w.buttonsVisible
      ? ENABLED
      : false
    : mac && !w.buttonsVisible
      ? { close: 'hidden', minimize: 'hidden', maximize: 'hidden' }
      : ENABLED
  const overStrip = mac && (under || (hidden && w.buttonsVisible))

  const frame = w.maximized ? { x: 0, y: 0, width: desk.width, height: desk.height } : w

  return (
    <DesktopStage
      platform={platform}
      appName={WINDOW_TITLE}
      layout="free"
      hint={running ? undefined : 'The application quit. Click the desktop to run it again.'}
      onPress={running ? undefined : actions.relaunch}
    >
      <div ref={measure} className="window-title-bar-view__measure" />
      {running && w.x >= 0 && (
        <DesktopWindow x={frame.x} y={frame.y}>
          <WindowFrame
            platform={platform}
            title={WINDOW_TITLE}
            width={frame.width}
            height={frame.height}
            titlebar={hidden ? false : undefined}
            fullSizeContent={under || undefined}
            titleHidden={under || undefined}
            controls={controls}
            className="window-title-bar-view__window"
            data-maximized={w.maximized ? '' : undefined}
            onClose={actions.quit}
            onMaximize={actions.toggleMaximize}
            onPointerDown={event => {
              // The native title bar moves the window too, while there is one.
              const target = event.target as Element
              if (target.closest('.dz-window-frame__titlebar') && !target.closest('button')) drag(event)
            }}
          >
            <div className="window-title-bar-view__body">
              <TitleBarStrip
                underTitleBar={overStrip}
                onPress={event => {
                  actions.startDragging()
                  drag(event)
                }}
                onDoubleClick={actions.toggleMaximize}
                onQuit={actions.quit}
              />
              <div className="window-title-bar-view__scroll">
                <div className="window-title-bar-view__options">
                  <span className="window-title-bar-view__label">Title bar</span>
                  <SegmentedControl<TitleBarState>
                    size="small"
                    items={[
                      { value: 'normal', label: 'Normal' },
                      { value: 'under', label: 'Content under title bar', disabled: !supported },
                      { value: 'hidden', label: 'Hidden' },
                    ]}
                    value={state}
                    onValueChange={actions.setState}
                  />
                  <span className="window-title-bar-view__hint">{supported ? '' : 'Under: macOS only'}</span>
                  <span className="window-title-bar-view__label">Buttons</span>
                  <SegmentedControl<'show' | 'hide'>
                    size="small"
                    items={[
                      { value: 'show', label: 'Show buttons' },
                      { value: 'hide', label: 'Hide buttons' },
                    ]}
                    value={w.buttonsVisible ? 'show' : 'hide'}
                    onValueChange={value => actions.setButtons(value === 'show')}
                  />
                  <span className="window-title-bar-view__hint">A style resets these; set them after it</span>
                </div>
                <ReadBack
                  label="Now"
                  columns={1}
                  keyWidth="18.5rem"
                  rows={[
                    ['titleBarStyle', `TitleBarStyle.${w.titleBarStyle}`],
                    ['isContentUnderTitleBar', String(w.contentUnderTitleBar)],
                    ['isWindowControlButtonsVisible', String(w.buttonsVisible)],
                    ['isContentUnderTitleBarSupported()', String(supported)],
                    ['contentSize', `${content.width} × ${content.height}`],
                  ]}
                />
                <div className="window-title-bar-view__notes">
                  <SectionLabel>What to look for</SectionLabel>
                  <ul>
                    {LOOK_FOR.map(text => (
                      <li key={text}>{text}</li>
                    ))}
                    {platform === 'omarchy' && (
                      <li>{PLATFORM_NAMES[platform]} draws no title bars: Normal and Hidden look alike, and contentSize does not change.</li>
                    )}
                  </ul>
                </div>
              </div>
              <WindowFooter className="window-title-bar-view__footer">
                <EventBar lastEvent={log.lastEvent} log={log.log} onClear={log.clear} />
              </WindowFooter>
            </div>
          </WindowFrame>
        </DesktopWindow>
      )}
    </DesktopStage>
  )
}
