import { type RefObject, useLayoutEffect, useRef, useState } from 'react'

import { WindowFrame, type WindowFramePlatform } from '@dazzlabs/dazzui'

import type { Workbench } from '../use-workbench'
import { viewElement } from '../view-tree'
import { AvatarImage } from './avatar-image'
import { NButton, NImage, NLabel, NView } from './native-view'
import { NotesSection, PlaygroundSection, SignInSection, TasksSection } from './workbench-sections'
import './workbench-window.css'

export interface WorkbenchWindowProps {
  platform: WindowFramePlatform
  wb: Workbench
  /** `os · backend backend`, the footer's status line before the count. */
  status: string
  width: number
  height: number
  inactive?: boolean
  rootRef: RefObject<HTMLDivElement | null>
  stageRef: RefObject<HTMLDivElement | null>
  stageSize: { width: number; height: number }
  /** The inspector's selected view, outlined. */
  selected: string | null
  onQuit: () => void
}

/**
 * Native Views Workbench: one column of nested rows and columns, every view
 * in it a platform control — the header with its painted avatar, the four
 * sections in two columns, and the footer. The inspector's selection is
 * outlined over it, and a flashed view goes yellow for a moment.
 */
export function WorkbenchWindow({
  platform,
  wb,
  status,
  width,
  height,
  inactive,
  rootRef,
  stageRef,
  stageSize,
  selected,
  onQuit,
}: WorkbenchWindowProps) {
  const { header } = wb
  return (
    <WindowFrame
      platform={platform}
      title="Native Views Workbench"
      width={width}
      height={height}
      inactive={inactive}
      className="workbench-window"
      onClose={onQuit}
    >
      <NView ref={rootRef} name="root" gap={12} padding={16} fill className="workbench-window__root">
        <NView name="header" gap={10}>
          <NView name="header.row" layout="row" gap={14}>
            <NImage name="header.avatar" size={56} alignment="center" tooltip="Painted in code, encoded as PNG, shown by ImageView">
              <AvatarImage seed={header.avatarSeed} />
            </NImage>
            <NView name="header.text" gap={4} flex={1} alignment="center">
              <NLabel name="header.title" text="Native Views Workbench" tone="accent" size="title" />
              <NLabel name="header.subtitle" text={header.subtitle} tone="muted" />
            </NView>
            <NButton name="header.newAvatar" text="New avatar" onPress={header.newAvatar} />
            <NButton
              name="header.accent"
              text={`Accent: ${header.accent}`}
              tooltip="Repaint everything in the next accent colour"
              onPress={header.nextAccent}
            />
          </NView>
          <NView name="header.bar" className="workbench-window__bar" />
        </NView>
        <NView name="columns" layout="row" gap={12} flex={1}>
          <NView name="columns.left" gap={12} flex={1}>
            <SignInSection signIn={wb.signIn} />
            <TasksSection tasks={wb.tasks} />
            <NView name="columns.left.fill" flex={1} />
          </NView>
          <NView name="columns.right" gap={12} flex={1}>
            <PlaygroundSection playground={wb.playground} stageSize={stageSize} stageRef={stageRef} />
            <NotesSection notes={wb.notes} />
          </NView>
        </NView>
        <NView name="footer" layout="row" gap={8}>
          <NLabel name="footer.status" text={`${status} · ${wb.log.total} events`} tone="muted" size="small" flex={1} alignment="center" />
          <NButton
            name="footer.inspector"
            text={wb.inspector.open ? 'Close inspector' : 'Open inspector'}
            tooltip="A second window, also native views, that watches this one"
            onPress={wb.inspector.toggle}
          />
          <NButton name="footer.quit" text="Quit" onPress={onQuit} />
        </NView>
        <FrameOverlay rootRef={rootRef} selected={selected} flashing={wb.inspector.flashing} />
      </NView>
    </WindowFrame>
  )
}

const rectOf = ({ left, top, width, height }: Box) => ({ left, top, width, height })

interface Box {
  name: string
  left: number
  top: number
  width: number
  height: number
}

/**
 * Over the workbench, outside its view tree: the outline of the view the
 * inspector selected, and the yellow of every flashed one. Measured after
 * each render, so both follow a re-layout.
 */
function FrameOverlay({
  rootRef,
  selected,
  flashing,
}: {
  rootRef: RefObject<HTMLDivElement | null>
  selected: string | null
  flashing: readonly string[]
}) {
  const [boxes, setBoxes] = useState<{ selected: Box | null; flashing: Box[] }>({ selected: null, flashing: [] })
  const last = useRef('')
  useLayoutEffect(() => {
    const root = rootRef.current
    const measure = (name: string): Box | null => {
      const el = viewElement(root, name)
      if (!root || !el) return null
      const r = el.getBoundingClientRect()
      const o = root.getBoundingClientRect()
      if (r.width === 0 && r.height === 0) return null
      return { name, left: r.left - o.left, top: r.top - o.top, width: r.width, height: r.height }
    }
    const next = {
      selected: selected ? measure(selected) : null,
      flashing: flashing.map(measure).filter((b): b is Box => b !== null),
    }
    const key = JSON.stringify(next)
    if (key !== last.current) {
      last.current = key
      setBoxes(next)
    }
  })
  return (
    <div className="workbench-window__overlay" aria-hidden>
      {boxes.flashing.map(box => (
        <span key={box.name} className="workbench-window__flash" style={rectOf(box)} />
      ))}
      {boxes.selected && (
        <span className="workbench-window__outline" style={rectOf(boxes.selected)}>
          <span className="workbench-window__outline-tag">
            {boxes.selected.name} · {Math.round(boxes.selected.width)}×{Math.round(boxes.selected.height)}
          </span>
        </span>
      )}
    </div>
  )
}
