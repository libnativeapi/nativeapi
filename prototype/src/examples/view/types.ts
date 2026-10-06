/** The concrete type of a view (`view.h`): the container, or one of its four controls. */
export type ViewKind = 'View' | 'Label' | 'Button' | 'TextField' | 'ImageView'

/** `ViewLayout`, as the bindings name it. */
export type ViewLayoutName = 'absolute' | 'row' | 'column'

/** `ViewAlignment`: where a subview sits on its row's or column's cross axis. */
export type ViewAlignmentName = 'stretch' | 'start' | 'center' | 'end'

/** The five `ViewEvent`s a view emits. */
export type ViewEventType =
  | 'ButtonClickedEvent'
  | 'TextFieldChangedEvent'
  | 'TextFieldSubmittedEvent'
  | 'ViewFocusedEvent'
  | 'ViewBlurredEvent'

export type AccentName = 'Indigo' | 'Teal' | 'Orange' | 'Pink' | 'Green'

/** `Rectangle`: a view's frame, in its parent's coordinates. */
export interface Frame {
  x: number
  y: number
  width: number
  height: number
}

/** One view of the workbench as the inspector reads it back: `subviews`, `frame` and the getters. */
export interface ViewNode {
  name: string
  kind: ViewKind
  frame: Frame
  /** Only for a container with subviews and a row or column layout. */
  layout?: ViewLayoutName
  gap?: number
  flex: number
  hidden: boolean
  disabled: boolean
  focused: boolean
  /** `text` of a Label, Button or TextField; a secure field's is masked. */
  text?: string
  tooltip?: string
  children: ViewNode[]
}

export interface ViewTree {
  root: ViewNode | null
  count: number
  depth: number
}

/** One line of the inspector's EVENTS: a view event, or a note from the example's own logic. */
export interface LogEntry {
  id: number
  time: string
  source: string
  message: string
  /** Set for a view event; a note has none. */
  type?: ViewEventType
  repeats: number
}

export interface Task {
  id: number
  title: string
  done: boolean
}

export type BoxName = 'A' | 'B' | 'C' | 'D'

export interface BoxSpec {
  name: BoxName
  flex: number
  /** How much of the accent the box is washed with, so the four tell apart. */
  shade: number
}

/** Where the workbench starts: the stories' scenes. */
export interface WorkbenchStart {
  /** Opens the View Inspector beside the workbench. */
  inspectorOpen?: boolean
  /** Starts signed in as this user, the password still in its masked field. */
  signedInAs?: string
  tasks?: readonly { title: string; done?: boolean }[]
  layout?: 'row' | 'column'
  alignment?: ViewAlignmentName
  notes?: string
  /** The workbench's height, as if its corner had been dragged. */
  windowHeight?: number
  /** The inspector's tree row picked at start, outlined in the workbench. */
  selected?: string
}
