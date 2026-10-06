/** `Brightness` (`application.h`): what `setBrightness` forces. */
export type Brightness = 'system' | 'light' | 'dark'

/** The `ApplicationEvent`s the example listens to. */
export type AppEventType = 'started' | 'activated' | 'deactivated' | 'quitRequested' | 'exiting'

/** The icons the example hands to `setIcon`; `missing` is a path that does not exist. */
export type AppIconPreset = 'default' | 'night' | 'amber' | 'beta' | 'missing'

/** Where the desktop shows the app's icon, and so its badge and progress. */
export type DockSurface = 'dock' | 'side-dock' | 'taskbar' | 'none'

export type Tab = 'lifecycle' | 'dock' | 'appearance' | 'windows'

/** One window the app opened: `getAllWindows` counts them. */
export interface AppWindow {
  id: number
  title: string
  /**
   * The appearance this window was given. On Windows `setBrightness` only
   * reaches the windows open at the time, so a window keeps what it had.
   */
  brightness: Brightness
}

/** How many of each event arrived, and when the last one did. */
export interface EventTally {
  count: number
  /** `performance.now()` of the last one. */
  at: number
}

/** `setProgressBar`'s argument: negative removes it, above 1 is busy. */
export type Progress = number
