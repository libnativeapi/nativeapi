/** `DragOperation` (`drag_source.h`): what happens to the data on a drop. */
export type DragOperation = 'none' | 'copy' | 'move' | 'link'

/** Where a drag can end: the example's drop area, the file manager, the editor. */
export type DropZone = 'example' | 'files' | 'editor'

/** Who started a drag: one of the example's two cards, or another app. */
export type DragSourceKind = 'note' | 'text' | 'files' | 'editor'

/** What a drag carries, as a `DropTargetDroppedEvent` would report it. */
export interface DragPayload {
  source: DragSourceKind
  /** Absolute paths, for files. */
  paths: string[]
  text: string | null
  /** What the ghost under the pointer says. */
  label: string
}

/** A drag in progress, with the pointer in the desktop's coordinates. */
export interface DragSession {
  payload: DragPayload
  x: number
  y: number
  /** The zone under the pointer, if it takes this drag. */
  over: DropZone | null
  /** What the zone under the pointer would do with it: `none` where it refuses. */
  operation: DragOperation
}

/** One entry in the file manager. */
export interface FileEntry {
  name: string
  kind: 'folder' | 'document' | 'image' | 'link'
  /** Just arrived from the example: highlighted. */
  fresh?: boolean
}

export type Tab = 'drop' | 'operations'
