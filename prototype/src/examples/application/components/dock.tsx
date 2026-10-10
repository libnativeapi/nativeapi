import { Folder20Regular, Globe20Regular, Mail20Regular, Rocket20Regular, Settings20Regular } from '@fluentui/react-icons'
import type { CSSProperties } from 'react'

import { cx, Icon, Tooltip } from '@dazzlabs/dazzui'

import type { AppIconPreset, Progress } from '../types'
import './dock.css'

/** The app's icon as `setIcon` left it, drawn at `size` px. */
export function AppIcon({ preset, size }: { preset: AppIconPreset; size: number }) {
  return (
    <span className="app-icon" data-preset={preset} style={{ '--app-icon-size': `${size}px` } as CSSProperties}>
      <Icon icon={Rocket20Regular} size={Math.round(size * 0.58)} />
      {preset === 'beta' && <span className="app-icon__ribbon">β</span>}
    </span>
  )
}

interface TileProps {
  preset: AppIconPreset
  badge: string
  progress: Progress
  running: boolean
  onPress: () => void
}

/** The progress `setProgressBar` asked for: a bar, a busy stripe, or nothing. */
function ProgressBar({ progress, full }: { progress: Progress; full?: boolean }) {
  if (progress < 0) return null
  const busy = progress > 1 && !full
  return (
    <span className="app-dock__progress" data-busy={busy ? '' : undefined}>
      <span style={{ width: busy ? undefined : `${Math.min(progress, 1) * 100}%` }} />
    </span>
  )
}

const OTHER_APPS = [
  { icon: Folder20Regular, name: 'Files' },
  { icon: Globe20Regular, name: 'Browser' },
  { icon: Mail20Regular, name: 'Mail' },
  { icon: Settings20Regular, name: 'Settings' },
]

export interface AppDockProps extends TileProps {
  /** `dock`: the macOS Dock along the bottom; `side`: Ubuntu Dock / Dash to Dock down the left. */
  variant: 'dock' | 'side'
  /** Whether the app has a tile at all: running, and its dock icon not hidden. */
  present: boolean
  hidden: boolean
}

/**
 * The dock the desktop draws the app in, with the badge and progress
 * `Application` set on the tile: the macOS Dock, or the Ubuntu Dock (Dash to
 * Dock), which reads them from LauncherEntry — and shows "busy" as full.
 */
export function AppDock({ variant, present, hidden, preset, badge, progress, running, onPress }: AppDockProps) {
  return (
    <div className={cx('app-dock', `app-dock--${variant}`)} onClick={event => event.stopPropagation()}>
      {OTHER_APPS.map(app => (
        <span key={app.name} className="app-dock__tile app-dock__tile--other" title={app.name}>
          <Icon icon={app.icon} size={variant === 'dock' ? 22 : 20} />
        </span>
      ))}
      {present && (
        <>
          <span className="app-dock__separator" />
          <Tooltip label={hidden ? 'Application Example · hidden' : 'Application Example'} side={variant === 'dock' ? 'top' : 'right'}>
            <button type="button" className="app-dock__tile app-dock__tile--app" aria-label="Application Example" onClick={onPress}>
              <AppIcon preset={preset} size={variant === 'dock' ? 44 : 38} />
              {badge && <span className="app-dock__badge">{badge}</span>}
              <ProgressBar progress={progress} full={variant === 'side'} />
              {running && <span className="app-dock__running" data-hidden={hidden ? '' : undefined} />}
            </button>
          </Tooltip>
        </>
      )}
    </div>
  )
}

export interface TaskbarButtonProps extends TileProps {
  /** `windows`: the progress fills the button and the badge is an overlay icon; `kde`: a bar along its foot. */
  variant: 'windows' | 'kde'
  active: boolean
}

/** The app's taskbar button, with the overlay badge and the progress `Application` set on it. */
export function TaskbarButton({ variant, preset, badge, progress, active, onPress }: TaskbarButtonProps) {
  const busy = progress > 1
  return (
    <Tooltip label="Application Example" side="top">
      <button
        type="button"
        className={cx('taskbar-button', `taskbar-button--${variant}`)}
        data-active={active ? '' : undefined}
        data-busy={busy ? '' : undefined}
        aria-label="Application Example"
        style={{ '--taskbar-progress': progress < 0 ? 0 : busy ? 1 : progress } as CSSProperties}
        onClick={event => {
          event.stopPropagation()
          onPress()
        }}
      >
        {progress >= 0 && <span className="taskbar-button__progress" />}
        <AppIcon preset={preset} size={24} />
        {badge && <span className="taskbar-button__badge">{badge}</span>}
        <span className="taskbar-button__running" />
      </button>
    </Tooltip>
  )
}
