import { type RefObject, useCallback, useEffect, useMemo, useRef, useState } from 'react'

import type { Os } from '../../components/platform'
import {
  ACCENTS,
  ALIGNMENTS,
  BACKEND_NAMES,
  BOXES,
  DEFAULT_SUBTITLE,
  FLASH_DURATION,
  MAX_TASKS,
  SIGN_IN_DELAY,
} from './data'
import type { BoxName, LogEntry, Task, ViewAlignmentName, ViewEventType, WorkbenchStart } from './types'
import { clipText, viewElement } from './view-tree'

const LOG_CAPACITY = 300

export type StatusTone = 'muted' | 'success' | 'danger'

export interface SignInState {
  username: string
  password: string
  secure: boolean
  status: { text: string; tone: StatusTone }
  pending: boolean
  signedInAs: string | null
}

export interface PlaygroundState {
  layout: 'row' | 'column'
  alignment: ViewAlignmentName
  spacing: number
  padding: number
  order: BoxName[]
  hiddenB: boolean
}

const stamp = (date = new Date()) => {
  const two = (n: number) => String(n).padStart(2, '0')
  return `${two(date.getHours())}:${two(date.getMinutes())}:${two(date.getSeconds())}.${String(date.getMilliseconds()).padStart(3, '0')}`
}

/** A seeded shuffle, as the example's `Random(7)`: the same presses give the same orders. */
function shuffled<T>(items: readonly T[], seed: number): T[] {
  const out = [...items]
  let state = seed >>> 0
  for (let i = out.length - 1; i > 0; i--) {
    state = (state * 1664525 + 1013904223) >>> 0
    const j = state % (i + 1)
    ;[out[i], out[j]] = [out[j]!, out[i]!]
  }
  return out
}

/**
 * The workbench's logic, simulated: what each section's buttons and fields
 * do, the shared event log both windows read (every view event in order,
 * a run of keystrokes folded into one line, and the example's own notes),
 * and the inspector's finder and flash. `rootRef` is the workbench's root
 * view, for moving the focus the way `focus()` does.
 */
export function useWorkbench(os: Os, start: WorkbenchStart, rootRef: RefObject<HTMLElement | null>) {
  const backend = BACKEND_NAMES[os]

  // --- The log both windows share ------------------------------------------
  const nextId = useRef(0)
  const [log, setLog] = useState<{ entries: LogEntry[]; total: number }>(() => {
    const notes: [string, string][] = [['app', `started on the ${backend} backend`]]
    for (const task of start.tasks ?? []) notes.push(['tasks', `added: ${task.title}`])
    for (const task of start.tasks ?? []) if (task.done) notes.push(['tasks', `done: ${task.title}`])
    if (start.signedInAs) notes.push(['signIn', `signed in as "${start.signedInAs}"`])
    if (start.layout === 'column') notes.push(['playground', 'layout → column'])
    if (start.inspectorOpen) notes.push(['app', 'inspector opened'])
    const entries = notes.map(([source, message]) => ({ id: nextId.current++, time: stamp(), source, message, repeats: 1 }))
    return { entries, total: entries.length }
  })

  const add = useCallback((source: string, message: string, type?: ViewEventType) => {
    setLog(({ entries, total }) => {
      const last = entries.at(-1)
      // Typing sends one event per keystroke: fold a run of them into the latest text.
      if (type === 'TextFieldChangedEvent' && last?.source === source && last.type === type) {
        const folded = { ...last, message, time: stamp(), repeats: last.repeats + 1 }
        return { entries: [...entries.slice(0, -1), folded], total: total + 1 }
      }
      const entry = { id: nextId.current++, time: stamp(), source, message, type, repeats: 1 }
      return { entries: [...entries, entry].slice(-LOG_CAPACITY), total: total + 1 }
    })
  }, [])

  const note = useCallback((source: string, message: string) => add(source, message), [add])
  const emit = useCallback(
    (source: string, type: ViewEventType, text?: string) =>
      add(source, type === 'TextFieldChangedEvent' ? `${type} → "${clipText(text ?? '', 32)}"` : type, type),
    [add],
  )
  const clearLog = useCallback(() => setLog(l => ({ entries: [], total: l.total })), [])

  const [focused, setFocused] = useState<string | null>(null)

  /** `focus()` on a field, once the render that enables it has landed. */
  const focus = useCallback(
    (name: string) => {
      const field = () => {
        const el = viewElement(rootRef.current, name)
        return el?.matches('input, textarea') ? (el as HTMLInputElement) : el?.querySelector<HTMLInputElement>('input, textarea')
      }
      const now = field()
      if (now && !now.disabled) now.focus()
      // A field the same change enables can take the focus only once that render lands.
      else window.setTimeout(() => field()?.focus())
    },
    [rootRef],
  )

  // --- Header ----------------------------------------------------------------
  const [accentIndex, setAccentIndex] = useState(0)
  const [avatarSeed, setAvatarSeed] = useState(0)
  const accent = ACCENTS[accentIndex]!

  const nextAccent = () => {
    const next = (accentIndex + 1) % ACCENTS.length
    setAccentIndex(next)
    note('header', `accent → ${ACCENTS[next]}`)
  }
  const newAvatar = () => {
    setAvatarSeed(seed => seed + 1)
    note('header', `avatar #${avatarSeed + 1}`)
  }

  // --- Sign in ---------------------------------------------------------------
  const [signIn, setSignIn] = useState<SignInState>(() => ({
    username: start.signedInAs ?? '',
    password: start.signedInAs ? 'lovelace' : '',
    secure: true,
    status: start.signedInAs
      ? { text: `Welcome, ${start.signedInAs}.`, tone: 'success' }
      : { text: 'Try any name; "admin" is locked.', tone: 'muted' },
    pending: false,
    signedInAs: start.signedInAs ?? null,
  }))
  const pendingTimer = useRef<number | undefined>(undefined)
  const signInRef = useRef(signIn)
  signInRef.current = signIn

  const submitSignIn = () => {
    const user = signIn.username.trim()
    note('signIn', `request for "${user}"`)
    setSignIn(s => ({ ...s, pending: true, status: { text: `Signing in as ${user}…`, tone: 'muted' } }))
    pendingTimer.current = window.setTimeout(() => {
      // The password as it is when the round trip ends, as the example reads it.
      const pass = signInRef.current.password
      const fail = (reason: string) => {
        note('signIn', `refused: ${reason}`)
        setSignIn(s => ({ ...s, pending: false, status: { text: reason, tone: 'danger' } }))
        focus('signIn.password')
      }
      if (user.toLowerCase() === 'admin') fail('The admin account is locked.')
      else if (pass.length < 6) fail('Password needs at least 6 characters.')
      else {
        note('signIn', `signed in as "${user}"`)
        setSignIn(s => ({ ...s, pending: false, signedInAs: user, status: { text: `Welcome, ${user}.`, tone: 'success' } }))
      }
    }, SIGN_IN_DELAY)
  }

  const signInActions = {
    setUsername: (username: string) => setSignIn(s => ({ ...s, username })),
    setPassword: (password: string) => setSignIn(s => ({ ...s, password })),
    usernameSubmitted: () => focus('signIn.password'),
    passwordSubmitted: () => {
      if (canSubmit) submitSignIn()
    },
    toggleReveal: () => {
      setSignIn(s => ({ ...s, secure: !s.secure }))
      focus('signIn.password')
    },
    clear: () => {
      setSignIn(s => ({ ...s, username: '', password: '', status: { text: 'Cleared.', tone: 'muted' } }))
      focus('signIn.username')
    },
    submit: () => {
      if (!signIn.signedInAs) return submitSignIn()
      note('signIn', `signed out "${signIn.signedInAs}"`)
      setSignIn(s => ({ ...s, signedInAs: null, password: '', status: { text: 'Signed out.', tone: 'muted' } }))
      focus('signIn.username')
    },
  }
  const canSubmit =
    !signIn.pending && (signIn.signedInAs !== null || (signIn.username.trim() !== '' && signIn.password !== ''))
  const subtitle = signIn.signedInAs ? `Signed in as ${signIn.signedInAs}` : DEFAULT_SUBTITLE

  // --- Tasks -----------------------------------------------------------------
  const created = useRef(0)
  const [tasks, setTasks] = useState<Task[]>(() =>
    (start.tasks ?? []).slice(0, MAX_TASKS).map(t => ({ id: ++created.current, title: t.title, done: Boolean(t.done) })),
  )
  const [taskInput, setTaskInput] = useState('')

  const taskActions = {
    setInput: setTaskInput,
    add: () => {
      const title = taskInput.trim()
      if (title && tasks.length < MAX_TASKS) {
        setTasks(list => [...list, { id: ++created.current, title, done: false }])
        note('tasks', `added: ${title}`)
      }
      setTaskInput('')
      focus('tasks.input')
    },
    toggle: (task: Task) => {
      note('tasks', `${task.done ? 'reopened' : 'done'}: ${task.title}`)
      setTasks(list => list.map(t => (t.id === task.id ? { ...t, done: !t.done } : t)))
    },
    // removeSubview + insertSubview: the row keeps its state and its listeners.
    moveUp: (task: Task) =>
      setTasks(list => {
        const i = list.findIndex(t => t.id === task.id)
        if (i <= 0) return list
        const next = [...list]
        next.splice(i, 1)
        next.splice(i - 1, 0, task)
        return next
      }),
    remove: (task: Task) => {
      note('tasks', `removed: ${task.title}`)
      setTasks(list => list.filter(t => t.id !== task.id))
    },
    clearDone: () => {
      for (const task of tasks.filter(t => t.done)) note('tasks', `removed: ${task.title}`)
      setTasks(list => list.filter(t => !t.done))
    },
  }

  // --- Layout playground -----------------------------------------------------
  const shuffles = useRef(0)
  const [playground, setPlayground] = useState<PlaygroundState>({
    layout: start.layout ?? 'row',
    alignment: start.alignment ?? 'stretch',
    spacing: 8,
    padding: 8,
    order: BOXES.map(b => b.name),
    hiddenB: false,
  })
  const change = (patch: Partial<PlaygroundState>, line: string) => {
    setPlayground(p => ({ ...p, ...patch }))
    note('playground', line)
  }
  const playgroundActions = {
    toggleLayout: () => {
      const layout = playground.layout === 'row' ? 'column' : 'row'
      change({ layout }, `layout → ${layout}`)
    },
    cycleAlignment: () => {
      const alignment = ALIGNMENTS[(ALIGNMENTS.indexOf(playground.alignment) + 1) % ALIGNMENTS.length]!
      change({ alignment }, `alignment → ${alignment}`)
    },
    cycleSpacing: () => {
      const spacing = (playground.spacing + 8) % 32
      change({ spacing }, `spacing → ${spacing}`)
    },
    cyclePadding: () => {
      const padding = (playground.padding + 8) % 32
      change({ padding }, `padding → ${padding}`)
    },
    toggleB: () => change({ hiddenB: !playground.hiddenB }, `box B ${playground.hiddenB ? 'shown' : 'hidden'}`),
    // clearSubviews(), then addSubview in the new order: all a reorder takes.
    shuffle: () => {
      const order = shuffled(playground.order, 7 + ++shuffles.current * 31)
      change({ order }, `order → ${order.join('')}`)
    },
  }

  // --- Notes -----------------------------------------------------------------
  const [notes, setNotes] = useState(start.notes ?? '')
  const [notesEditable, setNotesEditable] = useState(true)
  const notesActions = {
    setText: setNotes,
    toggleEditable: () => {
      note('notes', notesEditable ? 'read-only' : 'editable')
      setNotesEditable(e => !e)
    },
    // Set from code: unlike typing, no TextFieldChangedEvent.
    upper: () => setNotes(t => t.toUpperCase()),
    timestamp: () => {
      const now = new Date()
      const time = `${String(now.getHours()).padStart(2, '0')}:${String(now.getMinutes()).padStart(2, '0')}`
      setNotes(t => (t ? `${t}\n[${time}] ` : `[${time}] `))
      focus('notes.text')
    },
  }

  // --- Inspector ---------------------------------------------------------------
  const [inspectorOpen, setInspectorOpen] = useState(Boolean(start.inspectorOpen))
  const toggleInspector = () => {
    note('app', inspectorOpen ? 'inspector closed' : 'inspector opened')
    setInspectorOpen(!inspectorOpen)
  }

  /** Views painted yellow, until their timer puts their own colour back. */
  const [flashing, setFlashing] = useState<readonly string[]>([])
  const flashTimers = useRef(new Map<string, number>())
  const flash = (name: string) => {
    window.clearTimeout(flashTimers.current.get(name))
    setFlashing(list => (list.includes(name) ? list : [...list, name]))
    note('inspector', `flashed ${name}`)
    flashTimers.current.set(
      name,
      window.setTimeout(() => {
        flashTimers.current.delete(name)
        setFlashing(list => list.filter(n => n !== name))
      }, FLASH_DURATION),
    )
  }

  useEffect(() => {
    const timers = flashTimers.current
    return () => {
      window.clearTimeout(pendingTimer.current)
      timers.forEach(id => window.clearTimeout(id))
    }
  }, [])

  const events = useMemo(() => ({ emit, focused, setFocused }), [emit, focused])

  return {
    backend,
    events,
    log,
    note,
    clearLog,
    header: { accent, accentIndex, avatarSeed, subtitle, nextAccent, newAvatar },
    signIn: { state: signIn, canSubmit, ...signInActions },
    tasks: { list: tasks, input: taskInput, ...taskActions },
    playground: { state: playground, ...playgroundActions },
    notes: { text: notes, editable: notesEditable, ...notesActions },
    inspector: { open: inspectorOpen, toggle: toggleInspector, flashing, flash },
  }
}

export type Workbench = ReturnType<typeof useWorkbench>
