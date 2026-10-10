/** `DialogModality` (`dialog.h`). */
export type Modality = 'none' | 'application' | 'window'

/** `MessageDialogResult` (`message_dialog.h`): None while open, and on every non-extended backend. */
export type DialogResult = 'none' | 'primary' | 'secondary' | 'close'

/**
 * What draws the dialog: NSAlert, a Win32 MessageBox, WinUI 3's ContentDialog
 * (built with NATIVEAPI_ENABLE_WINUI3, the only extended backend), or GTK's
 * message dialog.
 */
export type Backend = 'appkit' | 'win32' | 'winui3' | 'gtk'

/** Everything the composer sets before `open()`. */
export interface DialogConfig {
  title: string
  message: string
  modality: Modality
  /** `setButtons`: empty labels hide buttons. */
  primary: string
  secondary: string
  close: string
  defaultButton: DialogResult
  /** `setParentWindow`: the example's window, or none. */
  parent: boolean
  inputEnabled: boolean
  inputText: string
  /** `setCheckbox`: an empty label hides it. */
  checkboxLabel: string
  checked: boolean
  /** `setProgress`: -2 hidden, -1 indeterminate, 0…1 determinate. */
  progress: number
}

/** The dialog on screen: what it was opened with. */
export interface OpenDialog {
  number: number
  config: DialogConfig
  backend: Backend
  /** A macOS sheet on the example's window, rather than an alert. */
  sheet: boolean
}

/** One `open()` the example made, newest first in the sidebar. */
export interface DialogRun {
  number: number
  title: string
  backend: Backend
  result: DialogResult
  /** The button the user pressed, by its label. */
  pressed: string
  inputText: string
  checked: boolean
}

export type Tab = 'compose' | 'extended' | 'result'

export type PresetId = 'update' | 'delete' | 'signIn'
