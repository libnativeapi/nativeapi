import type { Os } from '../../components/platform'
import type { Config } from './types'

const EXE = 'launch_at_login_example'

/**
 * What the default constructor resolves on each desktop: the bundle
 * identifier and name on macOS; elsewhere an id built from the executable's
 * name, and that name as the display name.
 */
export const DEFAULT_CONFIG: Record<Os, Config> = {
  macos: {
    id: 'com.example.launchAtLoginExample',
    displayName: EXE,
    executablePath: `/Applications/${EXE}.app/Contents/MacOS/${EXE}`,
    arguments: [],
  },
  windows: {
    id: `com.nativeapi.launch_at_login.${EXE}`,
    displayName: EXE,
    executablePath: `C:\\Users\\ada\\AppData\\Local\\Programs\\${EXE}\\${EXE}.exe`,
    arguments: [],
  },
  linux: {
    id: `com.nativeapi.launch_at_login.${EXE}`,
    displayName: EXE,
    executablePath: `/home/ada/${EXE}/bundle/${EXE}`,
    arguments: [],
  },
}

/** What the example's "Program" tab offers to set where the platform takes one. */
export const SAMPLE_ARGUMENTS = '--minimized --from-login'

/** The other things that start at login, so the app's row has company. */
export const OTHER_LOGIN_ITEMS: Record<Os, readonly { name: string; detail: string; enabled: boolean }[]> = {
  macos: [
    { name: 'Rectangle', detail: 'Application', enabled: true },
    { name: 'Dropbox', detail: 'Application', enabled: true },
  ],
  windows: [
    { name: 'Microsoft OneDrive', detail: 'Microsoft Corporation', enabled: true },
    { name: 'Microsoft Teams', detail: 'Microsoft', enabled: false },
  ],
  linux: [],
}

/** How a command line quotes its program and arguments: Windows' Run value, the .desktop Exec line. */
export function commandLine(path: string, args: readonly string[], os: Os) {
  const quote = (s: string) => (/[\s"]/.test(s) ? `"${s.replaceAll('"', '\\"')}"` : s)
  // The Run value always quotes the program; the Exec line quotes only what needs it.
  const program = os === 'windows' ? `"${path}"` : quote(path)
  return [program, ...args.map(quote)].join(' ')
}

/** The .desktop file `enable()` writes on Linux. */
export function desktopFile(name: string, exec: string) {
  return [
    '[Desktop Entry]',
    'Type=Application',
    `Name=${name}`,
    `Comment=LaunchAtLogin entry for ${name}`,
    `Exec=${exec}`,
    'X-GNOME-Autostart-enabled=true',
    'Hidden=false',
    'X-KDE-autostart-after=panel',
  ]
}
