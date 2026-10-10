import { type PointerEvent as ReactPointerEvent, useLayoutEffect, useRef, useState } from 'react'

import { Button, type WindowFramePlatform } from '@dazzlabs/dazzui'

import { DesktopStage, DesktopWindow, usePointerDrag } from '../../components/desktop-stage'
import { osOf } from '../../components/platform'
import { InspectorWindow } from './components/inspector-window'
import { NativeViewsProvider } from './components/native-view'
import { WorkbenchWindow } from './components/workbench-window'
import { FIND_HINT, OS_NAMES, STAGE_HEIGHT } from './data'
import type { ViewTree, WorkbenchStart } from './types'
import { useWorkbench } from './use-workbench'
import { describeViewTree, findView, flattenTree } from './view-tree'
import './view-view.css'

export interface ViewViewProps {
  platform: WindowFramePlatform
  /** The scene the example starts in. */
  start?: WorkbenchStart
}

/** The workbench's size as the example sets it, and the bounds a corner drag keeps it in. */
const SIZE = { width: 880, height: 640 }
const MIN = { width: 800, height: 620 }
const MAX = { width: 1100, height: 720 }
const clamp = (v: number, lo: number, hi: number) => Math.min(hi, Math.max(lo, v))

type Which = 'workbench' | 'inspector'

/** A window's titlebar, outside its buttons: where a press starts a move. */
const onTitlebar = (event: ReactPointerEvent) => {
  const target = event.target as HTMLElement
  return Boolean(target.closest('.dz-window-frame__titlebar')) && !target.closest('button, input, textarea, [role="button"]')
}

/**
 * The view example: a workbench of native views — AppKit, Win32 or GTK 3 —
 * driven from a plain Dart program, and the View Inspector beside it, also
 * native views, reading the workbench's tree back live. Selecting a view in
 * the inspector outlines it in the workbench; dragging the workbench's corner
 * resizes it, and every flexible view and frame follows.
 */
export function ViewView({ platform, start = { inspectorOpen: true } }: ViewViewProps) {
  const os = osOf(platform)
  const [run, setRun] = useState(0)
  const [quit, setQuit] = useState(false)

  if (quit) {
    return (
      <DesktopStage
        platform={platform}
        appName="view_example"
        hint={
          <span className="view-view__quit">
            <span>Application.quit(0): the example has quit.</span>
            <Button
              size="small"
              variant="normal"
              onClick={() => {
                setQuit(false)
                setRun(r => r + 1)
              }}
            >
              Run it again
            </Button>
          </span>
        }
      />
    )
  }
  return <Workbench key={run} platform={platform} os={os} start={start} onQuit={() => setQuit(true)} />
}

function Workbench({
  platform,
  os,
  start,
  onQuit,
}: {
  platform: WindowFramePlatform
  os: ReturnType<typeof osOf>
  start: WorkbenchStart
  onQuit: () => void
}) {
  const rootRef = useRef<HTMLDivElement | null>(null)
  const stageRef = useRef<HTMLDivElement | null>(null)
  const wb = useWorkbench(os, start, rootRef)

  // --- Windows on the desktop: where they stand, which is in front, the workbench's size.
  const [size, setSize] = useState({ width: SIZE.width, height: start.windowHeight ?? SIZE.height })
  const [places, setPlaces] = useState<Record<Which, { x: number; y: number }>>({
    workbench: { x: 16, y: 16 },
    inspector: { x: 16 + SIZE.width + 14, y: 16 },
  })
  const [front, setFront] = useState<Which>('workbench')
  const origin = useRef({ x: 0, y: 0, width: 0, height: 0, which: 'workbench' as Which })

  const move = usePointerDrag({
    onMove: ({ dx, dy }) =>
      setPlaces(p => ({ ...p, [origin.current.which]: { x: origin.current.x + dx, y: Math.max(0, origin.current.y + dy) } })),
  })
  const startMove = (which: Which) => (event: ReactPointerEvent) => {
    if (!onTitlebar(event)) return
    origin.current = { ...origin.current, ...places[which], which }
    move(event)
  }
  // The window manager's resize from the corner: the window's content size
  // changes, and every row and column lays itself out again.
  const resize = usePointerDrag({
    onStart: () => (origin.current = { ...origin.current, ...size }),
    onMove: ({ dx, dy }) =>
      setSize({
        width: clamp(origin.current.width + dx, MIN.width, MAX.width),
        height: clamp(origin.current.height + dy, MIN.height, MAX.height),
      }),
  })

  // --- The playground stage takes what its column leaves, and so follows the window.
  const [stageSize, setStageSize] = useState({ width: 380, height: STAGE_HEIGHT })
  useLayoutEffect(() => {
    const el = stageRef.current
    if (!el) return
    const read = () => setStageSize({ width: el.clientWidth, height: el.clientHeight })
    const observer = new ResizeObserver(read)
    observer.observe(el)
    read()
    return () => observer.disconnect()
  }, [])

  // --- The inspector's tree, read back after every render: events, edits and resizes all change it.
  const [tree, setTree] = useState<ViewTree>({ root: null, count: 0, depth: 0 })
  const treeKey = useRef('')
  useLayoutEffect(() => {
    if (!wb.inspector.open) return
    const next = describeViewTree(rootRef.current)
    const key = JSON.stringify(next)
    if (key !== treeKey.current) {
      treeKey.current = key
      setTree(next)
    }
  })

  const [selected, setSelected] = useState<string | null>(start.selected ?? null)
  const [query, setQuery] = useState('')
  const [found, setFound] = useState(FIND_HINT)
  const find = (text: string) => {
    const match = findView(describeViewTree(rootRef.current), text)
    setFound(match ? `Match: ${match.name}` : text.trim() ? 'No view matches.' : FIND_HINT)
    if (match) setSelected(match.name)
    return match
  }
  // A selection that left the tree (a removed task) is dropped.
  const shownSelection = selected && flattenTree(tree.root).some(n => n.name === selected) ? selected : null

  const status = `${OS_NAMES[os]} · ${wb.backend} backend`

  return (
    <NativeViewsProvider value={wb.events}>
      <DesktopStage
        platform={platform}
        appName="view_example"
        layout="free"
        className={`view-view view-view--${wb.header.accent.toLowerCase()}`}
        onPress={() => setFront('workbench')}
      >
        <DesktopWindow
          x={places.workbench.x}
          y={places.workbench.y}
          z={front === 'workbench' ? 2 : 1}
          onPress={() => setFront('workbench')}
        >
          <div className="view-view__window" onPointerDown={startMove('workbench')}>
            <WorkbenchWindow
              platform={platform}
              wb={wb}
              status={status}
              width={size.width}
              height={size.height}
              inactive={wb.inspector.open && front !== 'workbench'}
              rootRef={rootRef}
              stageRef={stageRef}
              stageSize={stageSize}
              selected={wb.inspector.open ? shownSelection : null}
              onQuit={onQuit}
            />
            <span className="view-view__grip" title="Drag to resize" onPointerDown={resize} />
          </div>
        </DesktopWindow>
        {wb.inspector.open && (
          <DesktopWindow
            x={places.inspector.x}
            y={places.inspector.y}
            z={front === 'inspector' ? 2 : 1}
            onPress={() => setFront('inspector')}
          >
            <div className="view-view__window" onPointerDown={startMove('inspector')}>
              <InspectorWindow
                platform={platform}
                tree={tree}
                entries={wb.log.entries}
                selected={shownSelection}
                onSelect={setSelected}
                query={query}
                found={found}
                onQuery={text => {
                  setQuery(text)
                  find(text)
                }}
                onFlash={() => {
                  const match = find(query)
                  if (match) wb.inspector.flash(match.name)
                }}
                onRefresh={() => {
                  treeKey.current = ''
                  setTree(describeViewTree(rootRef.current))
                }}
                onClearLog={wb.clearLog}
                inactive={front !== 'inspector'}
                onClose={wb.inspector.toggle}
              />
            </div>
          </DesktopWindow>
        )}
      </DesktopStage>
    </NativeViewsProvider>
  )
}
