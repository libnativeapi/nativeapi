import type { WindowFramePlatform } from '@dazzlabs/dazzui'

import type { Backend, DialogConfig, DialogResult, Modality, PresetId } from './types'

/** What `MessageDialog("Update Available", …)` starts with, as the example creates it. */
export const BASE_CONFIG: DialogConfig = {
  title: 'Update Available',
  message: 'A new version is available. Would you like to update?',
  modality: 'application',
  primary: 'Update',
  secondary: 'Later',
  close: 'Cancel',
  defaultButton: 'primary',
  parent: true,
  inputEnabled: false,
  inputText: '',
  checkboxLabel: '',
  checked: false,
  progress: -2,
}

/** The presets: each fills every field of the composer. */
export const PRESETS: readonly { id: PresetId; label: string; note: string; config: DialogConfig }[] = [
  { id: 'update', label: 'Update Available', note: 'Three buttons', config: BASE_CONFIG },
  {
    id: 'delete',
    label: 'Delete file?',
    note: 'A checkbox, Cancel by default',
    config: {
      ...BASE_CONFIG,
      title: 'Delete “report.pdf”?',
      message: 'This item will be deleted immediately. You can’t undo this action.',
      modality: 'window',
      primary: 'Delete',
      secondary: '',
      close: 'Cancel',
      defaultButton: 'close',
      checkboxLabel: 'Don’t ask again',
    },
  },
  {
    id: 'signIn',
    label: 'Sign in',
    note: 'A text field and progress',
    config: {
      ...BASE_CONFIG,
      title: 'Sign in',
      message: 'Enter the access code we sent to your email.',
      primary: 'Sign in',
      secondary: '',
      close: 'Cancel',
      inputEnabled: true,
      inputText: '',
      progress: -1,
    },
  },
]

export const MODALITIES: readonly { value: Modality; label: string; name: string }[] = [
  { value: 'none', label: 'None', name: 'DialogModality::None' },
  { value: 'application', label: 'Application', name: 'DialogModality::Application' },
  { value: 'window', label: 'Window', name: 'DialogModality::Window' },
]

export const RESULT_NAMES: Record<DialogResult, string> = {
  none: 'MessageDialogResult::None',
  primary: 'MessageDialogResult::Primary',
  secondary: 'MessageDialogResult::Secondary',
  close: 'MessageDialogResult::Close',
}

export const BACKEND_NAMES: Record<Backend, string> = {
  appkit: 'NSAlert',
  win32: 'Win32 MessageBox',
  winui3: 'WinUI 3 ContentDialog',
  gtk: 'GtkMessageDialog',
}

/** The backend a platform draws with; Windows has two, chosen when core is built. */
export function defaultBackendOf(platform: WindowFramePlatform): Backend {
  if (platform === 'macos' || platform === 'macos15') return 'appkit'
  if (platform === 'windows') return 'winui3'
  return 'gtk'
}

/** How each backend treats each modality. */
export const MODALITY_NOTES: Record<Backend, Record<Modality, string>> = {
  appkit: {
    none: 'NSAlert has no modeless mode: it still runs modal',
    application: 'An app-modal alert: every window waits',
    window: 'A sheet on the example’s window',
  },
  win32: {
    none: 'A MessageBox with no owner: the window stays usable',
    application: 'MB_APPLMODAL: the app’s windows wait',
    window: 'MB_SYSTEMMODAL stands in: no owner window',
  },
  winui3: {
    none: 'Opens and returns: live updates while it shows',
    application: 'Every window of the app is disabled',
    window: 'Covers the parent window only',
  },
  gtk: {
    none: 'gtk_widget_show: the window stays usable',
    application: 'gtk_dialog_run: the app waits',
    window: 'Modal like Application: GTK gets no parent',
  },
}
