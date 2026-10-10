import type { Os } from '../../components/platform'
import type { Dependent, TrustedApp } from './types'

export const APP_NAME = 'Accessibility Example'

/** The list System Settings shows before the example asks: other apps the user trusted. */
export const SETTINGS_APPS: readonly TrustedApp[] = [
  { name: 'Terminal', allowed: true },
  { name: 'Raycast', allowed: true },
  { name: 'Zoom', allowed: false },
]

/** What `AccessibilityManager` does on each platform. */
export const PLATFORM_BEHAVIOUR: readonly { os: Os; name: string; isEnabled: string; enable: string }[] = [
  {
    os: 'macos',
    name: 'macOS',
    isEnabled: 'AXIsProcessTrustedWithOptions: true once the app is allowed in Privacy & Security ▸ Accessibility',
    enable: 'Prompts, and adds the app to that list switched off; the user switches it on',
  },
  {
    os: 'windows',
    name: 'Windows',
    isEnabled: 'Always true: nothing gates the accessibility and input APIs',
    enable: 'A no-op',
  },
  {
    os: 'linux',
    name: 'Linux',
    isEnabled: 'True while assistive technology is active: an AT-SPI bus, Orca, GNOME_ACCESSIBILITY',
    enable: 'Initialises GTK’s accessibility bridge; nothing to grant',
  },
]

/** The other examples' features that hang on the permission, per platform. */
export function dependentsOf(os: Os): Dependent[] {
  return [
    {
      id: 'keyboard',
      name: 'Keyboard monitor',
      api: 'KeyboardMonitor.start()',
      needs: os === 'macos',
      why:
        os === 'macos'
          ? 'A session event tap sees other apps’ keys only for a trusted process'
          : os === 'windows'
            ? 'A low-level keyboard hook: no permission'
            : 'XInput2 on the X display: no permission, but no Wayland',
    },
    {
      id: 'shortcuts',
      name: 'Global shortcuts',
      api: 'ShortcutManager.register()',
      needs: false,
      why:
        os === 'macos'
          ? 'Carbon hot keys are delivered without it'
          : os === 'windows'
            ? 'RegisterHotKey: no permission'
            : 'XGrabKey on the root window: no permission',
    },
  ]
}
