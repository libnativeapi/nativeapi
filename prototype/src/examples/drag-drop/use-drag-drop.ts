import { type PointerEvent as ReactPointerEvent, useEffect, useRef, useState } from 'react'

import type { WindowFramePlatform } from '@dazzlabs/dazzui'

import { useEventLog } from '../../components/event-bar'
import { osOf } from '../../components/platform'
import { baseName, DOWNLOADS, DRAG_TEXT, FILES, linkName, NOTE_NAME, NOTE_PATH, pathIn } from './data'
import type { DragOperation, DragPayload, DragSession, DropZone, FileEntry } from './types'

export interface DragDropOptions {
  /** Start after a drop from the file manager: two files in the list. */
  afterDrop?: boolean
  /** Start with a file from the file manager held over the drop area. */
  hovering?: boolean
}

/** How far the pointer goes before a press becomes a drag, as the platforms' thresholds. */
const THRESHOLD = 4
/** The example's window, as `getWindowId()` reads it. */
const WINDOW_ID = 1

const fromExample = (p: DragPayload) => p.source === 'note' || p.source === 'text'

const quote = (s: string) => `"${s.length > 28 ? `${s.slice(0, 27)}…` : s}"`

/**
 * Drag and drop between the example and the desktop's other apps,
 * simulated with real pointer drags: the example's `DropTarget` (through
 * its `DropRegion`) reports drags that enter, move over, leave and drop on
 * its area; its `DragSource`s (the two cards) report how each drag they
 * start ends. A drop only happens when the source's operation and the
 * target's agree; the file manager takes files, the editor takes text.
 */
export function useDragDrop(platform: WindowFramePlatform, options: DragDropOptions = {}) {
  const os = osOf(platform)
  const wayland = platform === 'omarchy'
  const notePath = NOTE_PATH[os]
  const folder = DOWNLOADS[os]
  const log = useEventLog(
    [
      `File("${notePath}").writeAsStringSync(…)`,
      'DropTarget.isSupported() → true',
      'DragSource.isSupported() → true',
      `DropTarget(window ${WINDOW_ID}).addListener(…) → isActive true`,
    ],
    'Nothing dragged yet',
  )
  const { event, call } = log

  const [dropOp, setDropOpState] = useState<Exclude<DragOperation, 'none'>>('copy')
  const [dragOp, setDragOpState] = useState<Exclude<DragOperation, 'none'>>('copy')
  const [hover, setHover] = useState<{ x: number; y: number } | null>(null)
  const [drops, setDrops] = useState(options.afterDrop ? 1 : 0)
  const [droppedFiles, setDroppedFiles] = useState<string[]>(() =>
    options.afterDrop ? ['report.pdf', 'photo.jpg'].map(name => pathIn(os, folder, name)) : [],
  )
  const [droppedText, setDroppedText] = useState<string | null>(null)
  const [lastDrag, setLastDrag] = useState<DragOperation | null>(null)
  const [files, setFiles] = useState<FileEntry[]>(() => [...FILES])
  const [inserted, setInserted] = useState<string[]>([])
  const [session, setSession] = useState<DragSession | null>(null)
  const [dragging, setDragging] = useState(false)
  /** The desktop layer the ghost stands in; the view puts it in the stage's overlay. */
  const layer = useRef<HTMLDivElement>(null)
  const ops = useRef({ dropOp, dragOp })
  ops.current = { dropOp, dragOp }

  const regionOf = () =>
    layer.current?.closest('.desktop-stage')?.querySelector<HTMLElement>('[data-drop-zone="example"]') ?? null

  /** Whether `zone` takes the drag, and with what operation. */
  const verdict = (zone: DropZone | null, payload: DragPayload): DragOperation => {
    const { dropOp: target, dragOp: offered } = ops.current
    if (zone === 'example') return !fromExample(payload) ? target : offered === target ? target : 'none'
    if (zone === 'files') return payload.paths.length > 0 && payload.source !== 'files' ? (fromExample(payload) ? offered : 'copy') : 'none'
    if (zone === 'editor') {
      if (payload.text === null || payload.source === 'editor') return 'none'
      return fromExample(payload) && offered === 'link' ? 'none' : fromExample(payload) ? offered : 'copy'
    }
    return 'none'
  }

  const zoneAt = (x: number, y: number): DropZone | null => {
    for (const el of document.elementsFromPoint(x, y)) {
      const zone = el.closest<HTMLElement>('[data-drop-zone]')?.dataset.dropZone
      if (zone) return zone as DropZone
    }
    return null
  }

  const localIn = (el: HTMLElement | null, x: number, y: number) => {
    const r = el?.getBoundingClientRect()
    return r ? { x: Math.round(x - r.left), y: Math.round(y - r.top) } : { x: 0, y: 0 }
  }

  /** Where the drag ends on the screen, as `DragSourceEvent.getPosition()` reads it. */
  const screenAt = (x: number, y: number) => {
    if (wayland) return '0, 0'
    const r = layer.current?.closest('.desktop-stage')?.getBoundingClientRect()
    return r ? `${Math.round(x - r.left)}, ${Math.round(y - r.top)}` : '0, 0'
  }

  /** A held drag over the region: the Hovering story, or one interrupted. */
  const over = useRef(false)
  const moved = useRef(0)

  const flushMoves = () => {
    if (moved.current > 0) call(`DropTargetMovedEvent ×${moved.current}`)
    moved.current = 0
  }

  /** The pointer is at (x, y) with `payload` in hand: the zones under it hear about it. */
  const track = (payload: DragPayload, x: number, y: number) => {
    const zone = zoneAt(x, y)
    const op = verdict(zone, payload)
    const inside = zone === 'example' && op !== 'none'
    const at = localIn(regionOf(), x, y)
    if (inside && !over.current) {
      over.current = true
      event('Drag entered the drop area', `DropTargetEnteredEvent → ${at.x}, ${at.y}`)
    } else if (!inside && over.current) {
      over.current = false
      flushMoves()
      event('Drag left the drop area', `DropTargetExitedEvent → ${at.x}, ${at.y}`)
    } else if (inside) {
      moved.current += 1
    }
    setHover(inside ? at : null)
    const l = layer.current?.getBoundingClientRect()
    setSession({ payload, x: x - (l?.left ?? 0), y: y - (l?.top ?? 0), over: zone, operation: op })
  }

  /** The button came up at (x, y): whoever is under it takes the data, or nobody does. */
  const finish = (payload: DragPayload, x: number, y: number) => {
    const zone = zoneAt(x, y)
    const op = verdict(zone, payload)
    const at = localIn(regionOf(), x, y)
    setSession(null)
    setDragging(false)
    setHover(null)
    if (zone === 'example' && op !== 'none') {
      over.current = false
      flushMoves()
      setDrops(n => n + 1)
      setDroppedFiles(payload.paths)
      setDroppedText(payload.text)
      const what = payload.paths.length
        ? `${payload.paths.length} file${payload.paths.length === 1 ? '' : 's'}`
        : `text ${quote(payload.text ?? '')}`
      event(`Dropped ${payload.paths.length ? payload.paths.map(baseName).join(', ') : 'text'}`, `DropTargetDroppedEvent → ${at.x}, ${at.y} · ${what}`)
    } else if (over.current) {
      over.current = false
      flushMoves()
      event('Drag left the drop area', `DropTargetExitedEvent → ${at.x}, ${at.y}`)
    }
    if (zone === 'files' && op !== 'none') {
      const name = op === 'link' ? linkName(os) : NOTE_NAME
      setFiles(list => [...list.filter(f => f.name !== name).map(f => ({ ...f, fresh: false })), { name, kind: op === 'link' ? 'link' : 'document', fresh: true }])
    }
    if (zone === 'editor' && op !== 'none' && payload.text) setInserted(list => [...list, payload.text!])
    if (fromExample(payload)) {
      setLastDrag(op)
      const where = zone === 'files' ? ' · the file manager' : zone === 'editor' ? ' · the editor' : zone === 'example' ? ' · this window' : ''
      event(op === 'none' ? 'Drag not taken' : `Drag taken: ${op}`, `DragSourceEndedEvent → ${op} at ${screenAt(x, y)}${where}`)
    }
  }

  /** A press on something draggable: past the threshold it becomes a drag, until the button comes up. */
  const press = (payload: DragPayload) => (down: ReactPointerEvent) => {
    if (down.button !== 0) return
    down.preventDefault()
    const sx = down.clientX
    const sy = down.clientY
    let started = false
    const move = (e: PointerEvent) => {
      if (!started) {
        if (Math.hypot(e.clientX - sx, e.clientY - sy) < THRESHOLD) return
        started = true
        setDragging(true)
        if (fromExample(payload)) {
          call(payload.paths.length ? `setFilePaths(["${payload.paths[0]}"])` : `setText("${payload.text}")`)
          call(`setDragOperation(DragOperation.${ops.current.dragOp})`)
          call(`startDragging(window ${WINDOW_ID}) → true`)
        }
      }
      track(payload, e.clientX, e.clientY)
    }
    const up = (e: PointerEvent) => {
      window.removeEventListener('pointermove', move)
      window.removeEventListener('pointerup', up)
      window.removeEventListener('pointercancel', up)
      if (started) finish(payload, e.clientX, e.clientY)
    }
    window.addEventListener('pointermove', move)
    window.addEventListener('pointerup', up)
    window.addEventListener('pointercancel', up)
  }

  // The Hovering story: a file from the file manager, held over the drop area's middle.
  useEffect(() => {
    if (!options.hovering) return
    const id = requestAnimationFrame(() => {
      const r = regionOf()?.getBoundingClientRect()
      if (!r) return
      const payload: DragPayload = { source: 'files', paths: [pathIn(os, folder, 'notes.md')], text: null, label: 'notes.md' }
      track(payload, r.left + r.width * 0.55, r.top + r.height * 0.45)
      // A press anywhere lets go of it outside: the drag is cancelled.
      const cancel = () => {
        window.removeEventListener('pointerdown', cancel, true)
        setSession(null)
        setHover(null)
        over.current = false
        flushMoves()
        event('Drag left the drop area', 'DropTargetExitedEvent → cancelled')
      }
      window.addEventListener('pointerdown', cancel, true)
    })
    return () => cancelAnimationFrame(id)
    // Once, as the story starts.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [])

  const setDropOp = (value: Exclude<DragOperation, 'none'>) => {
    setDropOpState(value)
    call(`setDropOperation(DragOperation.${value}) → getDropOperation ${value}`)
  }
  const setDragOp = (value: Exclude<DragOperation, 'none'>) => {
    setDragOpState(value)
    call(`setDragOperation(DragOperation.${value}) → getDragOperation ${value}`)
  }

  const payloads = {
    note: { source: 'note', paths: [notePath], text: null, label: NOTE_NAME } satisfies DragPayload,
    text: { source: 'text', paths: [], text: DRAG_TEXT, label: DRAG_TEXT } satisfies DragPayload,
  }

  return {
    os,
    wayland,
    notePath,
    folder,
    layer,
    log,
    dropOp,
    dragOp,
    hover,
    drops,
    droppedFiles,
    droppedText,
    lastDrag,
    files,
    inserted,
    session,
    dragging,
    payloads,
    actions: { press, setDropOp, setDragOp },
  }
}
