import { AppGeneric24Filled, Window16Regular } from '@fluentui/react-icons'

import { cx, Icon, type WindowFramePlatform } from '@dazzlabs/dazzui'

import type { SimWindow } from '../types'
import './window-dock.css'

export interface WindowDockProps {
  platform: WindowFramePlatform
  windows: SimWindow[]
  focusedId: number | null
  /** A press on a window's entry: restore it, activate it, or minimize the active one. */
  onWindow: (id: number) => void
  /** A press on the app's own icon in the Dock. */
  onApp: () => void
}

/**
 * The Windows and KDE taskbar's buttons, one per window listed there:
 * `isVisibleInTaskbar` windows that are shown, minimized ones included.
 * Pressing one is what the shell does with it.
 */
export function TaskbarButtons({ windows, focusedId, onWindow }: Omit<WindowDockProps, 'platform' | 'onApp'>) {
  return (
    <>
      {windows
        .filter(w => w.visible && w.visibleInTaskbar)
        .map(w => (
          <button
            key={w.id}
            type="button"
            className="window-dock__task"
            data-active={focusedId === w.id ? '' : undefined}
            data-minimized={w.minimized ? '' : undefined}
            title={w.title}
            onClick={() => onWindow(w.id)}
          >
            <Icon icon={w.kind === 'example' ? AppGeneric24Filled : Window16Regular} size={w.kind === 'example' ? 22 : 18} />
            <span className="window-dock__task-number">{w.id}</span>
          </button>
        ))}
    </>
  )
}

/**
 * Where minimized windows go where there is no taskbar: the macOS Dock (the
 * app's icon, then each minimized window on the right of the divider), or a
 * shelf at the foot of a Linux desktop.
 */
export function WindowDock({ platform, windows, focusedId, onWindow, onApp }: WindowDockProps) {
  const minimized = windows.filter(w => w.visible && w.minimized)
  const mac = platform === 'macos' || platform === 'macos15'
  if (!mac && minimized.length === 0) return null
  return (
    <div className={cx('window-dock', mac && 'window-dock--mac')} onClick={event => event.stopPropagation()}>
      {mac ? (
        <button type="button" className="window-dock__app" title="Window Example" onClick={onApp}>
          <Icon icon={AppGeneric24Filled} size={28} />
          <span className="window-dock__running" />
        </button>
      ) : (
        <span className="window-dock__label">Minimized</span>
      )}
      {mac && minimized.length > 0 && <span className="window-dock__divider" />}
      {minimized.map(w => (
        <button
          key={w.id}
          type="button"
          className="window-dock__thumb"
          data-active={focusedId === w.id ? '' : undefined}
          title={`${w.title}: click to restore`}
          onClick={() => onWindow(w.id)}
        >
          <Icon icon={Window16Regular} size={16} />
          <span>{w.title}</span>
        </button>
      ))}
    </div>
  )
}
