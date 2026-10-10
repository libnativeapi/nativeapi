import { WindowFrame, type WindowFramePlatform } from '@dazzlabs/dazzui'

import './notes-window.css'

export interface NotesWindowProps {
  platform: WindowFramePlatform
  text: string
  focused: boolean
  /** Hyprland: a native Wayland window, out of an X11 monitor's sight. */
  wayland?: boolean
}

/**
 * Another app's window: give it the focus and type — the keys land here, and
 * the global monitor still reports every one of them to the example.
 */
export function NotesWindow({ platform, text, focused, wayland }: NotesWindowProps) {
  return (
    <WindowFrame platform={platform} title="Notes" width={300} height={220} inactive={!focused} className="notes-window">
      <div className="notes-window__page">
        {text || !focused ? (
          <span className="notes-window__text">{text}</span>
        ) : (
          <span className="notes-window__placeholder">Type here…</span>
        )}
        {focused && <span className="notes-window__caret" />}
      </div>
      <div className="notes-window__foot">
        {wayland ? 'A Wayland window: an X11 monitor sees none of its keys' : 'Another app: its keys still reach the monitor'}
      </div>
    </WindowFrame>
  )
}
