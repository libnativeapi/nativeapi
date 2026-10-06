/** What a `LaunchAtLogin` manager holds locally: its getters' values. */
export interface Config {
  id: string
  displayName: string
  executablePath: string
  arguments: string[]
}

/**
 * What the OS has recorded, written by `enable()` from the config at that
 * moment — a later `setDisplayName` or `setProgram` changes nothing here
 * until `enable()` runs again.
 */
export interface Registration {
  displayName: string
  executablePath: string
  arguments: string[]
}

export type Tab = 'login' | 'program'

/** Which of Windows' own tools shows the registration. */
export type WindowsView = 'task-manager' | 'registry'

/** The session around the app: signed in, on the way out, on the way back. */
export type Session = 'running' | 'logging-out' | 'signing-in' | 'not-running'
