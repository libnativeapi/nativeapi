import { DocumentText20Regular, TextDescription20Regular } from '@fluentui/react-icons'
import { useState } from 'react'

import {
  Badge,
  PreferenceRow,
  PreferenceSection,
  SegmentedControl,
  SidebarGroup,
  SidebarGroupContent,
  SidebarGroupLabel,
  type WindowFramePlatform,
} from '@dazzlabs/dazzui'

import { DesktopStage, DesktopWindow } from '../../components/desktop-stage'
import { EventBar } from '../../components/event-bar'
import { ExampleWindow } from '../../components/example-window'
import { Panel, PanelPreferences } from '../../components/panel'
import { ReadBack } from '../../components/read-back'
import { DragCard } from './components/drag-card'
import { DragGhost } from './components/drag-ghost'
import { DropPanel } from './components/drop-panel'
import { FileManagerWindow } from './components/file-manager-window'
import { TextEditorWindow } from './components/text-editor-window'
import { DRAG_TEXT, NOTE_NAME } from './data'
import type { DragOperation, Tab } from './types'
import { type DragDropOptions, useDragDrop } from './use-drag-drop'
import './drag-drop-view.css'

export interface DragDropViewProps {
  platform: WindowFramePlatform
  options?: DragDropOptions
  initialTab?: Tab
}

type Window = 'example' | 'files' | 'editor'
type Operation = Exclude<DragOperation, 'none'>

const OPERATIONS: readonly { value: Operation; label: string }[] = [
  { value: 'copy', label: 'Copy' },
  { value: 'move', label: 'Move' },
  { value: 'link', label: 'Link' },
]

/**
 * The drag and drop example beside two of the desktop's own apps: a file
 * manager on the Downloads folder and a text editor. Drag files or the
 * editor's selection into the example's drop area, and the note or the text
 * card out of it into them — with real pointer drags. The example logs what
 * its `DropTarget` and `DragSource` report; a drop happens only where the
 * source's operation and the target's agree.
 */
export function DragDropView({ platform, options, initialTab = 'drop' }: DragDropViewProps) {
  const dnd = useDragDrop(platform, options)
  const { log, actions, session } = dnd
  const [tab, setTab] = useState<Tab>(initialTab)
  const [front, setFront] = useState<Window>('example')
  const dragOut = session !== null && (session.payload.source === 'note' || session.payload.source === 'text')
  const z = (w: Window) => (front === w ? 3 : w === 'example' ? 2 : 1)

  const exampleWindow = (
    <ExampleWindow
      platform={platform}
      appName="Drag and drop"
      width={760}
      height={460}
      inactive={front !== 'example'}
      className="drag-drop-view__window"
      title="Drop target"
      toolbar={
        <SegmentedControl<Tab>
          size="small"
          items={[
            { value: 'drop', label: 'Drop' },
            { value: 'operations', label: 'Operations' },
          ]}
          value={tab}
          onValueChange={setTab}
        />
      }
      trailing={
        <Badge size="small" variant="tinted" tint="success">
          Active
        </Badge>
      }
      sidebar={
        <SidebarGroup>
          <SidebarGroupLabel>Drag out</SidebarGroupLabel>
          <SidebarGroupContent className="drag-drop-view__drag-out">
            <p className="drag-drop-view__hint">Into the file manager, the editor, or the drop area.</p>
            <DragCard
              icon={DocumentText20Regular}
              title="Drag this note"
              subtitle={NOTE_NAME}
              dragging={session?.payload.source === 'note'}
              onPointerDown={actions.press(dnd.payloads.note)}
            />
            <DragCard
              icon={TextDescription20Regular}
              title="Drag this text"
              subtitle={DRAG_TEXT}
              dragging={session?.payload.source === 'text'}
              onPointerDown={actions.press(dnd.payloads.text)}
            />
            <div className="drag-drop-view__last">
              <span>Last drag: {dnd.lastDrag ?? '-'}</span>
              {dnd.lastDrag && (
                <Badge size="small" variant="tinted" tint={dnd.lastDrag === 'none' ? 'neutral' : 'success'}>
                  {dnd.lastDrag === 'none' ? 'not taken' : 'taken'}
                </Badge>
              )}
            </div>
          </SidebarGroupContent>
        </SidebarGroup>
      }
      sidebarFooter={<span className="drag-drop-view__supported">isSupported → true</span>}
      footer={<EventBar lastEvent={log.lastEvent} log={log.log} onClear={log.clear} />}
    >
      {tab === 'drop' ? (
        <DropPanel hover={dnd.hover} drops={dnd.drops} files={dnd.droppedFiles} text={dnd.droppedText} />
      ) : (
        <div className="example-window__scroll">
          <Panel>
            <ReadBack
              label="DropTarget · read back"
              keyWidth="8rem"
              rows={[
                ['isSupported', 'true'],
                ['isActive', 'true'],
                ['getWindowId', '1'],
                ['getDropOperation', dnd.dropOp],
              ]}
            />
            <ReadBack
              label="DragSource · read back"
              keyWidth="8rem"
              rows={[
                ['isSupported', 'true'],
                ['isDragging', String(dragOut)],
                ['getDragOperation', dnd.dragOp],
                ['getFilePaths', `["…/${NOTE_NAME}"]`],
              ]}
            />
            <PanelPreferences>
              <PreferenceSection label="Operations">
                <PreferenceRow title="Drop operation" subtitle="setDropOperation: what this window does with a drop">
                  <SegmentedControl<Operation> size="small" items={OPERATIONS} value={dnd.dropOp} onValueChange={actions.setDropOp} />
                </PreferenceRow>
                <PreferenceRow title="Drag operation" subtitle="setDragOperation: what the cards offer">
                  <SegmentedControl<Operation> size="small" items={OPERATIONS} value={dnd.dragOp} onValueChange={actions.setDragOp} />
                </PreferenceRow>
              </PreferenceSection>
            </PanelPreferences>
            <p className="example-panel__note">
              A drop happens only where the two agree: a card dragged onto this window&apos;s own area needs the same
              operation on both sides. Apps offer every operation, so drops from them always land. The file manager
              takes files — a link becomes an {dnd.os === 'windows' ? 'shortcut' : 'alias'} — and the editor takes text
              to copy or move, never to link.
            </p>
          </Panel>
        </div>
      )}
    </ExampleWindow>
  )

  return (
    <DesktopStage
      platform={platform}
      appName="Drag Drop Example"
      layout="free"
      onPress={() => setFront('example')}
      hint="Drag files from the file manager or the editor's selection into the example, and the cards out of it."
      overlay={
        <div ref={dnd.layer} className="drag-drop-view__layer">
          {session && <DragGhost session={session} />}
        </div>
      }
    >
      <DesktopWindow x={32} y={28} z={z('example')} onPress={() => setFront('example')}>
        {exampleWindow}
      </DesktopWindow>
      <DesktopWindow x={828} y={28} z={z('files')} onPress={() => setFront('files')}>
        <FileManagerWindow
          platform={platform}
          os={dnd.os}
          folder={dnd.folder}
          files={dnd.files}
          targeted={session?.over === 'files' && session.operation !== 'none'}
          inactive={front !== 'files'}
          onPress={actions.press}
        />
      </DesktopWindow>
      <DesktopWindow x={828} y={352} z={z('editor')} onPress={() => setFront('editor')}>
        <TextEditorWindow
          platform={platform}
          inserted={dnd.inserted}
          targeted={session?.over === 'editor' && session.operation !== 'none'}
          inactive={front !== 'editor'}
          onPress={actions.press}
        />
      </DesktopWindow>
    </DesktopStage>
  )
}
