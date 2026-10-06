import type { Os } from '../../components/platform'

/**
 * The raw codes each platform's monitor reports, by the page's
 * `KeyboardEvent.code`: macOS virtual keycodes (`kVK_*`, from the CGEvent),
 * Windows virtual-key codes (`vkCode` from the low-level hook — left and right
 * modifiers apart), X11 keycodes (the XInput2 `detail`: evdev + 8).
 */
export const KEYCODES: Record<string, { label: string; mac: number; win: number; x11: number }> = {
  Escape: { label: 'Esc', mac: 53, win: 0x1b, x11: 9 },
  F1: { label: 'F1', mac: 122, win: 0x70, x11: 67 },
  F2: { label: 'F2', mac: 120, win: 0x71, x11: 68 },
  F3: { label: 'F3', mac: 99, win: 0x72, x11: 69 },
  F4: { label: 'F4', mac: 118, win: 0x73, x11: 70 },
  F5: { label: 'F5', mac: 96, win: 0x74, x11: 71 },
  F6: { label: 'F6', mac: 97, win: 0x75, x11: 72 },
  F7: { label: 'F7', mac: 98, win: 0x76, x11: 73 },
  F8: { label: 'F8', mac: 100, win: 0x77, x11: 74 },
  F9: { label: 'F9', mac: 101, win: 0x78, x11: 75 },
  F10: { label: 'F10', mac: 109, win: 0x79, x11: 76 },
  F11: { label: 'F11', mac: 103, win: 0x7a, x11: 95 },
  F12: { label: 'F12', mac: 111, win: 0x7b, x11: 96 },
  Backquote: { label: '`', mac: 50, win: 0xc0, x11: 49 },
  Digit1: { label: '1', mac: 18, win: 0x31, x11: 10 },
  Digit2: { label: '2', mac: 19, win: 0x32, x11: 11 },
  Digit3: { label: '3', mac: 20, win: 0x33, x11: 12 },
  Digit4: { label: '4', mac: 21, win: 0x34, x11: 13 },
  Digit5: { label: '5', mac: 23, win: 0x35, x11: 14 },
  Digit6: { label: '6', mac: 22, win: 0x36, x11: 15 },
  Digit7: { label: '7', mac: 26, win: 0x37, x11: 16 },
  Digit8: { label: '8', mac: 28, win: 0x38, x11: 17 },
  Digit9: { label: '9', mac: 25, win: 0x39, x11: 18 },
  Digit0: { label: '0', mac: 29, win: 0x30, x11: 19 },
  Minus: { label: '-', mac: 27, win: 0xbd, x11: 20 },
  Equal: { label: '=', mac: 24, win: 0xbb, x11: 21 },
  Backspace: { label: 'Backspace', mac: 51, win: 0x08, x11: 22 },
  Tab: { label: 'Tab', mac: 48, win: 0x09, x11: 23 },
  KeyQ: { label: 'Q', mac: 12, win: 0x51, x11: 24 },
  KeyW: { label: 'W', mac: 13, win: 0x57, x11: 25 },
  KeyE: { label: 'E', mac: 14, win: 0x45, x11: 26 },
  KeyR: { label: 'R', mac: 15, win: 0x52, x11: 27 },
  KeyT: { label: 'T', mac: 17, win: 0x54, x11: 28 },
  KeyY: { label: 'Y', mac: 16, win: 0x59, x11: 29 },
  KeyU: { label: 'U', mac: 32, win: 0x55, x11: 30 },
  KeyI: { label: 'I', mac: 34, win: 0x49, x11: 31 },
  KeyO: { label: 'O', mac: 31, win: 0x4f, x11: 32 },
  KeyP: { label: 'P', mac: 35, win: 0x50, x11: 33 },
  BracketLeft: { label: '[', mac: 33, win: 0xdb, x11: 34 },
  BracketRight: { label: ']', mac: 30, win: 0xdd, x11: 35 },
  Backslash: { label: '\\', mac: 42, win: 0xdc, x11: 51 },
  CapsLock: { label: 'Caps', mac: 57, win: 0x14, x11: 66 },
  KeyA: { label: 'A', mac: 0, win: 0x41, x11: 38 },
  KeyS: { label: 'S', mac: 1, win: 0x53, x11: 39 },
  KeyD: { label: 'D', mac: 2, win: 0x44, x11: 40 },
  KeyF: { label: 'F', mac: 3, win: 0x46, x11: 41 },
  KeyG: { label: 'G', mac: 5, win: 0x47, x11: 42 },
  KeyH: { label: 'H', mac: 4, win: 0x48, x11: 43 },
  KeyJ: { label: 'J', mac: 38, win: 0x4a, x11: 44 },
  KeyK: { label: 'K', mac: 40, win: 0x4b, x11: 45 },
  KeyL: { label: 'L', mac: 37, win: 0x4c, x11: 46 },
  Semicolon: { label: ';', mac: 41, win: 0xba, x11: 47 },
  Quote: { label: "'", mac: 39, win: 0xde, x11: 48 },
  Enter: { label: 'Enter', mac: 36, win: 0x0d, x11: 36 },
  ShiftLeft: { label: 'Shift', mac: 56, win: 0xa0, x11: 50 },
  KeyZ: { label: 'Z', mac: 6, win: 0x5a, x11: 52 },
  KeyX: { label: 'X', mac: 7, win: 0x58, x11: 53 },
  KeyC: { label: 'C', mac: 8, win: 0x43, x11: 54 },
  KeyV: { label: 'V', mac: 9, win: 0x56, x11: 55 },
  KeyB: { label: 'B', mac: 11, win: 0x42, x11: 56 },
  KeyN: { label: 'N', mac: 45, win: 0x4e, x11: 57 },
  KeyM: { label: 'M', mac: 46, win: 0x4d, x11: 58 },
  Comma: { label: ',', mac: 43, win: 0xbc, x11: 59 },
  Period: { label: '.', mac: 47, win: 0xbe, x11: 60 },
  Slash: { label: '/', mac: 44, win: 0xbf, x11: 61 },
  ShiftRight: { label: 'Shift', mac: 60, win: 0xa1, x11: 62 },
  ControlLeft: { label: 'Ctrl', mac: 59, win: 0xa2, x11: 37 },
  AltLeft: { label: 'Alt', mac: 58, win: 0xa4, x11: 64 },
  MetaLeft: { label: 'Meta', mac: 55, win: 0x5b, x11: 133 },
  Space: { label: 'Space', mac: 49, win: 0x20, x11: 65 },
  MetaRight: { label: 'Meta', mac: 54, win: 0x5c, x11: 134 },
  AltRight: { label: 'Alt', mac: 61, win: 0xa5, x11: 108 },
  ControlRight: { label: 'Ctrl', mac: 62, win: 0xa3, x11: 105 },
  ArrowLeft: { label: '←', mac: 123, win: 0x25, x11: 113 },
  ArrowUp: { label: '↑', mac: 126, win: 0x26, x11: 111 },
  ArrowDown: { label: '↓', mac: 125, win: 0x28, x11: 116 },
  ArrowRight: { label: '→', mac: 124, win: 0x27, x11: 114 },
  Delete: { label: 'Del', mac: 117, win: 0x2e, x11: 119 },
  Home: { label: 'Home', mac: 115, win: 0x24, x11: 110 },
  End: { label: 'End', mac: 119, win: 0x23, x11: 115 },
  PageUp: { label: 'PgUp', mac: 116, win: 0x21, x11: 112 },
  PageDown: { label: 'PgDn', mac: 121, win: 0x22, x11: 117 },
  NumLock: { label: 'Num', mac: 71, win: 0x90, x11: 77 },
  ScrollLock: { label: 'Scroll', mac: -1, win: 0x91, x11: 78 },
}

/** The raw code `code` comes with on `os`, or -1 for a key the table lacks. */
export function keycodeOf(code: string, os: Os): number {
  const entry = KEYCODES[code]
  if (!entry) return -1
  return os === 'macos' ? entry.mac : os === 'windows' ? entry.win : entry.x11
}

/** How a keycode is written for `os`: Windows VKs in hex, as the SDK spells them. */
export const formatKeycode = (keycode: number, os: Os) =>
  keycode < 0 ? '?' : os === 'windows' ? `0x${keycode.toString(16).toUpperCase().padStart(2, '0')}` : String(keycode)

/** The keys that are modifiers to the browser, and so to the monitor's mask. */
export const MODIFIER_CODES = new Set([
  'ShiftLeft',
  'ShiftRight',
  'ControlLeft',
  'ControlRight',
  'AltLeft',
  'AltRight',
  'MetaLeft',
  'MetaRight',
  'CapsLock',
])

/** Every `ModifierKey` flag (`keyboard.h`), with whether this platform's monitor ever reports it. */
export function modifierFlagsOf(os: Os): readonly { name: string; bit: number; reported: boolean; note?: string }[] {
  return [
    { name: 'Shift', bit: 1 << 0, reported: true },
    { name: 'Ctrl', bit: 1 << 1, reported: true },
    { name: 'Alt', bit: 1 << 2, reported: true, note: os === 'macos' ? 'Option' : undefined },
    { name: 'Meta', bit: 1 << 3, reported: true, note: os === 'macos' ? 'Command' : os === 'windows' ? 'Windows key' : 'Super' },
    { name: 'Fn', bit: 1 << 4, reported: os === 'macos', note: os === 'macos' ? undefined : 'Not reported' },
    { name: 'CapsLock', bit: 1 << 5, reported: true, note: os === 'linux' ? 'While held' : 'While on' },
    { name: 'NumLock', bit: 1 << 6, reported: true, note: os === 'macos' ? 'Keypad and arrows' : os === 'linux' ? 'While held' : 'While on' },
    { name: 'ScrollLock', bit: 1 << 7, reported: os !== 'macos', note: os === 'macos' ? 'Not reported' : os === 'linux' ? 'While held' : 'While on' },
  ]
}

/** `0x0003 → Shift + Ctrl`, as the example prints a mask. */
export function describeMask(mask: number, os: Os) {
  const names = modifierFlagsOf(os)
    .filter(flag => mask & flag.bit)
    .map(flag => flag.name)
  return `0x${mask.toString(16).padStart(4, '0')} → ${names.length ? names.join(' + ') : 'none'}`
}

export interface KeySpec {
  code: string
  /** Width in key units. */
  w?: number
}

/** The keyboard the visualizer draws, row by row. */
export function layoutOf(os: Os): KeySpec[][] {
  const bottom: KeySpec[] =
    os === 'macos'
      ? [
          { code: 'Fn', w: 1 },
          { code: 'ControlLeft', w: 1 },
          { code: 'AltLeft', w: 1 },
          { code: 'MetaLeft', w: 1.25 },
          { code: 'Space', w: 5.5 },
          { code: 'MetaRight', w: 1.25 },
          { code: 'AltRight', w: 1 },
        ]
      : [
          { code: 'ControlLeft', w: 1.25 },
          { code: 'MetaLeft', w: 1.25 },
          { code: 'AltLeft', w: 1.25 },
          { code: 'Space', w: 5.5 },
          { code: 'AltRight', w: 1.25 },
          { code: 'ControlRight', w: 1.25 },
        ]
  const letters = (s: string) => s.split('').map(c => ({ code: `Key${c}` }))
  return [
    [{ code: 'Escape', w: 1.5 }, ...Array.from({ length: 12 }, (_, i) => ({ code: `F${i + 1}` }))],
    [
      { code: 'Backquote' },
      ...'1234567890'.split('').map(d => ({ code: `Digit${d}` })),
      { code: 'Minus' },
      { code: 'Equal' },
      { code: 'Backspace', w: 1.75 },
    ],
    [{ code: 'Tab', w: 1.5 }, ...letters('QWERTYUIOP'), { code: 'BracketLeft' }, { code: 'BracketRight' }, { code: 'Backslash', w: 1.25 }],
    [{ code: 'CapsLock', w: 1.75 }, ...letters('ASDFGHJKL'), { code: 'Semicolon' }, { code: 'Quote' }, { code: 'Enter', w: 2 }],
    [{ code: 'ShiftLeft', w: 2.25 }, ...letters('ZXCVBNM'), { code: 'Comma' }, { code: 'Period' }, { code: 'Slash' }, { code: 'ShiftRight', w: 2.5 }],
    [...bottom, { code: 'ArrowLeft' }, { code: 'ArrowUp' }, { code: 'ArrowDown' }, { code: 'ArrowRight' }],
  ]
}

/** A key's cap, in the platform's own words for its modifiers. */
export function capLabel(code: string, os: Os): string {
  if (code === 'Fn') return 'fn'
  const mac: Record<string, string> = { ControlLeft: '⌃', AltLeft: '⌥', AltRight: '⌥', MetaLeft: '⌘', MetaRight: '⌘', ShiftLeft: '⇧', ShiftRight: '⇧', CapsLock: '⇪', Enter: '↩', Backspace: '⌫', Tab: '⇥' }
  if (os === 'macos' && mac[code]) return mac[code]!
  if (code.startsWith('Meta')) return os === 'windows' ? 'Win' : 'Super'
  return KEYCODES[code]?.label ?? code
}

/** What a monitor can do here, before it is started. */
export function monitorNoteOf(platform: string, os: Os) {
  if (os === 'macos') return 'A session event tap: needs the Accessibility permission'
  if (os === 'windows') return 'A low-level keyboard hook (WH_KEYBOARD_LL): no permission'
  if (platform === 'omarchy') return 'XInput2 through XWayland: sees X11 windows’ keys only'
  return 'XInput2 raw events on the X display: no permission'
}
