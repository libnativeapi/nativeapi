import { useCallback, useRef, useState } from 'react'

import { useEventLog } from '../../components/event-bar'
import type { Os } from '../../components/platform'
import { APP_NAME, SETTINGS_APPS } from './data'
import type { Front, TrustedApp } from './types'

export interface AccessibilityOptions {
  /** The user already allowed the example in System Settings (macOS). */
  granted?: boolean
  /** Start with enable() called and System Settings open on the Accessibility list. */
  settingsOpen?: boolean
}

export interface AccessibilityState {
  /** What System Settings says: the example's switch in the list. Always true off macOS. */
  allowed: boolean
  /** What the example last read back from `isEnabled()`. */
  trusted: boolean
  /** The example's row is in the list: `enable()` put it there. */
  listed: boolean
  /** The system's "would like to control this computer" alert is up. */
  prompt: boolean
  settingsOpen: boolean
  front: Front
  apps: TrustedApp[]
  enableCalls: number
  checks: number
  /** What `KeyboardMonitor.isMonitoring()` said after the last try, or null before one. */
  monitorStarted: boolean | null
}

/**
 * `AccessibilityManager`, simulated: on macOS the permission lives in System
 * Settings, which the example can only ask for and re-read; elsewhere there is
 * nothing to grant.
 */
export function useAccessibility(os: Os, options: AccessibilityOptions = {}) {
  const mac = os === 'macos'
  const allowedAtStart = !mac || Boolean(options.granted)
  const listedAtStart = mac && (Boolean(options.granted) || Boolean(options.settingsOpen))
  const { lastEvent, log, event, call, clear } = useEventLog(
    [
      `isEnabled → ${allowedAtStart}`,
      ...(options.settingsOpen ? ['enable()', 'System Settings opened on Privacy & Security ▸ Accessibility'] : []),
    ],
    allowedAtStart ? 'Trusted' : 'Not trusted',
  )
  const [state, setState] = useState<AccessibilityState>(() => ({
    allowed: allowedAtStart,
    trusted: allowedAtStart,
    listed: listedAtStart,
    prompt: false,
    settingsOpen: Boolean(options.settingsOpen) && mac,
    front: options.settingsOpen && mac ? 'settings' : 'example',
    apps: [...SETTINGS_APPS, ...(listedAtStart ? [{ name: APP_NAME, self: true, allowed: allowedAtStart }] : [])],
    enableCalls: options.settingsOpen ? 1 : 0,
    checks: 1,
    monitorStarted: null,
  }))
  const ref = useRef(state)
  ref.current = state
  const set = (patch: Partial<AccessibilityState>) => setState(s => ({ ...s, ...patch }))

  const check = useCallback(
    (why = '') => {
      const s = ref.current
      call(`isEnabled → ${s.allowed}${why}`)
      setState(prev => ({ ...prev, trusted: prev.allowed, checks: prev.checks + 1 }))
      if (s.allowed !== s.trusted) event(s.allowed ? 'Trusted' : 'Not trusted', `isEnabled changed → ${s.allowed}`)
    },
    [call, event],
  )

  const actions = {
    check: () => check(),
    enable: () => {
      setState(s => ({ ...s, enableCalls: s.enableCalls + 1 }))
      if (!mac) {
        call(`enable() · a no-op on ${os === 'windows' ? 'Windows' : 'Linux'}`)
        return
      }
      const s = ref.current
      if (s.allowed) {
        call('enable() · already trusted: no prompt')
        return
      }
      call('enable() → AXIsProcessTrustedWithOptions(prompt: true)')
      event('Permission requested', 'The system asks the user')
      // The first request adds the example to the list, switched off.
      setState(prev => ({
        ...prev,
        prompt: true,
        listed: true,
        apps: prev.listed ? prev.apps : [...prev.apps, { name: APP_NAME, self: true, allowed: false }],
      }))
    },
    openSettings: () => {
      set({ prompt: false, settingsOpen: true, front: 'settings' })
      event('System Settings opened', 'System Settings ▸ Privacy & Security ▸ Accessibility')
    },
    deny: () => {
      set({ prompt: false })
      event('Denied', 'The user dismissed the prompt')
    },
    /** The user flips a switch in System Settings: the system's state, not the example's. */
    setAllowed: (name: string, allowed: boolean) => {
      setState(s => ({
        ...s,
        apps: s.apps.map(app => (app.name === name ? { ...app, allowed } : app)),
        allowed: name === APP_NAME ? allowed : s.allowed,
      }))
      if (name === APP_NAME) event(allowed ? 'Allowed in System Settings' : 'Switched off in System Settings', `System Settings: ${APP_NAME} → ${allowed ? 'on' : 'off'}`)
    },
    closeSettings: () => set({ settingsOpen: false, front: 'example' }),
    /** A press on the example's window: it comes forward, and re-checks, as it does on every activation. */
    activate: () => {
      if (ref.current.front === 'example') return
      set({ front: 'example' })
      check(' · re-checked on ApplicationActivatedEvent')
    },
    frontSettings: () => set({ front: 'settings' }),
    tryKeyboard: () => {
      const started = !mac || ref.current.trusted
      call(`KeyboardMonitor.start() · isMonitoring → ${started}`)
      set({ monitorStarted: started })
      if (!started) event('Keyboard monitor blocked', 'CGEventTapCreate returned NULL: not trusted')
    },
    clearLog: clear,
  }

  return { state, actions, lastEvent, log }
}

export type AccessibilityActions = ReturnType<typeof useAccessibility>['actions']
