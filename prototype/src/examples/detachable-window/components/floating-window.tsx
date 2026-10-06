import { useLayoutEffect, useRef } from 'react'

import { WindowFrame, type WindowFramePlatform } from '@dazzlabs/dazzui'

import { PANELS } from '../data'
import type { DetachSimulation } from '../simulate-detach'
import type { FloatingWindow as FloatingWindowModel, PanelId } from '../types'
import { PanelContent } from './panels'

export interface FloatingWindowProps {
  platform: WindowFramePlatform
  detach: DetachSimulation
  id: PanelId
  window: FloatingWindowModel
  inactive?: boolean
}

/**
 * A panel in a window of its own, with the platform's title bar: the content
 * area is exactly the size of the slot the panel came from, so nothing reflows
 * when it is torn off. Closing it docks the panel back.
 */
export function FloatingWindow({ platform, detach, id, window, inactive }: FloatingWindowProps) {
  const content = useRef<HTMLDivElement>(null)

  // How far the content sits inside the frame: the title bar the platform
  // draws. The window is placed by its content, as the example places it.
  useLayoutEffect(() => {
    const element = content.current
    const frame = element?.closest('.dz-window-frame')
    if (!element || !frame) return
    const inner = element.getBoundingClientRect()
    const outer = frame.getBoundingClientRect()
    detach.setInset({ x: Math.round(inner.left - outer.left), y: Math.round(inner.top - outer.top) })
  }, [detach, platform])

  return (
    <WindowFrame
      platform={platform}
      title={PANELS[id].title}
      controls={{ close: true, minimize: true, maximize: true }}
      width={window.width + detach.inset.x * 2}
      inactive={inactive}
      className="floating-window"
      data-detach-window={window.nativeId}
      style={{ opacity: window.opacity, transition: 'opacity var(--motion-duration) var(--motion-easing)' }}
      onClose={() => detach.closeFloating(id)}
    >
      <div ref={content} className="floating-window__content" style={{ width: window.width, height: window.height }}>
        <PanelContent detach={detach} id={id} />
      </div>
    </WindowFrame>
  )
}
