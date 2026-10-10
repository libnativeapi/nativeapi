import { ChevronDown12Regular, Folder16Filled, Folder16Regular } from '@fluentui/react-icons'

import { cx, Icon, Table, TableCell, TableHead, TableRow, WindowFrame, type WindowFramePlatform } from '@dazzlabs/dazzui'

import { osOf } from '../../../components/platform'
import { SCOPE } from '../data'
import type { Store } from '../types'
import type { Changed } from '../use-preferences-app'
import './store-window.css'

export interface StoreWindowProps {
  platform: WindowFramePlatform
  store: Store
  changed: Changed | null
  /** Behind the example: it greys out its title. */
  inactive?: boolean
}

const DOMAIN = `com.nativeapi.preferences.${SCOPE}`
const FILE = `~/.config/nativeapi/preferences_${SCOPE}.conf`

/** `defaults read` quotes what is not a bare word. */
const bare = (s: string) => (/^[A-Za-z0-9_.]+$/.test(s) ? s : `"${s.replaceAll('"', '\\"')}"`)

/**
 * The store as the system shows it, beside the app and outliving it: `defaults
 * read` of the suite on macOS, Registry Editor on the scope's key on Windows,
 * the XDG config file on Linux. It is redrawn on every call, the line the
 * last one wrote picked out — and it stays put while the app relaunches.
 */
export function StoreWindow({ platform, store, changed, inactive }: StoreWindowProps) {
  const os = osOf(platform)
  const sorted = Object.entries(store).sort(([a], [b]) => a.localeCompare(b))
  const touched = changed?.keys ?? []
  const lineKey = (key: string) => (touched.includes(key) ? `${key}#${changed?.n}` : key)

  if (os === 'windows') {
    return (
      <WindowFrame platform={platform} title="Registry Editor" width={520} height={330} inactive={inactive}>
        <div className="store-window__regedit">
          <div className="store-window__address">Computer\HKEY_CURRENT_USER\Software\NativeAPI\Preferences\{SCOPE}</div>
          <div className="store-window__split">
            <ul className="store-window__tree">
              {['HKEY_CURRENT_USER', 'Software', 'NativeAPI', 'Preferences', SCOPE].map((name, depth) => (
                <li key={name} style={{ paddingInlineStart: `calc(var(--spacing-2-5) * ${depth})` }} data-selected={name === SCOPE || undefined}>
                  <Icon icon={ChevronDown12Regular} size={10} />
                  <Icon icon={name === SCOPE ? Folder16Filled : Folder16Regular} size={14} />
                  {name}
                </li>
              ))}
            </ul>
            <Table className="store-window__values">
              <TableHead>
                <TableCell head>Name</TableCell>
                <TableCell head className="store-window__type">
                  Type
                </TableCell>
                <TableCell head>Data</TableCell>
              </TableHead>
              <TableRow>
                <TableCell>(Default)</TableCell>
                <TableCell className="store-window__type">REG_SZ</TableCell>
                <TableCell className="store-window__faint">(value not set)</TableCell>
              </TableRow>
              {sorted.map(([key, value]) => (
                <TableRow key={lineKey(key)} data-changed={touched.includes(key) || undefined}>
                  <TableCell>{key}</TableCell>
                  <TableCell className="store-window__type">REG_SZ</TableCell>
                  <TableCell>{value}</TableCell>
                </TableRow>
              ))}
            </Table>
          </div>
        </div>
      </WindowFrame>
    )
  }

  // macOS and Linux: a terminal that reads the store again after every call.
  const command = os === 'macos' ? `defaults read ${DOMAIN}` : `cat ${FILE}`
  const lines: { text: string; key?: string }[] =
    os === 'macos'
      ? [
          { text: '{' },
          ...sorted.map(([key, value]) => ({ text: `    ${bare(key)} = ${bare(value)};`, key })),
          { text: '}' },
        ]
      : [
          { text: `# NativeAPI Preferences - ${SCOPE}` },
          ...sorted.map(([key, value]) => ({ text: `${key}=${value.replaceAll('\n', '\\n')}`, key })),
        ]

  return (
    <WindowFrame
      platform={platform}
      title={platform === 'omarchy' ? 'Alacritty' : 'Terminal'}
      width={460}
      height={330}
      inactive={inactive}
      className="store-window"
    >
      <pre className="store-window__terminal">
        <span className="store-window__prompt">
          <span>$</span> {command}
        </span>
        {lines.map((line, i) => (
          <span
            key={line.key ? lineKey(line.key) : `${i}-${line.text}`}
            className={cx('store-window__line', line.key && touched.includes(line.key) && 'store-window__line--changed')}
          >
            {line.text}
          </span>
        ))}
        <span className="store-window__prompt">
          <span>$</span> <span className="store-window__cursor" />
        </span>
      </pre>
    </WindowFrame>
  )
}
