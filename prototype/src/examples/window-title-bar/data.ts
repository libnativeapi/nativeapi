import type { WindowFramePlatform } from '@dazzlabs/dazzui'

import type { Size, TitleBarState } from './types'

export const WINDOW_TITLE = 'nativeapi · Title bar'

/** What the example asks for at start: `setContentSize(620×520)`, then `center()`. */
export const CONTENT_SIZE: Size = { width: 620, height: 520 }
export const MINIMUM_SIZE: Size = { width: 520, height: 420 }

/** The strip the example draws where a title bar would be; always there. */
export const STRIP_HEIGHT = 44
/** How far the strip's own controls start, clear of the macOS buttons over it. */
export const BUTTONS_INSET = 78

/** How tall each desktop's title bar is: what the content gains when it goes. */
export const TITLE_BAR_HEIGHT: Record<WindowFramePlatform, number> = {
  macos: 28,
  macos15: 28,
  windows: 32,
  gnome: 46,
  ubuntu: 46,
  kde: 30,
  omarchy: 0,
}

/** The note each state leaves in the event bar, as the real example words it. */
export const NOTES: Record<TitleBarState, string> = {
  normal: 'Standard title bar',
  under: 'The bar is a transparent overlay; its buttons stay',
  hidden: 'No title bar and no window buttons — move me by the strip',
}

/** "What to look for", from the real example. */
export const LOOK_FOR: readonly string[] = [
  'Hidden leaves no title bar and no window buttons on every platform. Move the window by the strip above; Quit ends the application (core has no Window.close() yet).',
  'Content under title bar keeps the bar and its buttons but stops it drawing: the strip runs to the top edge behind them. macOS only — elsewhere the chip is greyed out and the call returns false.',
  'Switching states keeps the window where it is and the same size; only contentSize above changes, by the height of the title bar.',
  'Hidden also stops the system moving the window when you drag the top of the content — that is what the strip is for.',
  'isContentUnderTitleBar stays as it was set while Hidden is on: with no title bar there is nothing for it to do, and it takes effect again on Normal.',
]
