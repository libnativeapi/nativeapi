import {
  createContext,
  type CSSProperties,
  type KeyboardEvent,
  type ReactElement,
  type ReactNode,
  type Ref,
  useContext,
} from 'react'

import { Button, cx, TextField, Tooltip } from '@dazzlabs/dazzui'

import type { ViewAlignmentName, ViewEventType, ViewKind, ViewLayoutName } from '../types'
import { clipText } from '../view-tree'
import './native-view.css'

/**
 * What every native view reports to: the event listener each one registers
 * (the example writes every event to its log before handling it), and which
 * view has the focus.
 */
export interface NativeViewEvents {
  emit: (source: string, type: ViewEventType, text?: string) => void
  focused: string | null
  setFocused: (name: string | null) => void
}

const NativeViewsContext = createContext<NativeViewEvents>({ emit: () => {}, focused: null, setFocused: () => {} })

export const NativeViewsProvider = NativeViewsContext.Provider

/** What every view has (`View`): a name in the example's registry, flex, alignment, visibility, enabled. */
interface ViewProps {
  /** The view's name in the example's registry, as the inspector and the log print it. */
  name: string
  flex?: number
  /** `ViewAlignment` on the parent's cross axis. */
  alignment?: ViewAlignmentName
  hidden?: boolean
  disabled?: boolean
  /** `setTooltip`. */
  tooltip?: string
  className?: string
  style?: CSSProperties
}

const ALIGN_SELF: Record<ViewAlignmentName, CSSProperties['alignSelf']> = {
  stretch: 'stretch',
  start: 'flex-start',
  center: 'center',
  end: 'flex-end',
}

/**
 * The view's getters as data attributes, which the inspector reads back, and
 * its place in its parent's row or column: a flex view shares the leftover
 * room, a fixed one keeps its natural size.
 */
function viewAttributes(kind: ViewKind, props: ViewProps, focused: boolean, text?: string) {
  const { name, flex = 0, alignment = 'stretch', hidden, disabled, tooltip, style } = props
  return {
    'data-view': name,
    'data-kind': kind,
    'data-flex': flex || undefined,
    'data-hidden': hidden ? '' : undefined,
    'data-disabled': disabled ? '' : undefined,
    'data-focused': focused ? '' : undefined,
    'data-text': text === undefined ? undefined : clipText(text),
    'data-tooltip': tooltip,
    style: {
      ...style,
      flex: flex > 0 ? `${flex} 1 0px` : '0 0 auto',
      alignSelf: ALIGN_SELF[alignment],
      display: hidden ? 'none' : undefined,
    } satisfies CSSProperties,
  }
}

function withTooltip(tooltip: string | undefined, element: ReactElement<Record<string, unknown>>) {
  return tooltip ? (
    <Tooltip label={tooltip} delay={400}>
      {element}
    </Tooltip>
  ) : (
    element
  )
}

export interface NViewProps extends ViewProps {
  layout?: ViewLayoutName
  /** `setSpacing`: between neighbours. */
  gap?: number
  /** `setPadding`, the same on every edge. */
  padding?: number | [number, number]
  ref?: Ref<HTMLDivElement>
  /** A window's content view: it fills the content area, which the window sizes, not a flex. */
  fill?: boolean
  children?: ReactNode
}

/** `View`: a container whose `layout` stacks its subviews in a row or a column. */
export function NView({ layout = 'column', gap = 0, padding = 0, ref, fill, children, ...props }: NViewProps) {
  const { focused } = useContext(NativeViewsContext)
  const attributes = viewAttributes('View', props, focused === props.name)
  const [block, inline] = Array.isArray(padding) ? padding : [padding, padding]
  return (
    <div
      {...attributes}
      ref={ref}
      data-layout={layout}
      data-gap={gap || undefined}
      className={cx('native-view', `native-view--${layout}`, props.className)}
      style={{ ...attributes.style, ...(fill && { flex: '1 1 0px' }), gap, padding: `${block}px ${inline}px` }}
    >
      {children}
    </div>
  )
}

export type NLabelTone = 'default' | 'muted' | 'accent' | 'success' | 'danger' | 'warning' | 'white'

export interface NLabelProps extends ViewProps {
  text: string
  /** `setTextColor`, from the example's palette. */
  tone?: NLabelTone
  /** `setFontSize`: 22 for the title, 11 for headings and read-outs. */
  size?: 'title' | 'body' | 'small'
  /** `setTextAlignment`. */
  textAlignment?: 'start' | 'center' | 'end'
  mono?: boolean
}

/** `Label`: text, its colour, size and alignment. */
export function NLabel({ text, tone = 'default', size = 'body', textAlignment = 'start', mono, ...props }: NLabelProps) {
  const attributes = viewAttributes('Label', props, false, text)
  const label = (
    <span
      {...attributes}
      className={cx('native-label', props.className)}
      data-tone={tone}
      data-size={size}
      data-mono={mono ? '' : undefined}
      style={{ ...attributes.style, textAlign: textAlignment === 'start' ? 'start' : textAlignment }}
    >
      {text}
    </span>
  )
  return withTooltip(props.tooltip, label)
}

export interface NButtonProps extends ViewProps {
  text: string
  /** Runs after the `ButtonClickedEvent` is logged. */
  onPress?: () => void
}

/**
 * `Button`: a push button with a text and nothing else, as the API has it —
 * so every button looks alike, the platform's own push button.
 */
export function NButton({ text, onPress, alignment = 'center', ...props }: NButtonProps) {
  const { emit } = useContext(NativeViewsContext)
  const attributes = viewAttributes('Button', { ...props, alignment }, false, text)
  const button = (
    <Button
      {...attributes}
      size="small"
      variant="normal"
      disabled={props.disabled}
      className={cx('native-button', props.className)}
      // A native push button takes the click without taking the focus from a field.
      onMouseDown={event => event.preventDefault()}
      onClick={() => {
        emit(props.name, 'ButtonClickedEvent')
        onPress?.()
      }}
    >
      {text}
    </Button>
  )
  return withTooltip(props.tooltip, button)
}

export interface NFieldProps extends ViewProps {
  text: string
  placeholder?: string
  /** `setSecure`: the text is masked, and the inspector masks it too. */
  secure?: boolean
  multiline?: boolean
  /** `setEditable(false)`: read-only, still selectable. */
  editable?: boolean
  rows?: number
  /** Typing only: setting the text from code emits no `TextFieldChangedEvent`. */
  onChanged?: (text: string) => void
  /** Enter in a single-line field, after the `TextFieldSubmittedEvent`. */
  onSubmitted?: () => void
}

/** `TextField`: single-line, secure or multi-line, editable or read-only. */
export function NField({
  text,
  placeholder,
  secure,
  multiline,
  editable = true,
  rows = 3,
  onChanged,
  onSubmitted,
  ...props
}: NFieldProps) {
  const { emit, focused, setFocused } = useContext(NativeViewsContext)
  const shown = secure && text ? '••••' : text
  const attributes = viewAttributes('TextField', props, focused === props.name, shown)
  const common = {
    ...attributes,
    size: 'small' as const,
    value: text,
    placeholder,
    disabled: props.disabled,
    readOnly: !editable,
    'aria-label': placeholder ?? props.name,
    className: cx('native-field', props.className),
    onFocus: () => {
      setFocused(props.name)
      emit(props.name, 'ViewFocusedEvent')
    },
    onBlur: () => {
      setFocused(null)
      emit(props.name, 'ViewBlurredEvent')
    },
  }
  const change = (value: string) => {
    emit(props.name, 'TextFieldChangedEvent', value)
    onChanged?.(value)
  }
  const field = multiline ? (
    <TextField {...common} multiline rows={rows} onChange={event => change(event.target.value)} />
  ) : (
    <TextField
      {...common}
      type={secure ? 'password' : 'text'}
      onChange={event => change((event.target as HTMLInputElement).value)}
      onKeyDown={(event: KeyboardEvent) => {
        if (event.key !== 'Enter') return
        event.preventDefault()
        emit(props.name, 'TextFieldSubmittedEvent')
        onSubmitted?.()
      }}
    />
  )
  return withTooltip(props.tooltip, field)
}

export interface NImageProps extends ViewProps {
  /** The image's own drawing; the view only places it. */
  children: ReactNode
  size: number
}

/** `ImageView`: shows an `Image`, here one the example painted. */
export function NImage({ children, size, ...props }: NImageProps) {
  const attributes = viewAttributes('ImageView', props, false)
  const image = (
    <div
      {...attributes}
      className={cx('native-image', props.className)}
      style={{ ...attributes.style, width: size, height: size }}
    >
      {children}
    </div>
  )
  return withTooltip(props.tooltip, image)
}
