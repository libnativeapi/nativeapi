import { Circle20Filled, RibbonStar20Regular } from '@fluentui/react-icons'
import type { PointerEvent as ReactPointerEvent } from 'react'

import { Button, Divider, Icon, Toggle } from '@dazzlabs/dazzui'

import { SWATCHES, TOOLBAR_SIZE } from '../data'
import type { Swatch } from '../types'
import './toolbar-window.css'

export interface ToolbarWindowProps {
  color: Swatch
  onPick: (color: Swatch) => void
  onStamp: () => void
  /** Where the app cannot place the toolbar, a press on the pill drags it (`startDragging`). */
  onDragPress?: (event: ReactPointerEvent) => void
}

/**
 * The floating toolbar: a second window, transparent, frameless and
 * shadowless, whose only paint is the pill. The window's own rectangle is
 * outlined while the pointer is over it, to show where it is. The pill
 * carries a small shadow of its own, since a window shadow would outline the
 * whole transparent rectangle.
 */
export function ToolbarWindow({ color, onPick, onStamp, onDragPress }: ToolbarWindowProps) {
  return (
    <div className="toolbar-window" style={{ width: TOOLBAR_SIZE.width, height: TOOLBAR_SIZE.height }}>
      <div
        className="toolbar-window__pill"
        data-draggable={onDragPress ? '' : undefined}
        onPointerDown={event => {
          if (onDragPress && !(event.target as HTMLElement).closest('button')) onDragPress(event)
        }}
      >
        {SWATCHES.map(swatch => (
          <Toggle
            key={swatch.value}
            size="small"
            variant="plain"
            label={swatch.label}
            pressed={color === swatch.value}
            onPressedChange={() => onPick(swatch.value)}
          >
            <Icon icon={Circle20Filled} style={{ color: swatch.color }} />
          </Toggle>
        ))}
        <Divider orientation="vertical" className="toolbar-window__divider" />
        <Button size="small" variant="filled" onClick={onStamp}>
          <Icon icon={RibbonStar20Regular} />
          Stamp
        </Button>
      </div>
    </div>
  )
}
