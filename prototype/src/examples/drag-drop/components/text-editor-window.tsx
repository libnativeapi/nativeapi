import type { PointerEvent } from 'react'

import { cx, WindowFrame, type WindowFramePlatform } from '@dazzlabs/dazzui'

import { EDITOR_AFTER, EDITOR_BEFORE, EDITOR_SELECTION, editorTitle } from '../data'
import type { DragPayload } from '../types'
import './text-editor-window.css'

export interface TextEditorWindowProps {
  platform: WindowFramePlatform
  /** Text dropped into the editor, in order; the last one is highlighted. */
  inserted: readonly string[]
  targeted: boolean
  inactive?: boolean
  onPress: (payload: DragPayload) => (event: PointerEvent) => void
}

/**
 * A plain-text editor with a sentence selected: drag the selection into the
 * example to drop text, or drag the example's text card here to insert it.
 */
export function TextEditorWindow({ platform, inserted, targeted, inactive, onPress }: TextEditorWindowProps) {
  return (
    <WindowFrame platform={platform} title={editorTitle(platform)} width={420} height={250} inactive={inactive}>
      <div className={cx('text-editor', targeted && 'text-editor--targeted')} data-drop-zone="editor">
        <p className="text-editor__text">
          {EDITOR_BEFORE}
          <mark
            className="text-editor__selection"
            title="Drag the selection"
            onPointerDown={onPress({ source: 'editor', paths: [], text: EDITOR_SELECTION, label: EDITOR_SELECTION })}
          >
            {EDITOR_SELECTION}
          </mark>
          {EDITOR_AFTER}
          {inserted.map((text, i) => (
            <span key={i} className={cx('text-editor__inserted', i === inserted.length - 1 && 'text-editor__inserted--last')}>
              {'\n'}
              {text}
            </span>
          ))}
          {targeted && <span className="text-editor__caret" />}
        </p>
      </div>
    </WindowFrame>
  )
}
