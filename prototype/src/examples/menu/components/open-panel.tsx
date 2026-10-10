import { CursorClick24Regular } from '@fluentui/react-icons'
import type { MouseEvent } from 'react'

import { Button, Icon, SectionLabel } from '@dazzlabs/dazzui'

import { Panel } from '../../../components/panel'
import { OPEN_POINTS } from '../data'
import type { MenuItemModel, MenuModel, Placement } from '../types'
import { PlacementPad } from './placement-pad'
import './panels.css'

export interface OpenPanelProps {
  context: MenuModel
  submenu: MenuModel
  submenuItem: MenuItemModel | null
  placement: Placement
  /** The context menu is open now. */
  menuOpen: boolean
  onPlacement: (value: Placement) => void
  /** A right-click on the region, in client coordinates. */
  onRegion: (event: MouseEvent) => void
  onOpenAt: (x: number, y: number, label: string) => void
  onOpenAtCursor: (event: MouseEvent) => void
}

/**
 * The context menu's home: a `ContextMenuRegion` that opens it where it is
 * right-clicked, at the pad's placement; the positioning menu opened at
 * points on the screen, edges included; and what the menu holds right now.
 */
export function OpenPanel({
  context,
  submenu,
  submenuItem,
  placement,
  menuOpen,
  onPlacement,
  onRegion,
  onOpenAt,
  onOpenAtCursor,
}: OpenPanelProps) {
  const checkbox = context.items.find(i => i.label === 'Checkbox Item')
  const radio = context.items.find(i => i.type === 'radio' && i.state === 'checked')

  const readouts: [string, string][] = [
    ['Items', String(context.items.length)],
    ['Checkbox', checkbox ? checkbox.state : 'removed'],
    ['Radio', radio ? radio.label.replace('Radio ', '') : 'none'],
    ['Submenu', !submenuItem ? 'removed' : submenuItem.submenu === null ? 'detached' : `${submenu.items.length} entries`],
  ]

  return (
    <Panel>
      <div className="menu-panel__stage">
        <div
          className="menu-panel__region"
          data-open={menuOpen ? '' : undefined}
          onContextMenu={event => {
            event.preventDefault()
            event.stopPropagation()
            onRegion(event)
          }}
        >
          <Icon icon={CursorClick24Regular} size={32} />
          <strong>Right-click here</strong>
          <span>opens the native context menu · ContextMenuRegion</span>
        </div>
        <PlacementPad value={placement} onChange={onPlacement} />
      </div>
      <div className="menu-panel__readouts">
        {readouts.map(([label, value]) => (
          <div key={label}>
            <span>{label}</span>
            <strong>{value}</strong>
          </div>
        ))}
      </div>
      <div className="menu-panel__section">
        <SectionLabel>Open the positioning menu at</SectionLabel>
        <div className="example-panel__actions">
          {OPEN_POINTS.slice(0, 2).map(p => (
            <Button key={p.label} size="small" variant="normal" onClick={() => onOpenAt(p.x, p.y, p.label)}>
              {p.label}
            </Button>
          ))}
          <Button size="small" variant="normal" onClick={onOpenAtCursor}>
            At cursor
          </Button>
          {OPEN_POINTS.slice(2).map(p => (
            <Button key={p.label} size="small" variant="normal" onClick={() => onOpenAt(p.x, p.y, p.label)}>
              {p.label}
            </Button>
          ))}
        </div>
        <p className="example-panel__note">
          Points are on the screen, from its top-left. Near an edge the menu is kept on screen.
        </p>
      </div>
    </Panel>
  )
}
