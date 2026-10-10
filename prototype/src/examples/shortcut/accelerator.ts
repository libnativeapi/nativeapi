import type { Os } from '../../components/platform'

/**
 * Accelerator strings, as `ShortcutManager` reads them: `[Modifier+]*Key`,
 * Electron style. These mirror core's checks so the Register tab can answer
 * as `isValidAccelerator` would, and the page's key presses can be matched
 * against what was registered.
 */
const MODIFIER = /^(ctrl|control|alt|option|shift|cmd|command|super|meta|cmdorctrl|commandorcontrol)$/i
const KEY =
  /^(F1[0-9]|F2[0-4]|F[1-9]|Num[0-9]|NumDec|NumAdd|NumSub|NumMult|NumDiv|NumEnter|Space|Tab|Enter|Return|Escape|Esc|Backspace|ForwardDelete|Delete|Insert|Help|Home|End|PageUp|PageDown|Up|Down|Left|Right|Plus|Minus|Equal|Comma|Period|Slash|Backslash|Semicolon|Quote|LeftBracket|RightBracket|Grave|Backquote|[A-Za-z0-9]|[,./\\;'[\]`=-])$/i

/** `isValidAccelerator`: the same pattern core matches. */
export function simulateIsValidAccelerator(accelerator: string): boolean {
  if (!accelerator) return false
  const parts = accelerator.split('+')
  // A trailing "+" is the Plus key spelt as a character only in "Plus".
  const key = parts.pop()!
  return parts.every(part => MODIFIER.test(part)) && KEY.test(key)
}

interface Chord {
  ctrl: boolean
  alt: boolean
  shift: boolean
  meta: boolean
  /** The page's `KeyboardEvent.code` for the key. */
  code: string
}

const NAMED: Record<string, string> = {
  space: 'Space',
  tab: 'Tab',
  enter: 'Enter',
  return: 'Enter',
  escape: 'Escape',
  esc: 'Escape',
  backspace: 'Backspace',
  delete: 'Delete',
  forwarddelete: 'Delete',
  insert: 'Insert',
  home: 'Home',
  end: 'End',
  pageup: 'PageUp',
  pagedown: 'PageDown',
  up: 'ArrowUp',
  down: 'ArrowDown',
  left: 'ArrowLeft',
  right: 'ArrowRight',
  minus: 'Minus',
  '-': 'Minus',
  equal: 'Equal',
  '=': 'Equal',
  plus: 'Equal',
  comma: 'Comma',
  ',': 'Comma',
  period: 'Period',
  '.': 'Period',
  slash: 'Slash',
  '/': 'Slash',
  backslash: 'Backslash',
  '\\': 'Backslash',
  semicolon: 'Semicolon',
  ';': 'Semicolon',
  quote: 'Quote',
  "'": 'Quote',
  leftbracket: 'BracketLeft',
  '[': 'BracketLeft',
  rightbracket: 'BracketRight',
  ']': 'BracketRight',
  grave: 'Backquote',
  backquote: 'Backquote',
  '`': 'Backquote',
}

/** The keys an accelerator names, as the page's key events spell them; null when it is not valid. */
export function chordOf(accelerator: string, os: Os): Chord | null {
  if (!simulateIsValidAccelerator(accelerator)) return null
  const parts = accelerator.split('+')
  const key = parts.pop()!
  const chord: Chord = { ctrl: false, alt: false, shift: false, meta: false, code: '' }
  for (const part of parts.map(p => p.toLowerCase())) {
    if (part === 'ctrl' || part === 'control') chord.ctrl = true
    else if (part === 'alt' || part === 'option') chord.alt = true
    else if (part === 'shift') chord.shift = true
    else if (part === 'cmdorctrl' || part === 'commandorcontrol') chord[os === 'macos' ? 'meta' : 'ctrl'] = true
    else chord.meta = true
  }
  const lower = key.toLowerCase()
  if (/^[a-z]$/.test(lower)) chord.code = `Key${lower.toUpperCase()}`
  else if (/^[0-9]$/.test(lower)) chord.code = `Digit${lower}`
  else if (/^f\d+$/.test(lower)) chord.code = key.toUpperCase()
  else if (/^num\d$/.test(lower)) chord.code = `Numpad${lower.slice(3)}`
  else chord.code = NAMED[lower] ?? key
  return chord
}

/** Whether a page key press is the chord. */
export function matches(event: KeyboardEvent, chord: Chord) {
  return (
    event.code === chord.code &&
    event.ctrlKey === chord.ctrl &&
    event.altKey === chord.alt &&
    event.shiftKey === chord.shift &&
    event.metaKey === chord.meta
  )
}

const GLYPH_KEYS: Record<string, string> = {
  '↩': 'Enter',
  '⌤': 'NumEnter',
  '⇥': 'Tab',
  '⌫': 'Backspace',
  '⌦': 'Delete',
  '↑': 'Up',
  '↓': 'Down',
  '←': 'Left',
  '→': 'Right',
  '↖': 'Home',
  '↘': 'End',
  '⇞': 'PageUp',
  '⇟': 'PageDown',
  '⎋': 'Escape',
}

/**
 * DazzUI's ShortcutRecorder prints a chord in glyphs (`⌃⇧A`, `⌥ Space`);
 * spell it as an accelerator in this platform's words: Cmd on macOS, Super
 * for the Windows or logo key elsewhere.
 */
export function acceleratorFromGlyphs(glyphs: string, os: Os): string {
  const parts: string[] = []
  let rest = glyphs
  const names: Record<string, string> = { '⌃': 'Ctrl', '⌥': 'Alt', '⇧': 'Shift', '⌘': os === 'macos' ? 'Cmd' : 'Super' }
  while (rest && names[rest[0]!]) {
    parts.push(names[rest[0]!]!)
    rest = rest.slice(1)
  }
  const key = rest.trim()
  parts.push(GLYPH_KEYS[key] ?? key)
  return parts.join('+')
}

/** How a platform names its keys in an accelerator, for the syntax note. */
export const SYNTAX: Record<Os, string> = {
  macos: 'Ctrl (⌃), Alt or Option (⌥), Shift, Cmd (⌘) — CmdOrCtrl is Cmd here',
  windows: 'Ctrl, Alt, Shift, Super or Meta for the Windows key — CmdOrCtrl is Ctrl here',
  linux: 'Ctrl, Alt, Shift, Super for the logo key — CmdOrCtrl is Ctrl here',
}
