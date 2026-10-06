import { useCallback, useEffect, useRef, useState } from 'react'

import type { WindowFramePlatform } from '@dazzlabs/dazzui'

import { useEventLog } from '../../components/event-bar'
import { osOf } from '../../components/platform'
import { describeMask, formatKeycode, KEYCODES, keycodeOf, MODIFIER_CODES } from './data'
import type { Blocked, Focus, KeyEventRecord, KeyEventType } from './types'

export interface KeyboardOptions {
  /** The process is trusted for accessibility (macOS). */
  trusted?: boolean
  /** Call `start()` as the example opens. */
  startAtStart?: boolean
  focus?: Focus
}

export interface KeyboardState {
  monitoring: boolean
  blocked: Blocked
  trusted: boolean
  focus: Focus
  /** The keys held down, by `KeyboardEvent.code`, as the monitor saw them. */
  pressed: ReadonlySet<string>
  /** The last mask a ModifierKeysChangedEvent carried. */
  mask: number
  /** Newest first. */
  events: KeyEventRecord[]
  counts: Record<KeyEventType, number>
  /** What the user typed into the Notes window while it had the focus. */
  notes: string
}

const LIMIT = 80
const NAV_KEYS = new Set(['Space', 'Tab', 'ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight', 'Backspace', 'Enter', 'PageUp', 'PageDown', 'Home', 'End'])

/** A page key event as the monitor's mask has it on `os`. */
function maskOf(e: KeyboardEvent, held: ReadonlySet<string>, os: ReturnType<typeof osOf>) {
  let mask = 0
  if (e.shiftKey) mask |= 1 << 0
  if (e.ctrlKey) mask |= 1 << 1
  if (e.altKey) mask |= 1 << 2
  if (e.metaKey) mask |= 1 << 3
  if (os === 'macos') {
    if (e.getModifierState('Fn')) mask |= 1 << 4
    if (e.getModifierState('CapsLock')) mask |= 1 << 5
    // kCGEventFlagMaskNumericPad: set while a keypad or arrow key is down.
    if ([...held].some(code => code.startsWith('Arrow') || code.startsWith('Numpad'))) mask |= 1 << 6
  } else if (os === 'windows') {
    // GetAsyncKeyState's toggle bit: the lock is on.
    if (e.getModifierState('CapsLock')) mask |= 1 << 5
    if (e.getModifierState('NumLock')) mask |= 1 << 6
    if (e.getModifierState('ScrollLock')) mask |= 1 << 7
  } else {
    // XQueryKeymap: the lock key is physically down.
    if (held.has('CapsLock')) mask |= 1 << 5
    if (held.has('NumLock')) mask |= 1 << 6
    if (held.has('ScrollLock')) mask |= 1 << 7
  }
  return mask
}

/**
 * `KeyboardMonitor`, fed by the real keyboard: while it monitors, every key
 * pressed on the page arrives as the platform's monitor would deliver it —
 * its raw keycode, and the modifier mask in each platform's own rhythm (macOS
 * sends modifier keys as flag changes only; Windows sends the mask when it
 * changes; X11 after every key). Whichever window has the focus.
 */
export function useKeyboardMonitor(platform: WindowFramePlatform, options: KeyboardOptions = {}) {
  const os = osOf(platform)
  const trustedAtStart = os !== 'macos' || options.trusted !== false
  const startsOk = Boolean(options.startAtStart) && trustedAtStart
  const { lastEvent, log, event, call, clear } = useEventLog(
    options.startAtStart ? ['KeyboardMonitor()', 'start()', `isMonitoring → ${startsOk}`] : ['KeyboardMonitor()', 'isMonitoring → false'],
    startsOk ? 'Monitoring: press some keys' : options.startAtStart ? 'Monitor did not start' : 'Not monitoring',
  )
  const [state, setState] = useState<KeyboardState>(() => ({
    monitoring: startsOk,
    blocked: options.startAtStart && !trustedAtStart ? 'permission' : null,
    trusted: trustedAtStart,
    focus: options.focus ?? 'example',
    pressed: new Set(),
    mask: 0,
    events: [],
    counts: { pressed: 0, released: 0, modifiers: 0 },
    notes: '',
  }))
  const ref = useRef(state)
  ref.current = state
  const held = useRef(new Set<string>())
  const lastMask = useRef(0)
  const number = useRef(0)

  const handle = useCallback(
    (e: KeyboardEvent, down: boolean) => {
      const s = ref.current
      const code = e.code || e.key
      if (down) held.current.add(code)
      else held.current.delete(code)
      const editing = e.target instanceof HTMLElement && (e.target.isContentEditable || /^(INPUT|TEXTAREA|SELECT)$/.test(e.target.tagName))

      // The Notes window has the focus: the keys are typed into it.
      if (s.focus === 'notes' && down && !editing) {
        const notes =
          e.key === 'Backspace' ? s.notes.slice(0, -1) : e.key === 'Enter' ? `${s.notes}\n` : e.key.length === 1 && !e.metaKey && !e.ctrlKey ? s.notes + e.key : s.notes
        if (notes !== s.notes) setState(prev => ({ ...prev, notes }))
      }

      // A Wayland window's keys never reach an X11 monitor.
      const delivered = s.monitoring && !(platform === 'omarchy' && s.focus === 'notes')
      if ((s.monitoring || s.focus === 'notes') && !editing) {
        // Keep the page's own shortcuts (and Storybook's) out of it.
        e.stopPropagation()
        if (NAV_KEYS.has(code) || (s.focus === 'notes' && e.key.length === 1)) e.preventDefault()
      }
      if (!delivered) return

      const mask = maskOf(e, held.current, os)
      const records: KeyEventRecord[] = []
      const at = performance.now()
      const add = (type: KeyEventType, keycode: number, modifiers = 0) =>
        records.push({ number: ++number.current, type, keycode, key: KEYCODES[code]?.label ?? (e.key.length === 1 ? e.key.toUpperCase() : code), modifiers, repeat: e.repeat, at })
      const keycode = keycodeOf(code, os) >= 0 ? keycodeOf(code, os) : os === 'windows' ? e.keyCode : -1
      const isModifier = MODIFIER_CODES.has(code)

      if (os === 'macos') {
        // A modifier key is a flagsChanged event: no press or release.
        if (isModifier) add('modifiers', 0, mask)
        else add(down ? 'pressed' : 'released', keycode)
      } else {
        add(down ? 'pressed' : 'released', keycode)
        if (os === 'linux' || mask !== lastMask.current) add('modifiers', 0, mask)
      }
      if (records.some(r => r.type === 'modifiers')) lastMask.current = mask
      const newest = [...records].reverse()
      const pressed = new Set(held.current)

      setState(prev => {
        const counts = { ...prev.counts }
        for (const r of records) counts[r.type] += 1
        return {
          ...prev,
          pressed,
          mask: records.some(r => r.type === 'modifiers') ? mask : prev.mask,
          events: [...newest, ...prev.events].slice(0, LIMIT),
          counts,
        }
      })
      for (const r of records) {
        if (r.type === 'modifiers') event(`Modifiers ${describeMask(r.modifiers, os).split(' → ')[1]}`, `ModifierKeysChangedEvent #${r.number} → ${describeMask(r.modifiers, os)}`)
        else
          event(
            `${r.key} ${r.type}${r.repeat ? ' (repeat)' : ''}`,
            `${r.type === 'pressed' ? 'KeyPressedEvent' : 'KeyReleasedEvent'} #${r.number} → keycode ${formatKeycode(r.keycode, os)}`,
          )
      }
    },
    [event, os, platform],
  )

  useEffect(() => {
    // Bubble phase on the document: the page's controls see their keys
    // first, and the window (where Storybook listens) never does.
    const onDown = (e: KeyboardEvent) => handle(e, true)
    const onUp = (e: KeyboardEvent) => handle(e, false)
    const onBlur = () => {
      held.current.clear()
      setState(prev => ({ ...prev, pressed: new Set() }))
    }
    document.addEventListener('keydown', onDown)
    document.addEventListener('keyup', onUp)
    window.addEventListener('blur', onBlur)
    return () => {
      document.removeEventListener('keydown', onDown)
      document.removeEventListener('keyup', onUp)
      window.removeEventListener('blur', onBlur)
    }
  }, [handle])

  const actions = {
    start: () => {
      call('start()')
      if (os === 'macos' && !ref.current.trusted) {
        call('isMonitoring → false · CGEventTapCreate returned NULL')
        event('Monitor did not start')
        setState(prev => ({ ...prev, monitoring: false, blocked: 'permission' }))
        return
      }
      call('isMonitoring → true')
      event('Monitoring: press some keys')
      setState(prev => ({ ...prev, monitoring: true, blocked: null }))
    },
    stop: () => {
      call('stop()')
      call('isMonitoring → false')
      event('Stopped')
      held.current.clear()
      setState(prev => ({ ...prev, monitoring: false, pressed: new Set() }))
    },
    /** The user allows the example in System Settings, then it starts again. */
    grant: () => {
      call('System Settings: Accessibility Example → on')
      call('AccessibilityManager.isEnabled → true')
      setState(prev => ({ ...prev, trusted: true }))
      event('Accessibility granted')
    },
    setFocus: (focus: Focus) => {
      if (ref.current.focus === focus) return
      setState(prev => ({ ...prev, focus }))
    },
    clearEvents: () => {
      clear()
      setState(prev => ({ ...prev, events: [], counts: { pressed: 0, released: 0, modifiers: 0 } }))
    },
    clearNotes: () => setState(prev => ({ ...prev, notes: '' })),
  }

  return { state, actions, lastEvent, log }
}

export type KeyboardActions = ReturnType<typeof useKeyboardMonitor>['actions']
