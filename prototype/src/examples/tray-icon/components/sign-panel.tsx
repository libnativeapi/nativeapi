import { Button, SectionLabel } from '@dazzlabs/dazzui'

import { createSign, TEMPLATES } from '../sign-data'
import type { SignStyle } from '../sign-types'
import type { Capabilities, TrayEntry } from '../types'
import type { TrayActions } from '../use-tray'
import { SignArt } from './sign-art'
import { SignEditor } from './sign-editor'
import './sign-panel.css'

const STYLE_LABELS: Record<SignStyle, string> = {
  missing: 'Missing You',
  welcome: 'Welcome',
  travel: 'Travel',
  guide: 'Guide',
}

/** A content-view scene for the currently selected TrayIcon. */
export function SignPanel({ entry, caps, actions }: { entry: TrayEntry; caps: Capabilities; actions: TrayActions }) {
  return <div className="tray-sign-panel">
    {!caps.contentView && <p className="tray-sign-panel__note">Edit and preview signs on this desktop. Custom menu-bar signs are available on macOS.</p>}
    <SectionLabel>Sign style</SectionLabel>
    <div className="tray-sign-panel__templates">
      {TEMPLATES.map(template => <Button key={template.value} size="small" className="tray-sign-panel__template"
        aria-label={template.label} title={template.label}
        variant={entry.sign.style === template.value ? 'tinted' : 'normal'} aria-pressed={entry.sign.style === template.value}
        onClick={() => actions.setSignStyle(template.value)}>
        <SignArt entry={createSign(0, template.value)} size="tile" />
        <span>{STYLE_LABELS[template.value]}</span>
      </Button>)}
    </div>
    <SignEditor key={`${entry.number}:${entry.sign.style}`} entry={entry.sign} actions={{
      content: actions.setSignContent, restore: actions.restoreSign, option: actions.setSignOption,
    }} />
    <div className="tray-sign-panel__actions"><Button size="small" variant="normal" onClick={() => actions.previewSign()}>Enlarge preview</Button></div>
  </div>
}
