import {
  ArrowEnter20Regular,
  Open20Regular,
  Options20Regular,
  ReOrderDotsVertical20Regular,
  Timer20Regular,
} from '@fluentui/react-icons'
import type { ReactNode } from 'react'

import { Icon, IconButton, Tooltip } from '@dazzlabs/dazzui'

import { PANELS } from '../data'
import type { DetachSimulation } from '../simulate-detach'
import type { PanelId } from '../types'
import './panel-frame.css'

const ICONS = { inspector: Options20Regular, stopwatch: Timer20Regular }

export interface PanelFrameProps {
  detach: DetachSimulation
  id: PanelId
  children?: ReactNode
}

/**
 * Chrome shared by the panels: the header to grab, which is the grip the
 * example is about, and the line that proves the panel's state is the same
 * wherever it is shown. A floating panel's header takes the info wash.
 */
export function PanelFrame({ detach, id, children }: PanelFrameProps) {
  const floating = detach.places[id].kind === 'floating'
  const state = detach.panel(id)
  const title = PANELS[id].title
  return (
    <div className="panel-frame" data-panel={id} data-floating={floating ? '' : undefined}>
      <div className="panel-frame__header" onPointerDown={event => detach.pressHeader(event, id)}>
        <Icon icon={ReOrderDotsVertical20Regular} className="panel-frame__grip" />
        <Icon icon={ICONS[id]} />
        <span className="panel-frame__title">{title}</span>
        {floating ? (
          <Tooltip label="Dock back">
            <IconButton label="Dock back" size="small" variant="plain" onClick={() => detach.dockAnywhere(id)}>
              <Icon icon={ArrowEnter20Regular} />
            </IconButton>
          </Tooltip>
        ) : (
          <Tooltip label="Open in a window">
            <IconButton label="Open in a window" size="small" variant="plain" onClick={() => detach.float(id)}>
              <Icon icon={Open20Regular} />
            </IconButton>
          </Tooltip>
        )}
      </div>
      {/* Wraps rather than truncates: the move count at the end is the point of the line. */}
      <code className="panel-frame__state">
        State #{state.instance} · created {state.createdAt} · moved between windows {state.moves}×
      </code>
      <div className="panel-frame__body">{children}</div>
    </div>
  )
}
