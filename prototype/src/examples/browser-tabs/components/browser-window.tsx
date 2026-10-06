import { Add16Regular, Dismiss12Regular, Dismiss16Regular } from '@fluentui/react-icons'
import { type CSSProperties, useLayoutEffect, useRef, useState } from 'react'

import { Icon, IconButton, Tooltip, WindowFrame, type WindowFramePlatform } from '@dazzlabs/dazzui'

import { osOf } from '../../../components/platform'
import { tabColor } from '../data'
import type { TabsSimulation } from '../simulate-tab-drag'
import { STRIP, tabExtent, tabLeft } from '../tab-layout'
import type { BrowserWindow as BrowserWindowModel } from '../types'
import { TabPage } from './tab-page'
import './browser-window.css'

export interface BrowserWindowProps {
  platform: WindowFramePlatform
  tabs: TabsSimulation
  window: BrowserWindowModel
  inactive?: boolean
}

/** An element's width, kept up to date as it resizes. */
function useWidth() {
  const ref = useRef<HTMLDivElement>(null)
  const [width, setWidth] = useState(0)
  useLayoutEffect(() => {
    const element = ref.current
    if (!element) return
    setWidth(element.clientWidth)
    const observer = new ResizeObserver(() => setWidth(element.clientWidth))
    observer.observe(element)
    return () => observer.disconnect()
  }, [])
  return [ref, width] as const
}

/**
 * A browser window with no title bar of its own: the 40px tab strip is its
 * chrome. On macOS the traffic lights stay, over the strip's leading inset;
 * elsewhere the strip carries a close button at its end. The strip is drawn
 * here, not taken from the kit, because it is what the example demonstrates.
 */
export function BrowserWindow({ platform, tabs, window, inactive }: BrowserWindowProps) {
  const mac = osOf(platform) === 'macos'
  const [strip, stripWidth] = useWidth()
  const extent = tabExtent(tabs.layout, stripWidth, window.tabs.length)
  const active = window.tabs.find(t => t.id === window.activeTabId) ?? window.tabs[0]

  return (
    <WindowFrame
      platform={platform}
      title="Browser"
      titlebar={false}
      controls={mac ? { close: true, minimize: true, maximize: true } : false}
      width={window.width}
      height={window.height}
      inactive={inactive}
      className="browser-window"
      data-browser-window={window.id}
      onClose={() => tabs.closeWindow(window.id)}
    >
      {/* The empty strip moves the window, like a title bar. */}
      <div
        ref={strip}
        className="browser-window__strip"
        data-browser-strip={window.id}
        onPointerDown={event => tabs.pressStrip(event, window.id)}
      >
        {stripWidth > 0 &&
          window.tabs.map((tab, index) => {
            const dragged = tabs.draggedLeft(window.id, tab.id)
            return (
              <div
                key={tab.id}
                className="browser-tab"
                data-active={tab.id === active?.id ? '' : undefined}
                data-lifted={tabs.isDragging(tab.id) ? '' : undefined}
                data-sliding={dragged !== null ? '' : undefined}
                style={{ left: dragged ?? tabLeft(tabs.layout, index, extent), width: extent } as CSSProperties}
                onPointerDown={event => tabs.pressTab(event, window.id, tab.id)}
              >
                <span className="browser-tab__dot" style={{ background: tabColor(tab.hue) }} />
                <span className="browser-tab__title">{tab.title}</span>
                <IconButton
                  label="Close tab"
                  size="tiny"
                  variant="plain"
                  className="browser-tab__close"
                  onPointerDown={event => event.stopPropagation()}
                  onClick={() => tabs.closeTab(window.id, tab.id)}
                >
                  <Icon icon={Dismiss12Regular} size={12} />
                </IconButton>
              </div>
            )
          })}
        {stripWidth > 0 && (
          <span
            className="browser-window__new-tab"
            style={{ left: tabLeft(tabs.layout, window.tabs.length, extent) + 4, width: STRIP.newTabButtonWidth - 8 }}
          >
            <Tooltip label="New tab">
              <IconButton label="New tab" size="small" variant="plain" onClick={() => tabs.addTab(window.id)}>
                <Icon icon={Add16Regular} />
              </IconButton>
            </Tooltip>
          </span>
        )}
        {!mac && (
          <span className="browser-window__close">
            <Tooltip label="Close window">
              <IconButton label="Close window" size="small" variant="plain" onClick={() => tabs.closeWindow(window.id)}>
                <Icon icon={Dismiss16Regular} />
              </IconButton>
            </Tooltip>
          </span>
        )}
      </div>
      <div className="browser-window__page">
        {/* Only the active page is drawn; every page's state lives on its tab. */}
        {active && <TabPage key={active.id} tab={active} onChange={() => tabs.touch()} />}
      </div>
    </WindowFrame>
  )
}
