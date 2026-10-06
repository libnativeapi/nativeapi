import { Accessibility20Regular } from '@fluentui/react-icons'

import { Button, Icon } from '@dazzlabs/dazzui'

import { APP_NAME } from '../data'
import './system-prompt.css'

export interface SystemPromptProps {
  onOpenSettings: () => void
  onDeny: () => void
}

/**
 * The alert macOS raises for `AXIsProcessTrustedWithOptions(prompt: true)`:
 * the system's own, over every window, pointing the user to System Settings.
 */
export function SystemPrompt({ onOpenSettings, onDeny }: SystemPromptProps) {
  return (
    <div className="system-prompt" role="alertdialog" aria-label="Accessibility Access">
      <span className="system-prompt__icon">
        <Icon icon={Accessibility20Regular} size={30} />
      </span>
      <strong>Accessibility Access (Events)</strong>
      <p>
        “{APP_NAME}” would like to control this computer using accessibility features.
      </p>
      <p className="system-prompt__detail">
        Grant access to this application in Privacy & Security settings, located in System Settings.
      </p>
      <div className="system-prompt__buttons">
        <Button size="small" variant="filled" fullWidth onClick={onOpenSettings}>
          Open System Settings
        </Button>
        <Button size="small" variant="normal" fullWidth onClick={onDeny}>
          Deny
        </Button>
      </div>
    </div>
  )
}
