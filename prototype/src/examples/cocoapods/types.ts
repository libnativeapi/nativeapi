/** The six checks, one per nativeapi module the smoke test calls. */
export type CheckId = 'url' | 'tray' | 'accessibility' | 'display' | 'window' | 'preferences'

export type CheckStatus = 'idle' | 'running' | 'pass' | 'fail'

export interface CheckResult {
  status: CheckStatus
  /** What the call returned, or the error it threw. */
  detail: string
}

export type Tab = 'checks' | 'build'

/** What the Build tab shows the setup for. */
export type Target = 'macos' | 'ios'
