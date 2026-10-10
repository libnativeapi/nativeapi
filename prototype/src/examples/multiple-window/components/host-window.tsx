import { ArrowMaximize16Regular, ArrowSync16Regular } from '@fluentui/react-icons'

import { Badge, Button, Icon, WindowFrame, type WindowFramePlatform } from '@dazzlabs/dazzui'

import { ReadBack } from '../../../components/read-back'
import { RESIZE_TO, WINDOWS } from '../data'
import type { HooksSimulation } from '../simulate-hooks'
import type { WindowKey } from '../types'
import './host-window.css'

export interface HostWindowProps {
  platform: WindowFramePlatform
  hooks: HooksSimulation
  windowKey: WindowKey
  inactive?: boolean
}

/**
 * What every window shows: its title and slot, where the hook put it, and the
 * frame nativeapi reads back for it — a read, refreshed by Read frame and on
 * a resize, never an echo of where the hook asked it to go.
 */
export function HostWindow({ platform, hooks, windowKey, inactive }: HostWindowProps) {
  const window = hooks.windows[windowKey]
  const { title, slot, description } = WINDOWS[windowKey]
  const frame = window.readBack
  return (
    <WindowFrame
      platform={platform}
      title={title}
      width={window.frame.width}
      height={window.frame.height}
      inactive={inactive}
      className="host-window"
      data-moving={hooks.moving === windowKey ? '' : undefined}
      onClose={() => hooks.hide(windowKey)}
    >
      <div className="host-window__head">
        <span className="host-window__title">{title}</span>
        <Badge size="small" variant="tinted" tint="primary">
          {slot}
        </Badge>
        <span className="host-window__spacer" />
        {windowKey === 'primary' && (
          <Button size="small" variant="filled" onClick={() => hooks.resizePrimary()}>
            <Icon icon={ArrowMaximize16Regular} />
            Resize to {RESIZE_TO.width} × {RESIZE_TO.height}
          </Button>
        )}
        <Button size="small" variant="normal" onClick={() => hooks.readFrame(windowKey)}>
          <Icon icon={ArrowSync16Regular} />
          Read frame
        </Button>
      </div>
      <div className="host-window__body">
        <p className="host-window__description">{description}</p>
        <ReadBack
          label="Window.bounds"
          columns={1}
          keyWidth="6rem"
          rows={
            frame
              ? [
                  ['Frame', `${frame.x}, ${frame.y}  ${frame.width} × ${frame.height}`],
                  ['Window id', String(window.id)],
                ]
              : [['Frame', 'No native window yet']]
          }
        />
      </div>
    </WindowFrame>
  )
}
