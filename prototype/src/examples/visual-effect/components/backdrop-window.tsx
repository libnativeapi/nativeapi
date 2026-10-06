import type { PointerEvent as ReactPointerEvent } from 'react'

import { WindowFrame, type WindowFramePlatform } from '@dazzlabs/dazzui'

import './backdrop-window.css'

export interface BackdropWindowProps {
  platform: WindowFramePlatform
  id: number
  width: number
  height: number
  inactive: boolean
  onTitlePress: (event: ReactPointerEvent) => void
  onClose: () => void
}

/**
 * The plain red window the Backdrop switch puts behind the example, as its
 * parent: a test pattern, not part of the design, so any see-through is
 * obvious wherever the example stands on the desktop.
 */
export function BackdropWindow({ platform, id, width, height, inactive, onTitlePress, onClose }: BackdropWindowProps) {
  return (
    <div
      className="backdrop-window"
      onPointerDown={event => {
        const target = event.target as HTMLElement
        if (!target.closest('button') && target.closest('.dz-window-frame__titlebar')) onTitlePress(event)
      }}
    >
      <WindowFrame
        platform={platform}
        title="Backdrop"
        subtitle={`#${id}`}
        width={width}
        height={height}
        inactive={inactive}
        className="backdrop-window__frame"
        onClose={onClose}
      />
    </div>
  )
}
