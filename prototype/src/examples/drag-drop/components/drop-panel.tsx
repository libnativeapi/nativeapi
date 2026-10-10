import { ArrowDownload20Regular, Document20Regular, TextDescription20Regular } from '@fluentui/react-icons'

import { Badge, cx, Icon, PreferenceRow, SectionLabel } from '@dazzlabs/dazzui'

import { PanelPreferences } from '../../../components/panel'
import { baseName } from '../data'
import './drop-panel.css'

export interface DropPanelProps {
  hover: { x: number; y: number } | null
  drops: number
  files: readonly string[]
  text: string | null
}

/**
 * The drop target, drawn as the example draws its `DropRegion`: it lights up
 * under a drag it takes, follows the pointer in its own coordinates, and
 * lists what the last drop carried — each file by name and full path, or the
 * text.
 */
export function DropPanel({ hover, drops, files, text }: DropPanelProps) {
  const empty = files.length === 0 && text === null
  return (
    <div className="drop-panel">
      <div className={cx('drop-panel__region', hover && 'drop-panel__region--hovering')} data-drop-zone="example">
        <div className="drop-panel__head">
          <Icon icon={ArrowDownload20Regular} className="drop-panel__icon" />
          <strong>{hover ? 'Release to drop' : 'Drop files or text here'}</strong>
          <Badge size="small" variant={drops > 0 ? 'tinted' : 'outlined'} tint={drops > 0 ? 'primary' : 'neutral'}>
            Drops: {drops}
          </Badge>
        </div>
        <span className="drop-panel__at">{hover ? `At ${hover.x}, ${hover.y}` : 'DropRegion.isSupported → true'}</span>
        <div className="drop-panel__list">
          <SectionLabel>Last drop</SectionLabel>
          {empty ? (
            <p className="drop-panel__empty">{drops === 0 ? 'Nothing dropped yet' : 'The drop was empty'}</p>
          ) : (
            <PanelPreferences dense>
              {files.map(path => (
                <PreferenceRow key={path} icon={<Icon icon={Document20Regular} />} title={baseName(path)} subtitle={path} />
              ))}
              {text !== null && <PreferenceRow icon={<Icon icon={TextDescription20Regular} />} title={`Text: ${text}`} />}
            </PanelPreferences>
          )}
        </div>
      </div>
    </div>
  )
}
