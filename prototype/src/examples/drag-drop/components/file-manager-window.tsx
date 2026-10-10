import {
  ArrowLeft16Regular,
  Document20Regular,
  DocumentText20Regular,
  Folder20Filled,
  Image20Regular,
  Link20Regular,
} from '@fluentui/react-icons'
import { type PointerEvent, useState } from 'react'

import { cx, Icon, WindowFrame, type WindowFramePlatform } from '@dazzlabs/dazzui'

import type { Os } from '../../../components/platform'
import { fileManagerTitle, NOTE_NAME, pathIn } from '../data'
import type { DragPayload, FileEntry } from '../types'
import './file-manager-window.css'

export interface FileManagerWindowProps {
  platform: WindowFramePlatform
  os: Os
  folder: string
  files: readonly FileEntry[]
  /** A drag is over the window and the window takes it. */
  targeted: boolean
  inactive?: boolean
  onPress: (payload: DragPayload) => (event: PointerEvent) => void
}

const ICONS = { folder: Folder20Filled, document: Document20Regular, image: Image20Regular, link: Link20Regular }

/**
 * The desktop's file manager on the Downloads folder: files to drag into the
 * example (click to select, ⌘ / Ctrl-click for more), and the place the
 * example's note lands when it is dragged out.
 */
export function FileManagerWindow({ platform, os, folder, files, targeted, inactive, onPress }: FileManagerWindowProps) {
  const [selected, setSelected] = useState<string[]>(['report.pdf'])

  /** A press selects (⌘ / Ctrl adds or removes), then carries the selection if it turns into a drag. */
  const press = (name: string) => (event: PointerEvent) => {
    if (event.button !== 0) return
    const add = event.metaKey || event.ctrlKey
    const next = add
      ? selected.includes(name)
        ? selected.filter(n => n !== name)
        : [...selected, name]
      : selected.includes(name)
        ? selected
        : [name]
    setSelected(next)
    if (!next.includes(name)) return
    const names = files.filter(f => next.includes(f.name)).map(f => f.name)
    onPress({
      source: 'files',
      paths: names.map(n => pathIn(os, folder, n)),
      text: null,
      label: names.length === 1 ? names[0]! : `${names.length} items`,
    })(event)
  }

  return (
    <WindowFrame platform={platform} title={fileManagerTitle(platform)} width={420} height={300} inactive={inactive}>
      <div className={cx('file-manager', targeted && 'file-manager--targeted')} data-drop-zone="files">
        <div className="file-manager__path">
          <Icon icon={ArrowLeft16Regular} size={14} />
          <span>{folder}</span>
        </div>
        <ul className="file-manager__list" role="listbox" aria-multiselectable aria-label="Downloads">
          {files.map(file => (
            <li
              key={file.name}
              role="option"
              aria-selected={selected.includes(file.name)}
              className={cx(
                'file-manager__file',
                selected.includes(file.name) && 'file-manager__file--selected',
                file.fresh && 'file-manager__file--fresh',
              )}
              onPointerDown={press(file.name)}
            >
              <Icon
                icon={file.name === NOTE_NAME ? DocumentText20Regular : ICONS[file.kind]}
                className={`file-manager__icon file-manager__icon--${file.kind}`}
              />
              <span>{file.name}</span>
            </li>
          ))}
        </ul>
        <div className="file-manager__status">
          {files.length} items{selected.length > 0 && ` · ${selected.length} selected`}
        </div>
      </div>
    </WindowFrame>
  )
}
