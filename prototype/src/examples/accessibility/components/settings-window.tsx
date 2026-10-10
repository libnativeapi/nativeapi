import {
  Accessibility20Regular,
  Add20Regular,
  ChevronLeft16Regular,
  Desktop20Regular,
  Globe20Regular,
  LockClosed20Regular,
  Search16Regular,
  Settings20Regular,
  Subtract20Regular,
  WeatherMoon20Regular,
  Wifi120Regular,
} from '@fluentui/react-icons'

import { Card, Icon, Switch, WindowFrame, type WindowFramePlatform } from '@dazzlabs/dazzui'

import type { TrustedApp } from '../types'
import './settings-window.css'

export interface SettingsWindowProps {
  platform: WindowFramePlatform
  apps: TrustedApp[]
  inactive?: boolean
  onAllowedChange: (name: string, allowed: boolean) => void
  onClose: () => void
}

const PANES = [
  { icon: Wifi120Regular, label: 'Wi-Fi' },
  { icon: Globe20Regular, label: 'Network' },
  { icon: Settings20Regular, label: 'General' },
  { icon: WeatherMoon20Regular, label: 'Appearance' },
  { icon: Accessibility20Regular, label: 'Accessibility' },
  { icon: Desktop20Regular, label: 'Displays' },
  { icon: LockClosed20Regular, label: 'Privacy & Security', active: true },
]

/**
 * System Settings ▸ Privacy & Security ▸ Accessibility, as macOS opens it for
 * a process that called `enable()`: the apps allowed to control the
 * computer, the example among them once it has asked, switched off until
 * the user switches it on.
 */
export function SettingsWindow({ platform, apps, inactive, onAllowedChange, onClose }: SettingsWindowProps) {
  return (
    <WindowFrame
      platform={platform}
      title="Accessibility"
      width={600}
      height={430}
      inactive={inactive}
      className="settings-window"
      onClose={onClose}
    >
      <div className="settings-window__body">
        <nav className="settings-window__nav">
          <span className="settings-window__search">
            <Icon icon={Search16Regular} size={14} />
            Search
          </span>
          {PANES.map(pane => (
            <span key={pane.label} className="settings-window__pane" data-active={pane.active ? '' : undefined}>
              <span className="settings-window__pane-icon">
                <Icon icon={pane.icon} size={14} />
              </span>
              {pane.label}
            </span>
          ))}
        </nav>
        <section className="settings-window__content">
          <header className="settings-window__header">
            <Icon icon={ChevronLeft16Regular} />
            Accessibility
          </header>
          <p className="settings-window__lead">Allow the applications below to control your computer.</p>
          <Card variant="outlined" size="small" className="settings-window__list">
            {apps.map(app => (
              <div key={app.name} className="settings-window__app" data-self={app.self ? '' : undefined}>
                <span className="settings-window__app-icon" data-self={app.self ? '' : undefined}>
                  {app.name[0]}
                </span>
                <span className="settings-window__app-name">{app.name}</span>
                <Switch
                  size="small"
                  aria-label={`Allow ${app.name}`}
                  checked={app.allowed}
                  onCheckedChange={allowed => onAllowedChange(app.name, allowed)}
                />
              </div>
            ))}
            <div className="settings-window__tools">
              <Icon icon={Add20Regular} size={14} />
              <Icon icon={Subtract20Regular} size={14} />
            </div>
          </Card>
        </section>
      </div>
    </WindowFrame>
  )
}
