import { ReOrderDotsVertical20Regular } from '@fluentui/react-icons'
import type { PointerEvent as ReactPointerEvent } from 'react'

import { Button, Icon } from '@dazzlabs/dazzui'

import './title-bar-strip.css'

export interface TitleBarStripProps {
  /** The bar is an overlay over the content: the strip runs up behind its buttons. */
  underTitleBar: boolean
  onPress: (event: ReactPointerEvent) => void
  onDoubleClick: () => void
  onQuit: () => void
}

/**
 * The example's own title bar: a 44px strip that is always there, whatever
 * the native one is doing, so that a window with no title bar and no close
 * button can still be moved (`startDragging()` on the press), maximized (a
 * double-click) and quit. Under a transparent title bar it runs up behind
 * the macOS buttons, which is why its controls start clear of them.
 */
export function TitleBarStrip({ underTitleBar, onPress, onDoubleClick, onQuit }: TitleBarStripProps) {
  return (
    <div
      className="title-bar-strip"
      data-under={underTitleBar ? '' : undefined}
      onPointerDown={event => {
        if (!(event.target as Element).closest('button')) onPress(event)
      }}
      onDoubleClick={event => {
        if (!(event.target as Element).closest('button')) onDoubleClick()
      }}
    >
      <Icon icon={ReOrderDotsVertical20Regular} className="title-bar-strip__grip" />
      <span className="title-bar-strip__label">Drag this strip to move the window</span>
      <Button size="small" variant="normal" onClick={onQuit}>
        Quit
      </Button>
    </div>
  )
}
