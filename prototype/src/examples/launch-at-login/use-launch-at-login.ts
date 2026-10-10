import { useEffect, useRef, useState } from 'react'

import { useEventLog } from '../../components/event-bar'
import type { Os } from '../../components/platform'
import { DEFAULT_CONFIG } from './data'
import type { Config, Registration, Session } from './types'

export interface LaunchAtLoginOptions {
  os: Os
  /** What `LaunchAtLogin.isSupported()` returns. */
  supported?: boolean
  /** Registered already as the example starts, with these arguments where the platform keeps them. */
  initialEnabled?: boolean
  initialArguments?: string[]
}

const list = (args: readonly string[]) => `[${args.map(a => `"${a}"`).join(', ')}]`

/**
 * One `LaunchAtLogin` manager made with the default constructor, and the
 * OS record behind it. The manager's setters only change its local config;
 * `enable()` writes the config into the OS, `disable()` takes it out, and
 * the user can switch it off from the system's own tools — which
 * `isEnabled()` then reports.
 */
export function useLaunchAtLogin({ os, supported = true, initialEnabled, initialArguments = [] }: LaunchAtLoginOptions) {
  const [config, setConfig] = useState<Config>(() => ({
    ...DEFAULT_CONFIG[os],
    arguments: os === 'macos' ? [] : initialArguments,
  }))
  const [registration, setRegistration] = useState<Registration | null>(() =>
    supported && initialEnabled
      ? { displayName: config.displayName, executablePath: config.executablePath, arguments: config.arguments }
      : null,
  )
  // Windows' StartupApproved flag: Task Manager's Disable clears it, enable() sets it again.
  const [approved, setApproved] = useState(true)
  const [session, setSession] = useState<Session>('running')
  const [launchedAtLogin, setLaunchedAtLogin] = useState<string[] | null>(null)
  const [run, setRun] = useState(1)
  const timers = useRef<number[]>([])
  useEffect(() => () => timers.current.forEach(clearTimeout), [])

  const enabled = supported && registration !== null && approved
  const log = useEventLog(
    supported
      ? ['isSupported() → true', 'LaunchAtLogin.create()', `getId() → "${config.id}"`, `isEnabled() → ${enabled}`]
      : ['isSupported() → false'],
    supported ? `Launch at login is ${enabled ? 'enabled' : 'disabled'}` : 'Launch at login is not supported here',
  )
  const { event, call } = log

  /** macOS registers the app bundle: a program other than this app makes enable() fail. */
  const programIsThisApp = config.executablePath === DEFAULT_CONFIG[os].executablePath

  const enable = () => {
    const ok = supported && (os !== 'macos' || programIsThisApp)
    if (ok) {
      setRegistration({ displayName: config.displayName, executablePath: config.executablePath, arguments: config.arguments })
      setApproved(true)
    }
    event(ok ? 'Launch at login enabled' : 'Could not enable launch at login', `enable() → ${ok}`)
    call(`isEnabled() → ${ok || enabled}`)
  }

  const disable = () => {
    const ok = supported
    if (ok) setRegistration(null)
    event(ok ? 'Launch at login disabled' : 'Could not disable launch at login', `disable() → ${ok}`)
    call(`isEnabled() → false`)
  }

  const setDisplayName = (name: string) => {
    const value = name.trim()
    if (!value) return event('Display name cannot be empty')
    setConfig(c => ({ ...c, displayName: value }))
    event(
      registration ? 'Display name set — enable() again to register it' : 'Display name set',
      `setDisplayName("${value}") → true`,
    )
  }

  const setProgram = (path: string, args: string[]) => {
    const value = path.trim()
    if (!value) return event('Executable path cannot be empty')
    setConfig(c => ({ ...c, executablePath: value, arguments: args }))
    event(
      registration ? 'Program set — enable() again to register it' : 'Program set',
      `setProgram("${value}", ${list(args)}) → true`,
    )
  }

  const refresh = () => {
    call(`isEnabled() → ${enabled}`)
    event(`Launch at login is ${enabled ? 'enabled' : 'disabled'}`)
  }

  /** From the system's own tools: Task Manager's Disable, the − in Login Items. */
  const systemSetApproved = (value: boolean) => setApproved(value)
  const systemRemove = () => setRegistration(null)

  /**
   * Log out, and sign in again: the session ends the app, and on the way
   * back the OS starts what is registered — with the arguments it recorded,
   * except on macOS, where SMAppService starts the bundle without any.
   */
  const logOutAndIn = () => {
    setSession('logging-out')
    const startsAtLogin = enabled
    const args = registration?.arguments ?? []
    timers.current.push(
      window.setTimeout(() => setSession('signing-in'), 900),
      window.setTimeout(() => {
        setRun(r => r + 1)
        log.clear()
        if (startsAtLogin) {
          setLaunchedAtLogin(os === 'macos' ? [] : args)
          setSession('running')
          call('isSupported() → true')
          event('Launched at login', `argv → ${list(os === 'macos' ? [] : args)}`)
        } else {
          setLaunchedAtLogin(null)
          setSession('not-running')
        }
      }, 2100),
    )
  }

  /** Opened by hand after a login that did not start it. */
  const open = () => {
    setLaunchedAtLogin(null)
    setSession('running')
    event(`Launch at login is ${enabled ? 'enabled' : 'disabled'}`, `isEnabled() → ${enabled}`)
  }

  const quit = () => setSession('not-running')

  return {
    os,
    supported,
    config,
    registration,
    approved,
    enabled,
    session,
    launchedAtLogin,
    run,
    log,
    programIsThisApp,
    actions: { enable, disable, setDisplayName, setProgram, refresh, logOutAndIn, open, quit, systemSetApproved, systemRemove },
  }
}

export type LaunchAtLoginState = ReturnType<typeof useLaunchAtLogin>
