import { useCallback, useEffect, useRef, useState } from 'react'

import type { WindowFramePlatform } from '@dazzlabs/dazzui'

import { useEventLog } from '../../components/event-bar'
import { osOf } from '../../components/platform'
import { chordOf, matches, simulateIsValidAccelerator } from './accelerator'
import { SCOPE_NAMES, SEEDS, SYSTEM_TAKEN } from './data'
import type { Firing, RegistrationFailure, Scope, ShortcutDraft, ShortcutEntry } from './types'

export interface ShortcutOptions {
  /** The app has the focus as it starts. */
  focused?: boolean
  /** Registered after the seeds, as if from the Register tab. */
  extra?: ShortcutDraft[]
  /** Tried with register() as the example starts, after the seeds. */
  attempt?: ShortcutDraft
}

export interface ShortcutsState {
  shortcuts: ShortcutEntry[]
  managerEnabled: boolean
  focused: boolean
  selected: number | null
  failure: RegistrationFailure | null
  firing: Firing | null
  /** Bumped on each activation of a shortcut, so its header flashes again. */
  flash: Record<number, number>
}

type Result = { ok: true; entry: ShortcutEntry } | { ok: false; failure: RegistrationFailure }

const scopeName = (scope: Scope) => SCOPE_NAMES[scope]

/**
 * `ShortcutManager` and its `Shortcut`s, simulated against the real keyboard:
 * press a registered accelerator anywhere on the page and it fires — unless
 * the manager or the shortcut is disabled, or it is an Application shortcut
 * and the app does not have the focus.
 */
export function useShortcuts(platform: WindowFramePlatform, options: ShortcutOptions = {}) {
  const os = osOf(platform)
  const wayland = platform === 'omarchy'
  const { lastEvent, log, event, call, clear } = useEventLog(['isSupported → true'], 'No shortcut fired yet')
  const nextId = useRef(1)
  const [state, setState] = useState<ShortcutsState>({
    shortcuts: [],
    managerEnabled: true,
    focused: options.focused ?? true,
    selected: null,
    failure: null,
    firing: null,
    flash: {},
  })
  const ref = useRef(state)
  ref.current = state
  const fireTimer = useRef<number | undefined>(undefined)

  /** `register()`: the checks core makes, in its order, and the event each outcome emits. */
  const register = useCallback(
    (draft: ShortcutDraft, withOptions = true, callback: ShortcutEntry['callback'] = 'counter'): Result => {
      const { accelerator } = draft
      const call_ = withOptions
        ? `register({ accelerator: "${accelerator}", scope: ${scopeName(draft.scope)}${draft.description ? `, description: "${draft.description}"` : ''}${draft.enabled ? '' : ', enabled: false'} })`
        : `register("${accelerator}", callback)`
      const fail = (id: number, error: string): Result => {
        call(`${call_} → null`)
        event(`Could not register ${accelerator || 'an empty accelerator'}`, `ShortcutRegistrationFailedEvent #${id} "${accelerator}" → "${error}"`)
        const failure = { id, accelerator, error }
        setState(s => ({ ...s, failure }))
        return { ok: false, failure }
      }
      if (!simulateIsValidAccelerator(accelerator)) return fail(0, 'Invalid accelerator format')
      if (ref.current.shortcuts.some(s => s.accelerator === accelerator)) return fail(0, 'Accelerator already registered')
      const id = nextId.current++
      if (SYSTEM_TAKEN[os][accelerator]) return fail(id, 'Platform registration failed')
      const entry: ShortcutEntry = { ...draft, id, callback, activations: 0, calls: 0 }
      call(`${call_} → #${id}`)
      event(`Registered ${accelerator}`, `ShortcutRegisteredEvent #${id} "${accelerator}"`)
      ref.current = { ...ref.current, shortcuts: [...ref.current.shortcuts, entry] }
      setState(s => ({ ...s, shortcuts: [...s.shortcuts, entry], selected: id, failure: null }))
      return { ok: true, entry }
    },
    [call, event, os],
  )

  const started = useRef(false)
  useEffect(() => {
    if (started.current) return
    started.current = true
    for (const candidate of ['Ctrl+Shift+A', 'NotAKey'])
      call(`isValidAccelerator("${candidate}") → ${simulateIsValidAccelerator(candidate)} · isAvailable → true`)
    for (const seed of SEEDS) register({ ...seed, enabled: true }, seed.withOptions, seed.callback)
    for (const draft of options.extra ?? []) register(draft)
    if (options.attempt) register(options.attempt)
    setState(s => ({ ...s, selected: 1 }))
    // Once, as the example starts.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [])

  const runCallback = useCallback(
    (entry: ShortcutEntry) => {
      const calls = entry.calls + 1
      call(entry.callback === 'counter' ? `callback: ${entry.accelerator} fired ${calls} time(s)` : `callback: ${entry.accelerator} fired`)
      return calls
    },
    [call],
  )

  const update = (id: number, patch: (entry: ShortcutEntry) => Partial<ShortcutEntry>) =>
    setState(s => ({ ...s, shortcuts: s.shortcuts.map(e => (e.id === id ? { ...e, ...patch(e) } : e)) }))

  const onKey = useCallback(
    (e: KeyboardEvent) => {
      const target = e.target as HTMLElement | null
      if (target && (target.isContentEditable || /^(INPUT|TEXTAREA|SELECT)$/.test(target.tagName))) return
      const s = ref.current
      const entry = s.shortcuts.find(sc => {
        const chord = chordOf(sc.accelerator, os)
        return chord && matches(e, chord)
      })
      if (!entry || e.repeat) return
      // The system takes the chord: the page (and Storybook) never see it.
      e.preventDefault()
      e.stopPropagation()
      if (!s.managerEnabled || !entry.enabled) {
        call(`${entry.accelerator} pressed · ${s.managerEnabled ? 'shortcut' : 'manager'} disabled: nothing fires`)
        return
      }
      if (!s.focused && (entry.scope === 'application' || wayland)) {
        call(`${entry.accelerator} pressed · ${wayland ? 'a Wayland window has the focus: XWayland’s grab is blind to it' : 'Application scope and the app is not focused'}`)
        return
      }
      event(`${entry.accelerator} activated`, `ShortcutActivatedEvent #${entry.id} "${entry.accelerator}"`)
      const calls = runCallback(entry)
      update(entry.id, en => ({ activations: en.activations + 1, calls }))
      setState(prev => ({
        ...prev,
        flash: { ...prev.flash, [entry.id]: (prev.flash[entry.id] ?? 0) + 1 },
        firing: { key: performance.now(), accelerator: entry.accelerator, description: entry.description },
      }))
      clearTimeout(fireTimer.current)
      fireTimer.current = window.setTimeout(() => setState(prev => ({ ...prev, firing: null })), 1400)
    },
    [call, event, os, runCallback, wayland],
  )

  useEffect(() => {
    document.addEventListener('keydown', onKey)
    return () => document.removeEventListener('keydown', onKey)
  }, [onKey])
  useEffect(() => () => clearTimeout(fireTimer.current), [])

  const unregistered = (entry: ShortcutEntry) => {
    event(`Unregistered ${entry.accelerator}`, `ShortcutUnregisteredEvent #${entry.id} "${entry.accelerator}"`)
  }

  const actions = {
    register: (draft: ShortcutDraft) => register(draft),
    select: (id: number) => setState(s => ({ ...s, selected: id })),
    setEnabled: (id: number, enabled: boolean) => {
      call(`shortcut #${id}.setEnabled(${enabled}) · isEnabled → ${enabled}`)
      update(id, () => ({ enabled }))
    },
    setDescription: (id: number, description: string) => {
      call(`shortcut #${id}.setDescription("${description}") · getDescription → "${description}"`)
      update(id, () => ({ description }))
    },
    /** `invoke()`: the callback runs; no ShortcutActivatedEvent, as a test harness would want. */
    invoke: (id: number) => {
      const entry = ref.current.shortcuts.find(e => e.id === id)
      if (!entry) return
      call(`shortcut #${id}.invoke()`)
      const calls = runCallback(entry)
      update(id, () => ({ calls }))
    },
    unregister: (id: number, by: 'id' | 'accelerator' = 'id') => {
      const entry = ref.current.shortcuts.find(e => e.id === id)
      if (!entry) return
      call(by === 'id' ? `unregister(#${id}) → true` : `unregister("${entry.accelerator}") → true`)
      unregistered(entry)
      setState(s => {
        const shortcuts = s.shortcuts.filter(e => e.id !== id)
        return { ...s, shortcuts, selected: s.selected === id ? (shortcuts[0]?.id ?? null) : s.selected }
      })
    },
    unregisterAll: () => {
      const all = ref.current.shortcuts
      call(`unregisterAll → ${all.length}`)
      for (const entry of all) unregistered(entry)
      setState(s => ({ ...s, shortcuts: [], selected: null }))
    },
    setManagerEnabled: (enabled: boolean) => {
      call(`setEnabled(${enabled}) · isEnabled → ${enabled}`)
      setState(s => ({ ...s, managerEnabled: enabled }))
    },
    setFocused: (focused: boolean) => {
      if (ref.current.focused === focused) return
      event(focused ? 'App focused' : 'App lost the focus', focused ? 'ApplicationActivatedEvent' : 'ApplicationDeactivatedEvent')
      setState(s => ({ ...s, focused }))
    },
    /** `get(accelerator)`, `getAll`, `getByScope`: logged as the example reads them. */
    read: (line: string) => call(line),
    dismissFailure: () => setState(s => ({ ...s, failure: null })),
    clearLog: clear,
  }

  return { state, actions, lastEvent, log }
}

export type ShortcutActions = ReturnType<typeof useShortcuts>['actions']
