import {
  Add16Regular,
  AppGeneric20Regular,
  ChevronLeft16Regular,
  ChevronRight16Regular,
  Subtract16Regular,
} from '@fluentui/react-icons'

import {
  Button,
  Icon,
  IconButton,
  Table,
  TableCell,
  TableHead,
  TableRow,
  WindowFrame,
  type WindowFramePlatform,
} from '@dazzlabs/dazzui'

import { osOf } from '../../../components/platform'
import { commandLine, desktopFile, OTHER_LOGIN_ITEMS } from '../data'
import type { WindowsView } from '../types'
import type { LaunchAtLoginState } from '../use-launch-at-login'
import './system-window.css'

export interface SystemWindowProps {
  platform: WindowFramePlatform
  state: LaunchAtLoginState
  windowsView: WindowsView
  inactive?: boolean
}

/** What the executable's version resource calls it: Task Manager's and Login Items' name for the app. */
const APP_NAME = 'launch_at_login_example'

/**
 * The registration as the system shows it, live: System Settings ▸ General
 * ▸ Login Items on macOS; Task Manager's Startup apps or the Run key in
 * Registry Editor on Windows; the autostart .desktop file on Linux. Its own
 * controls act on the OS record, not on the app.
 */
export function SystemWindow({ platform, state, windowsView, inactive }: SystemWindowProps) {
  const os = osOf(platform)
  const { registration, approved, config, actions } = state

  if (os === 'macos') {
    const items = [
      ...OTHER_LOGIN_ITEMS.macos,
      ...(registration ? [{ name: APP_NAME, detail: 'Application', enabled: true, ours: true }] : []),
    ]
    return (
      <WindowFrame platform={platform} title="Login Items & Extensions" width={460} height={380} inactive={inactive}>
        <div className="system-window__settings">
          <div className="system-window__crumbs">
            <Icon icon={ChevronLeft16Regular} />
            <Icon icon={ChevronRight16Regular} />
            <strong>Login Items &amp; Extensions</strong>
          </div>
          <span className="system-window__section">Open at Login</span>
          <div className="system-window__list">
            {items.map(item => (
              <div key={item.name} className="system-window__item" data-ours={'ours' in item || undefined}>
                <Icon icon={AppGeneric20Regular} />
                <span className="system-window__name">{item.name}</span>
                <span className="system-window__detail">{item.detail}</span>
              </div>
            ))}
            <div className="system-window__toolbar">
              <IconButton label="Add" size="tiny" variant="plain" disabled>
                <Icon icon={Add16Regular} />
              </IconButton>
              <IconButton label={`Remove ${APP_NAME}`} size="tiny" variant="plain" disabled={!registration} onClick={actions.systemRemove}>
                <Icon icon={Subtract16Regular} />
              </IconButton>
            </div>
          </div>
          <p className="system-window__note">
            The list names the bundle, whatever setDisplayName() says. The − unregisters it, and isEnabled() reads false.
          </p>
        </div>
      </WindowFrame>
    )
  }

  if (os === 'windows' && windowsView === 'task-manager') {
    return (
      <WindowFrame platform={platform} title="Task Manager" width={500} height={330} inactive={inactive}>
        <div className="system-window__tm">
          <div className="system-window__tm-head">
            <strong>Startup apps</strong>
            <Button
              size="small"
              variant="normal"
              disabled={!registration}
              onClick={() => actions.systemSetApproved(!approved)}
            >
              {approved ? 'Disable' : 'Enable'}
            </Button>
          </div>
          <Table className="system-window__table">
            <TableHead>
              <TableCell head>Name</TableCell>
              <TableCell head>Publisher</TableCell>
              <TableCell head className="system-window__status">
                Status
              </TableCell>
            </TableHead>
            {OTHER_LOGIN_ITEMS.windows.map(item => (
              <TableRow key={item.name}>
                <TableCell>{item.name}</TableCell>
                <TableCell>{item.detail}</TableCell>
                <TableCell className="system-window__status">{item.enabled ? 'Enabled' : 'Disabled'}</TableCell>
              </TableRow>
            ))}
            {registration && (
              <TableRow active>
                <TableCell>{APP_NAME}</TableCell>
                <TableCell>—</TableCell>
                <TableCell className="system-window__status">{approved ? 'Enabled' : 'Disabled'}</TableCell>
              </TableRow>
            )}
          </Table>
          <p className="system-window__note">
            Disable sets the StartupApproved flag: the Run value stays, isEnabled() reads false, and enable() clears it.
          </p>
        </div>
      </WindowFrame>
    )
  }

  if (os === 'windows') {
    const data = registration ? commandLine(registration.executablePath, registration.arguments, os) : ''
    return (
      <WindowFrame platform={platform} title="Registry Editor" width={520} height={330} inactive={inactive}>
        <div className="system-window__regedit">
          <div className="system-window__address">Computer\HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run</div>
          <Table className="system-window__table system-window__table--wrap">
            <TableHead>
              <TableCell head>Name</TableCell>
              <TableCell head className="system-window__type">
                Type
              </TableCell>
              <TableCell head>Data</TableCell>
            </TableHead>
            <TableRow>
              <TableCell>OneDrive</TableCell>
              <TableCell className="system-window__type">REG_SZ</TableCell>
              <TableCell>"C:\Program Files\Microsoft OneDrive\OneDrive.exe" /background</TableCell>
            </TableRow>
            {registration && (
              <TableRow active>
                <TableCell>{config.id}</TableCell>
                <TableCell className="system-window__type">REG_SZ</TableCell>
                <TableCell>{data}</TableCell>
              </TableRow>
            )}
          </Table>
          <div className="system-window__address">…\Explorer\StartupApproved\Run</div>
          <Table className="system-window__table">
            <TableRow>
              <TableCell>{registration ? config.id : '(no value for this id)'}</TableCell>
              <TableCell className="system-window__type">{registration ? 'REG_BINARY' : ''}</TableCell>
              <TableCell>{registration ? `${approved ? '02' : '03'} 00 00 00 00 00 00 00 00 00 00 00` : ''}</TableCell>
            </TableRow>
          </Table>
        </div>
      </WindowFrame>
    )
  }

  // Linux: the autostart entry is a file, read back after every call.
  const path = `~/.config/autostart/${config.id}.desktop`
  const lines = registration
    ? desktopFile(registration.displayName, commandLine(registration.executablePath, registration.arguments, os))
    : [`cat: ${path}: No such file or directory`]
  return (
    <WindowFrame
      platform={platform}
      title={platform === 'omarchy' ? 'Alacritty' : 'Terminal'}
      width={500}
      height={330}
      inactive={inactive}
    >
      <pre className="system-window__terminal">
        <span className="system-window__prompt">
          <span>$</span> cat {path}
        </span>
        {lines.map(line => (
          <span key={line} className="system-window__line" data-missing={!registration || undefined}>
            {line}
          </span>
        ))}
        <span className="system-window__prompt">
          <span>$</span> <span className="system-window__cursor" />
        </span>
      </pre>
    </WindowFrame>
  )
}
