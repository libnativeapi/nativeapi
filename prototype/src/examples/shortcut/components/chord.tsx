import { cx, KeyCap, type KeyCapProps } from '@dazzlabs/dazzui'

import type { Os } from '../../../components/platform'
import './chord.css'

const MAC_GLYPHS: Record<string, string> = {
  ctrl: '⌃',
  control: '⌃',
  alt: '⌥',
  option: '⌥',
  shift: '⇧',
  cmd: '⌘',
  command: '⌘',
  meta: '⌘',
  super: '⌘',
  cmdorctrl: '⌘',
  commandorcontrol: '⌘',
}

export interface ChordProps {
  accelerator: string
  os: Os
  size?: KeyCapProps['size']
  className?: string
}

/** An accelerator as key caps: `Ctrl` `Shift` `A`, with the Mac's glyphs on macOS. */
export function Chord({ accelerator, os, size = 'small', className }: ChordProps) {
  const parts = accelerator.split('+').filter(Boolean)
  return (
    <span className={cx('chord', className)}>
      {parts.map((part, i) => (
        <KeyCap key={i} variant="key" size={size}>
          {os === 'macos' && i < parts.length - 1 ? (MAC_GLYPHS[part.toLowerCase()] ?? part) : part}
        </KeyCap>
      ))}
    </span>
  )
}
