import { Window20Regular } from '@fluentui/react-icons'

import { Icon, type WindowControl, WindowFrame, type WindowFrameControls, type WindowFramePlatform } from '@dazzlabs/dazzui'

import { osOf } from '../../../components/platform'
import { effectName } from '../data'
import type { Rect, SimWindow } from '../types'
import { sizeText, type WindowActions } from '../use-windows'
import './plain-window.css'

export interface PlainWindowProps {
  w: SimWindow
  platform: WindowFramePlatform
  frame: Rect
  focused: boolean
  actions: WindowActions
}

const control = (enabled: boolean): WindowControl => (enabled ? 'enabled' : 'disabled')

/**
 * A window the example made with `Window.create()`: nothing in it but its
 * own name and size, so what shows is what the Window API does to it — the
 * title bar's style, the buttons the window may have, the background and
 * the material behind it.
 */
export function PlainWindow({ w, platform, frame, focused, actions }: PlainWindowProps) {
  const mac = osOf(platform) === 'macos'
  const hidden = w.titleBarStyle === 'hidden'
  const buttons: WindowFrameControls = {
    close: control(w.closable),
    minimize: control(w.minimizable),
    maximize: control(w.maximizable && w.resizable),
  }
  // Hidden takes the buttons with the bar; on macOS they can be turned back
  // on over the content. Hidden buttons on a normal bar: macOS only.
  const controls: WindowFrameControls | false = hidden
    ? mac && w.controlButtonsVisible
      ? buttons
      : false
    : mac && !w.controlButtonsVisible
      ? { close: 'hidden', minimize: 'hidden', maximize: 'hidden' }
      : buttons
  const under = mac && w.contentUnderTitleBar && !hidden

  return (
    <WindowFrame
      platform={platform}
      title={w.title}
      width={frame.width}
      height={frame.height}
      inactive={!focused}
      titlebar={hidden ? false : undefined}
      fullSizeContent={under || undefined}
      titleHidden={under || undefined}
      controls={controls}
      className="plain-window"
      onClose={() => actions.close(w.id)}
      onMinimize={() => actions.userMinimize(w.id)}
      onMaximize={() => actions.userToggleMaximize(w.id)}
    >
      <div className="plain-window__content" data-under={under || (hidden && controls) ? '' : undefined}>
        <Icon icon={Window20Regular} size={28} />
        <strong className="plain-window__title">{w.title}</strong>
        <span className="plain-window__size">{sizeText(frame)}</span>
        <span className="plain-window__note">
          {hidden
            ? 'No title bar: move it with Start dragging'
            : w.visualEffect !== 'none'
              ? `${effectName(w.visualEffect)} behind the content`
              : platform === 'omarchy'
                ? 'Super-drag to move it (Alt here)'
                : `id ${w.id} · Window.create()`}
        </span>
      </div>
    </WindowFrame>
  )
}
