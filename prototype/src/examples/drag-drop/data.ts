import type { WindowFramePlatform } from '@dazzlabs/dazzui'

import { osOf, type Os } from '../../components/platform'
import type { FileEntry } from './types'

export const NOTE_NAME = 'nativeapi-drag-drop-note.txt'
export const DRAG_TEXT = 'Hello from nativeapi'

/** Where the example writes its note: the system's temporary directory. */
export const NOTE_PATH: Record<Os, string> = {
  macos: `/var/folders/7x/k2m9q1_n4bz/T/${NOTE_NAME}`,
  windows: `C:\\Users\\me\\AppData\\Local\\Temp\\${NOTE_NAME}`,
  linux: `/tmp/${NOTE_NAME}`,
}

/** The folder the file manager shows. */
export const DOWNLOADS: Record<Os, string> = {
  macos: '/Users/me/Downloads',
  windows: 'C:\\Users\\me\\Downloads',
  linux: '/home/me/Downloads',
}

export const pathIn = (os: Os, folder: string, name: string) => `${folder}${os === 'windows' ? '\\' : '/'}${name}`

/** The basename of a path, whichever separator it uses. */
export const baseName = (path: string) => path.split(/[\\/]/).pop() ?? path

/** The file manager each desktop opens: Finder, File Explorer, Files, Dolphin, Nautilus. */
export function fileManagerTitle(platform: WindowFramePlatform) {
  switch (platform) {
    case 'macos':
    case 'macos15':
      return 'Downloads'
    case 'windows':
      return 'Downloads - File Explorer'
    case 'kde':
      return 'Downloads — Dolphin'
    default:
      return 'Downloads — Files'
  }
}

/** The plain-text editor each desktop ships. */
export function editorTitle(platform: WindowFramePlatform) {
  switch (osOf(platform)) {
    case 'macos':
      return 'Notes.txt — TextEdit'
    case 'windows':
      return 'Notes.txt - Notepad'
    default:
      return platform === 'kde' ? 'Notes.txt — KWrite' : 'Notes.txt — Text Editor'
  }
}

/** What an alias of the dragged note is called where a link is dropped. */
export function linkName(os: Os) {
  return os === 'macos' ? `${NOTE_NAME} alias` : os === 'windows' ? `${NOTE_NAME} - Shortcut.lnk` : NOTE_NAME
}

export const FILES: readonly FileEntry[] = [
  { name: 'Screenshots', kind: 'folder' },
  { name: 'report.pdf', kind: 'document' },
  { name: 'photo.jpg', kind: 'image' },
  { name: 'notes.md', kind: 'document' },
]

/** The editor's text; the selected sentence is what can be dragged out of it. */
export const EDITOR_BEFORE = 'Shopping list for the weekend.\n\n'
export const EDITOR_SELECTION = 'Drag these words into the example.'
export const EDITOR_AFTER = '\n\nMilk, bread, coffee.'
