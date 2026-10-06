import { Battery820Regular, ChevronUp16Regular, Grid20Filled, Speaker220Regular, Wifi120Regular } from '@fluentui/react-icons'
import type { ReactNode } from 'react'

import { cx, Icon, type WindowFramePlatform } from '@dazzlabs/dazzui'

import { BAR_EDGE } from '../platform'
import { useNow } from '../use-now'
import './desktop-stage.css'

export interface DesktopStageProps {
  platform: WindowFramePlatform
  /** The app's name in the macOS menu bar. */
  appName: string
  /** The app's menus in the macOS menu bar; File, Edit, Window and Help by default. */
  menus?: ReactNode
  /** Status items the example puts in the tray, before the system's own. */
  tray?: ReactNode
  /**
   * How the desktop holds its windows: `center` stands one window in the
   * middle (or `placement`'s corner); `free` gives the children the whole
   * desktop, positioned by themselves.
   */
  layout?: 'center' | 'free'
  /** With `center`: `icon` stands the window by the tray, at the trailing end of the bar. */
  placement?: 'center' | 'icon'
  /** Shown when there is no window to show. */
  hint?: ReactNode
  /** Laid over the desktop, above the windows: a dock, a HUD, a drag preview. */
  overlay?: ReactNode
  /** In the Windows and KDE taskbar, in place of the example's running-app button. */
  taskbarApp?: ReactNode
  /** A press on the desktop or the bar, outside the windows. */
  onPress?: () => void
  className?: string
  children?: ReactNode
}

/**
 * The desktop an example runs on, drawn for the Style toolbar's platform: the
 * bar — the macOS menu bar, the Windows taskbar, GNOME's top bar, KDE's
 * panel, Omarchy's Waybar — with the example's tray items and the system's
 * own, and the wallpaper below where the windows stand.
 */
export function DesktopStage({
  platform,
  appName,
  menus,
  tray,
  layout = 'center',
  placement = 'center',
  hint,
  overlay,
  taskbarApp,
  onPress,
  className,
  children,
}: DesktopStageProps) {
  const edge = BAR_EDGE[platform]
  const now = useNow(1000)
  const clock = new Date(now * 1000 + performance.timeOrigin)
  const time = clock.toLocaleTimeString('en-GB', { hour: '2-digit', minute: '2-digit' })

  const items = (
    <div className="desktop-stage__tray">
      {platform === 'windows' && (
        <span className="desktop-stage__glyph" title="Show hidden icons">
          <Icon icon={ChevronUp16Regular} size={16} />
        </span>
      )}
      {tray}
    </div>
  )

  const status = (
    <div className="desktop-stage__status">
      <Icon icon={Wifi120Regular} size={16} />
      {platform === 'windows' && <Icon icon={Speaker220Regular} size={16} />}
      <Icon icon={Battery820Regular} size={16} />
    </div>
  )

  const bar = (() => {
    switch (platform) {
      case 'macos':
      case 'macos15':
        return (
          <>
            <div className="desktop-stage__menus">
              <strong>{appName}</strong>
              {menus ?? (
                <>
                  <span>File</span>
                  <span>Edit</span>
                  <span>Window</span>
                  <span>Help</span>
                </>
              )}
            </div>
            <div className="desktop-stage__end">
              {items}
              {status}
              <span>{clock.toLocaleDateString('en-US', { weekday: 'short', month: 'short', day: 'numeric' })}</span>
              <span>{time}</span>
            </div>
          </>
        )
      case 'windows':
      case 'kde':
        return (
          <>
            <div className="desktop-stage__taskbar-apps">
              <span className="desktop-stage__start">
                <Icon icon={Grid20Filled} size={20} />
              </span>
              {taskbarApp ?? <span className="desktop-stage__app desktop-stage__app--running" title={appName} />}
              {platform === 'windows' && (
                <>
                  <span className="desktop-stage__app" />
                  <span className="desktop-stage__app" />
                </>
              )}
            </div>
            <div className="desktop-stage__end">
              {items}
              {status}
              <span className="desktop-stage__clock">
                <span>{time}</span>
                <span>{clock.toLocaleDateString(platform === 'windows' ? 'en-CA' : 'en-GB')}</span>
              </span>
            </div>
          </>
        )
      case 'omarchy':
        return (
          <>
            <div className="desktop-stage__workspaces">
              <span data-active="">1</span>
              <span>2</span>
              <span>3</span>
            </div>
            <span className="desktop-stage__center">{time}</span>
            <div className="desktop-stage__end">{items}</div>
          </>
        )
      default:
        // GNOME and Ubuntu: Activities, the clock in the middle, the
        // AppIndicator icons beside the system menu.
        return (
          <>
            <div className="desktop-stage__menus">
              {platform === 'ubuntu' ? (
                <span className="desktop-stage__workspaces">
                  <span data-active="" />
                  <span />
                </span>
              ) : (
                <span>Activities</span>
              )}
            </div>
            <span className="desktop-stage__center">
              {clock.toLocaleDateString('en-US', { month: 'short', day: 'numeric' })} {time}
            </span>
            <div className="desktop-stage__end">
              {items}
              {status}
            </div>
          </>
        )
    }
  })()

  return (
    <div
      className={cx('desktop-stage', `desktop-stage--${edge}`, className)}
      data-platform={platform}
      onClick={onPress}
    >
      <div className="desktop-stage__bar" onClick={event => event.stopPropagation()}>
        {bar}
      </div>
      <div className="desktop-stage__desktop" data-layout={layout} data-placement={placement}>
        {children ? (
          layout === 'free' ? (
            children
          ) : (
            <div className="desktop-stage__window" onClick={event => event.stopPropagation()}>
              {children}
            </div>
          )
        ) : (
          hint && <p className="desktop-stage__hint">{hint}</p>
        )}
        {children && layout === 'free' && hint && <p className="desktop-stage__hint desktop-stage__hint--corner">{hint}</p>}
        {overlay && <div className="desktop-stage__overlay">{overlay}</div>}
      </div>
    </div>
  )
}
