/** `TitleBarStyle` (`window.h`). */
export type TitleBarStyle = 'normal' | 'hidden'

/** The three states the example puts the title bar in, as its chips name them. */
export type TitleBarState = 'normal' | 'under' | 'hidden'

/** What the window's getters return: the subject of the example. */
export interface TitleBarWindow {
  titleBarStyle: TitleBarStyle
  /** Recorded even while Hidden, where it does nothing. */
  contentUnderTitleBar: boolean
  /** `isWindowControlButtonsVisible`: macOS only, true wherever it is not implemented. */
  buttonsVisible: boolean
  maximized: boolean
  /** The frame on the desktop: kept when the state changes. */
  x: number
  y: number
  width: number
  height: number
}

export interface Size {
  width: number
  height: number
}
