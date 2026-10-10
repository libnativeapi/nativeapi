import {
  PreferenceGroup,
  PreferenceRow,
  PreferenceSection,
  SegmentedControl,
  Slider,
  Switch,
  TextField,
} from '@dazzlabs/dazzui'

import { Panel, PanelPreferences } from '../../../components/panel'
import { ReadBack } from '../../../components/read-back'
import { BACKEND_NAMES, MODALITIES, MODALITY_NOTES, RESULT_NAMES } from '../data'
import type { Backend, DialogConfig, DialogResult, Modality } from '../types'
import type { MessageDialogActions, MessageDialogState } from '../use-message-dialog'
import './panels.css'

interface PanelProps {
  state: MessageDialogState
  actions: MessageDialogActions
}

/** What every backend has: the title, the message, and how modal it is. */
export function ComposePanel({ state, actions }: PanelProps) {
  const { config } = state
  return (
    <Panel>
      <PanelPreferences>
        <PreferenceSection>
          <PreferenceGroup title="MessageDialog">
            <PreferenceRow title="Title" subtitle="setTitle">
              <TextField
                size="small"
                aria-label="Title"
                className="message-dialog-panel__field"
                value={config.title}
                onChange={e => actions.set({ title: (e.target as HTMLInputElement).value })}
              />
            </PreferenceRow>
            <PreferenceRow title="Message" subtitle="setMessage">
              <TextField
                multiline
                rows={2}
                aria-label="Message"
                className="message-dialog-panel__field"
                value={config.message}
                onChange={e => actions.set({ message: e.target.value })}
              />
            </PreferenceRow>
            <PreferenceRow title="Modality" subtitle={MODALITY_NOTES[state.backend][config.modality]}>
              <SegmentedControl<Modality>
                size="small"
                items={MODALITIES}
                value={config.modality}
                onValueChange={modality => actions.set({ modality })}
              />
            </PreferenceRow>
          </PreferenceGroup>
        </PreferenceSection>
      </PanelPreferences>
      <p className="example-panel__note">
        {state.backend === 'winui3'
          ? 'Application and Window block until a button closes the dialog; None returns at once and the extended controls update live.'
          : `${BACKEND_NAMES[state.backend]} has one OK button. The buttons, the default, the parent, the field, the checkbox and the progress are WinUI 3 only.`}
      </p>
    </Panel>
  )
}

const DEFAULTS: readonly { value: DialogResult; label: string }[] = [
  { value: 'none', label: 'None' },
  { value: 'primary', label: 'Primary' },
  { value: 'secondary', label: 'Secondary' },
  { value: 'close', label: 'Close' },
]

type ProgressMode = 'hidden' | 'busy' | 'value'

/** The extended controls: WinUI 3's ContentDialog only; elsewhere every setter returns false. */
export function ExtendedPanel({ state, actions, extended, windows }: PanelProps & { extended: boolean; windows: boolean }) {
  const { config } = state
  const off = !extended
  const reason = windows ? 'Win32 MessageBox: built without NATIVEAPI_ENABLE_WINUI3' : 'WinUI 3 only: isExtendedSupported → false here'
  const mode: ProgressMode = config.progress === -2 ? 'hidden' : config.progress === -1 ? 'busy' : 'value'
  const text = (key: 'primary' | 'secondary' | 'close' | 'inputText' | 'checkboxLabel', label: string, placeholder = '') => (
    <TextField
      size="small"
      aria-label={label}
      placeholder={placeholder}
      disabled={off}
      className="message-dialog-panel__short"
      value={config[key]}
      onChange={e => actions.set({ [key]: (e.target as HTMLInputElement).value } as Partial<DialogConfig>)}
    />
  )

  return (
    <Panel>
      {off && <p className="message-dialog-panel__off">{reason}</p>}
      <PanelPreferences>
        <PreferenceSection>
          <PreferenceGroup title="Buttons">
            <PreferenceRow title="setButtons" subtitle="Primary, secondary, close: an empty label hides it">
              {text('primary', 'Primary', 'Primary')}
              {text('secondary', 'Secondary', 'Secondary')}
              {text('close', 'Close', 'Close')}
            </PreferenceRow>
            <PreferenceRow title="setDefaultButton" subtitle="Enter presses it">
              <SegmentedControl<DialogResult>
                size="small"
                disabled={off}
                items={DEFAULTS}
                value={config.defaultButton}
                onValueChange={defaultButton => actions.set({ defaultButton })}
              />
            </PreferenceRow>
            <PreferenceRow title="setParentWindow" subtitle="The window it covers and blocks">
              <SegmentedControl
                size="small"
                disabled={off}
                items={[
                  { value: 'none', label: 'None' },
                  { value: 'window', label: 'Window #1' },
                ]}
                value={config.parent ? 'window' : 'none'}
                onValueChange={v => actions.set({ parent: v === 'window' })}
              />
            </PreferenceRow>
          </PreferenceGroup>
          <PreferenceGroup title="Content">
            <PreferenceRow title="setInputEnabled" subtitle="A text field under the message">
              {text('inputText', 'Input text', 'Initial text')}
              <Switch disabled={off} checked={config.inputEnabled} onCheckedChange={inputEnabled => actions.set({ inputEnabled })} />
            </PreferenceRow>
            <PreferenceRow title="setCheckbox" subtitle="An empty label hides it; the value outlives the dialog">
              {text('checkboxLabel', 'Checkbox label', 'Label')}
              <Switch disabled={off} checked={config.checked} onCheckedChange={checked => actions.set({ checked })} />
            </PreferenceRow>
            <PreferenceRow title="setProgress" subtitle="-2 hidden, -1 indeterminate, 0…1 — live while open">
              <SegmentedControl<ProgressMode>
                size="small"
                disabled={off}
                items={[
                  { value: 'hidden', label: 'Hidden' },
                  { value: 'busy', label: 'Busy' },
                  { value: 'value', label: 'Value' },
                ]}
                value={mode}
                onValueChange={next => actions.set({ progress: next === 'hidden' ? -2 : next === 'busy' ? -1 : 0.4 })}
              />
            </PreferenceRow>
            {mode === 'value' && (
              <PreferenceRow title="Fraction">
                <Slider
                  size="small"
                  disabled={off}
                  className="message-dialog-panel__slider"
                  min={0}
                  max={1}
                  step={0.05}
                  value={config.progress}
                  showValue
                  formatValue={v => `${Math.round(v[0]! * 100)}%`}
                  onValueChange={v => actions.set({ progress: v as number })}
                />
              </PreferenceRow>
            )}
          </PreferenceGroup>
        </PreferenceSection>
      </PanelPreferences>
    </Panel>
  )
}

/** What the getters return after the dialog closed, and the backend that answered. */
export function ResultPanel({ state, actions, windows }: PanelProps & { windows: boolean }) {
  const extended = state.backend === 'winui3'
  return (
    <Panel>
      {windows && (
        <PanelPreferences>
          <PreferenceSection>
            <PreferenceRow title="Backend" subtitle="Chosen when core is built: NATIVEAPI_ENABLE_WINUI3">
              <SegmentedControl<Backend>
                size="small"
                disabled={Boolean(state.open)}
                items={[
                  { value: 'winui3', label: 'WinUI 3' },
                  { value: 'win32', label: 'Win32' },
                ]}
                value={state.backend}
                onValueChange={actions.setBackend}
              />
            </PreferenceRow>
          </PreferenceSection>
        </PanelPreferences>
      )}
      <ReadBack
        keyWidth="9rem"
        columns={1}
        rows={[
          ['isExtendedSupported', String(extended)],
          ['getTitle', `"${state.config.title}"`],
          ['getModality', MODALITIES.find(m => m.value === state.config.modality)!.name],
          ['open() returned', state.opened === null ? (state.open ? '— blocking' : '—') : String(state.opened)],
          ['isOpen', String(extended && Boolean(state.open))],
          ['getResult', RESULT_NAMES[state.result]],
          ['getInputText', `"${extended ? state.config.inputText : ''}"`],
          ['isCheckboxChecked', String(extended && state.config.checked)],
        ]}
      />
      <p className="example-panel__note">
        {extended
          ? 'getResult is None while the dialog shows; the input text and the checkbox keep their values after it closes.'
          : 'No extended backend: getResult stays None and isOpen false whichever button closed it.'}
      </p>
    </Panel>
  )
}
