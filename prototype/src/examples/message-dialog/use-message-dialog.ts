import { useCallback, useEffect, useRef, useState } from 'react'

import type { WindowFramePlatform } from '@dazzlabs/dazzui'

import { useEventLog } from '../../components/event-bar'
import { BASE_CONFIG, defaultBackendOf, MODALITIES, PRESETS, RESULT_NAMES } from './data'
import type { Backend, DialogConfig, DialogResult, DialogRun, OpenDialog, PresetId } from './types'

export interface MessageDialogOptions {
  preset?: PresetId
  config?: Partial<DialogConfig>
  /** Open the dialog as the example starts. */
  openAtStart?: boolean
  backend?: Backend
}

export interface MessageDialogState {
  backend: Backend
  config: DialogConfig
  open: OpenDialog | null
  /** What the getters return now. */
  result: DialogResult
  /** What the last `open()` returned; null before one, or while a modal one blocks. */
  opened: boolean | null
  runs: DialogRun[]
  /** Bumped when a click on a blocked window is refused, so the dialog flashes. */
  refused: number
}

const quote = (s: string) => `"${s}"`

/** Whether `open()` blocks until the dialog closes: any modal one, and every NSAlert. */
export const blocks = (dialog: Pick<OpenDialog, 'backend' | 'config'>) =>
  dialog.backend === 'appkit' || dialog.config.modality !== 'none'
const modalityName = (config: DialogConfig) => MODALITIES.find(m => m.value === config.modality)!.name

/**
 * A `MessageDialog`, simulated: composed, opened on the desktop as the
 * platform draws it, blocking what its modality blocks, and read back once
 * a button closes it. Only WinUI 3 has the extended controls and a result;
 * everywhere else `getResult` stays None and `isOpen` false.
 */
export function useMessageDialog(platform: WindowFramePlatform, options: MessageDialogOptions = {}) {
  const backendAtStart = options.backend ?? defaultBackendOf(platform)
  const configAtStart: DialogConfig = {
    ...(PRESETS.find(p => p.id === options.preset)?.config ?? BASE_CONFIG),
    ...options.config,
  }
  const { lastEvent, log, event, call, clear } = useEventLog(
    [`MessageDialog("${BASE_CONFIG.title}", "${BASE_CONFIG.message}")`, `isExtendedSupported → ${backendAtStart === 'winui3'}`],
    'No dialog opened yet',
  )
  const [state, setState] = useState<MessageDialogState>({
    backend: backendAtStart,
    config: configAtStart,
    open: null,
    result: 'none',
    opened: null,
    runs: [],
    refused: 0,
  })
  const ref = useRef(state)
  ref.current = state
  const closeTimer = useRef<number | undefined>(undefined)
  const number = useRef(0)

  const extended = state.backend === 'winui3'

  const finish = useCallback(
    (result: DialogResult, pressed: string, how: string) => {
      const s = ref.current
      if (!s.open) return
      clearTimeout(closeTimer.current)
      const isExtended = s.open.backend === 'winui3'
      // Only the extended backend records which button closed it.
      const recorded: DialogResult = isExtended ? result : 'none'
      if (blocks(s.open)) call('open() → true · returned once the dialog closed')
      event(`Closed: ${pressed}`, `${how} · getResult → ${RESULT_NAMES[recorded]}`)
      const run: DialogRun = {
        number: s.open.number,
        title: s.open.config.title,
        backend: s.open.backend,
        result: recorded,
        pressed,
        inputText: isExtended ? s.config.inputText : '',
        checked: isExtended ? s.config.checked : false,
      }
      setState(prev => ({
        ...prev,
        open: null,
        result: recorded,
        opened: true,
        runs: [run, ...prev.runs].slice(0, 10),
      }))
    },
    [call, event],
  )

  const open = useCallback(
    (closeAfter?: number) => {
      const s = ref.current
      if (s.open) return
      const c = s.config
      const isExtended = s.backend === 'winui3'
      call(`setTitle(${quote(c.title)}) · setMessage(${quote(c.message)})`)
      call(`setModality(${modalityName(c)})`)
      if (isExtended) {
        call(`setButtons(${quote(c.primary)}, ${quote(c.secondary)}, ${quote(c.close)}) → ${Boolean(c.primary || c.secondary || c.close)}`)
        call(`setDefaultButton(${RESULT_NAMES[c.defaultButton]}) → true`)
        call(`setParentWindow(${c.parent ? 'window #1' : 'null'}) → true`)
        call(`setInputEnabled(${c.inputEnabled}) · setInputText(${quote(c.inputText)})`)
        call(`setCheckbox(${quote(c.checkboxLabel)}, ${c.checked}) · setProgress(${c.progress})`)
      }
      const modal = blocks({ backend: s.backend, config: c })
      call(modal ? 'open() · blocks until the dialog closes' : 'open() → true')
      number.current += 1
      event('Dialog open', `${modal ? 'Modal' : 'Modeless'} dialog #${number.current}`)
      setState(prev => ({
        ...prev,
        open: {
          number: number.current,
          config: c,
          backend: prev.backend,
          sheet: prev.backend === 'appkit' && c.modality === 'window',
        },
        result: 'none',
        opened: modal ? null : true,
      }))
      if (closeAfter) {
        closeTimer.current = window.setTimeout(() => {
          const now = ref.current
          if (!now.open) return
          // A Win32 MessageBox cannot be closed from code.
          if (now.open.backend === 'win32') {
            call('close() → false · a MessageBox cannot be closed from code')
            return
          }
          call('close() → true')
          finish('close', 'close()', 'Closed from code')
        }, closeAfter)
      }
    },
    [call, event, finish],
  )

  const started = useRef(false)
  useEffect(() => {
    if (started.current) return
    started.current = true
    if (options.openAtStart) open()
    // Once, as the example starts.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [])
  useEffect(() => () => clearTimeout(closeTimer.current), [])

  const actions = {
    set: (patch: Partial<DialogConfig>) => {
      setState(prev => ({ ...prev, config: { ...prev.config, ...patch } }))
      const s = ref.current
      // The extended controls update live while the dialog shows.
      if (s.open && s.backend === 'winui3') {
        if (patch.progress !== undefined) call(`setProgress(${patch.progress}) → true · live`)
        if (patch.checked !== undefined || patch.checkboxLabel !== undefined)
          call(`setCheckbox(${quote(patch.checkboxLabel ?? s.config.checkboxLabel)}, ${patch.checked ?? s.config.checked}) → true · live`)
      }
    },
    /** The user types into the open dialog's field, or ticks its checkbox. */
    edit: (patch: Pick<Partial<DialogConfig>, 'inputText' | 'checked'>) =>
      setState(prev => ({ ...prev, config: { ...prev.config, ...patch } })),
    preset: (id: PresetId) => {
      const preset = PRESETS.find(p => p.id === id)!
      setState(prev => ({ ...prev, config: preset.config }))
    },
    setBackend: (backend: Backend) => {
      call(`Built with NATIVEAPI_ENABLE_WINUI3=${backend === 'winui3' ? 'ON' : 'OFF'} · isExtendedSupported → ${backend === 'winui3'}`)
      setState(prev => ({ ...prev, backend }))
    },
    open,
    /** A button in the dialog: extended dialogs record which. */
    press: (result: DialogResult, label: string) => finish(result, label, `Pressed ${quote(label)}`),
    close: () => {
      const s = ref.current
      if (!s.open) {
        call('close() → false · not open')
        return
      }
      if (s.open.backend === 'win32') {
        call('close() → false · a MessageBox cannot be closed from code')
        return
      }
      call('close() → true')
      finish('close', 'close()', 'Closed from code')
    },
    /** A click on a window the dialog blocks: the system refuses it. */
    refuse: () => {
      call('Click refused: a modal dialog is open')
      setState(prev => ({ ...prev, refused: prev.refused + 1 }))
    },
    read: (line: string) => call(line),
    clearLog: clear,
  }

  return { state, actions, extended, lastEvent, log }
}

export type MessageDialogActions = ReturnType<typeof useMessageDialog>['actions']
