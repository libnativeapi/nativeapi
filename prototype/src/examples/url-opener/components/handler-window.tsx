import {
  Call20Regular,
  Document20Regular,
  Folder20Regular,
  LockClosed16Regular,
  Mail20Regular,
} from '@fluentui/react-icons'

import { Button, Icon, Skeleton, TextField, WindowFrame, type WindowFramePlatform } from '@dazzlabs/dazzui'

import type { Handler } from '../types'
import './handler-window.css'

export interface HandlerWindowProps {
  platform: WindowFramePlatform
  handler: Handler
  url: string
  /** Not the key window once the example is clicked again. */
  inactive?: boolean
  onClose: () => void
}

const TITLES: Record<Handler, string> = {
  browser: 'Browser',
  mail: 'New Message',
  phone: 'Phone',
  files: 'Downloads',
}

/**
 * The app the system handed the URL to, opened over the desktop: a browser
 * on the page, a mail client composing to the address, a dialer, a file
 * manager on the folder. It belongs to the system, not to the example.
 */
export function HandlerWindow({ platform, handler, url, inactive, onClose }: HandlerWindowProps) {
  const target = url.replace(/^[a-z]+:(\/\/)?/i, '')
  return (
    <WindowFrame
      platform={platform}
      title={handler === 'browser' ? target.split('/')[0] : TITLES[handler]}
      width={handler === 'phone' ? 260 : 420}
      height={handler === 'phone' ? 300 : 320}
      inactive={inactive}
      className="handler-window"
      onClose={onClose}
    >
      {handler === 'browser' && (
        <div className="handler-window__browser">
          <div className="handler-window__address">
            <Icon icon={LockClosed16Regular} size={14} />
            <span>{url}</span>
          </div>
          <div className="handler-window__page">
            <Skeleton shape="rect" width={'60%'} height={22} />
            <Skeleton shape="rect" width={'100%'} height={12} />
            <Skeleton shape="rect" width={'92%'} height={12} />
            <Skeleton shape="rect" width={'96%'} height={12} />
            <Skeleton shape="rect" width={'100%'} height={96} />
          </div>
        </div>
      )}
      {handler === 'mail' && (
        <div className="handler-window__mail">
          <label>
            <span>To</span>
            <TextField size="small" readOnly value={target} />
          </label>
          <label>
            <span>Subject</span>
            <TextField size="small" readOnly value="" />
          </label>
          <div className="handler-window__body" />
          <div className="handler-window__mail-actions">
            <Icon icon={Mail20Regular} />
            <Button size="small" variant="filled" disabled>
              Send
            </Button>
          </div>
        </div>
      )}
      {handler === 'phone' && (
        <div className="handler-window__phone">
          <Icon icon={Call20Regular} size={32} />
          <strong>{target}</strong>
          <span>Call from this computer?</span>
          <Button size="small" variant="filled" tint="success" disabled>
            Call
          </Button>
        </div>
      )}
      {handler === 'files' && (
        <ul className="handler-window__files">
          {['Screenshots', 'nativeapi-0.4.1.zip', 'report.pdf', 'demo.mp4'].map((name, i) => (
            <li key={name}>
              <Icon icon={i === 0 ? Folder20Regular : Document20Regular} />
              {name}
            </li>
          ))}
        </ul>
      )}
    </WindowFrame>
  )
}
