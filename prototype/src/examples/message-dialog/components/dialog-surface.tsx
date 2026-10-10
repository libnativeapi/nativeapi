import { Info20Filled, Rocket20Regular } from '@fluentui/react-icons'

import { Button, Checkbox, cx, Icon, Progress, TextField, WindowFrame, type WindowFramePlatform } from '@dazzlabs/dazzui'

import type { DialogConfig, DialogResult, OpenDialog } from '../types'
import './dialog-surface.css'

export interface DialogSurfaceProps {
  platform: WindowFramePlatform
  dialog: OpenDialog
  /** The extended controls as they are now: WinUI 3 updates them live. */
  live: DialogConfig
  /** Bumped when a blocked window was clicked: the dialog flashes for attention. */
  refused: number
  onPress: (result: DialogResult, label: string) => void
  onEdit: (patch: Pick<Partial<DialogConfig>, 'inputText' | 'checked'>) => void
}

/** The app's icon, as NSAlert shows it. */
function AppIcon() {
  return (
    <span className="dialog-surface__app-icon">
      <Icon icon={Rocket20Regular} size={26} />
    </span>
  )
}

/**
 * The dialog `open()` put up, drawn by its backend: an NSAlert (or its sheet),
 * a Win32 MessageBox, a WinUI 3 ContentDialog with the extended controls, or
 * GTK's message dialog. Every backend but WinUI 3 has one OK button.
 */
export function DialogSurface({ platform, dialog, live, refused, onPress, onEdit }: DialogSurfaceProps) {
  const { config } = dialog
  const flash = { 'data-refused': refused > 0 ? '' : undefined }

  if (dialog.backend === 'appkit') {
    return (
      <div key={refused} {...flash} className={cx('dialog-surface__mac', dialog.sheet && 'dialog-surface__mac--sheet')} role="alertdialog">
        <AppIcon />
        <strong>{config.title}</strong>
        <p>{config.message}</p>
        <Button size="small" variant="filled" fullWidth autoFocus onClick={() => onPress('none', 'OK')}>
          OK
        </Button>
      </div>
    )
  }

  if (dialog.backend === 'win32') {
    return (
      <div key={refused} {...flash} className="dialog-surface__frame">
        <WindowFrame platform={platform} title={config.title} width={380} controls={{ close: true, minimize: 'hidden', maximize: 'hidden' }} onClose={() => onPress('none', 'Close box')}>
          <div className="dialog-surface__win32">
            <div className="dialog-surface__win32-body">
              <Icon icon={Info20Filled} size={32} className="dialog-surface__info" />
              <p>{config.message}</p>
            </div>
            <div className="dialog-surface__win32-foot">
              <Button size="small" variant="normal" autoFocus onClick={() => onPress('none', 'OK')}>
                OK
              </Button>
            </div>
          </div>
        </WindowFrame>
      </div>
    )
  }

  if (dialog.backend === 'winui3') {
    const buttons: { result: DialogResult; label: string }[] = [
      { result: 'primary' as const, label: config.primary },
      { result: 'secondary' as const, label: config.secondary },
      { result: 'close' as const, label: config.close },
    ].filter(b => b.label)
    return (
      <div key={refused} {...flash} className="dialog-surface__content-dialog" role="dialog" aria-label={config.title}>
        <div className="dialog-surface__content-body">
          <strong>{config.title}</strong>
          <p>{config.message}</p>
          {live.inputEnabled && (
            <TextField
              size="small"
              autoFocus
              aria-label="Input"
              placeholder="Code"
              value={live.inputText}
              onChange={e => onEdit({ inputText: (e.target as HTMLInputElement).value })}
            />
          )}
          {live.checkboxLabel && (
            <Checkbox size="small" checked={live.checked} onCheckedChange={checked => onEdit({ checked: Boolean(checked) })}>
              {live.checkboxLabel}
            </Checkbox>
          )}
          {live.progress >= -1 && <Progress size="small" value={live.progress === -1 ? undefined : live.progress * 100} />}
        </div>
        <div className="dialog-surface__content-buttons" style={{ gridTemplateColumns: `repeat(${buttons.length}, 1fr)` }}>
          {buttons.map(b => (
            <Button
              key={b.result}
              size="small"
              variant={config.defaultButton === b.result ? 'filled' : 'normal'}
              onClick={() => onPress(b.result, b.label)}
            >
              {b.label}
            </Button>
          ))}
        </div>
      </div>
    )
  }

  // GTK: the message is the dialog's bold primary text; the title is the window's.
  return (
    <div key={refused} {...flash} className="dialog-surface__frame">
      <WindowFrame platform={platform} title={config.title} width={360} controls={false}>
        <div className="dialog-surface__gtk">
          <Icon icon={Info20Filled} size={32} className="dialog-surface__info" />
          <strong>{config.message}</strong>
        </div>
        <div className="dialog-surface__gtk-foot">
          <Button size="small" variant="normal" fullWidth={platform !== 'kde'} autoFocus onClick={() => onPress('none', 'OK')}>
            OK
          </Button>
        </div>
      </WindowFrame>
    </div>
  )
}
