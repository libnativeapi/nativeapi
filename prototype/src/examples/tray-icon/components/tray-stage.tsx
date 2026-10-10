import type { ReactNode } from 'react'

import { Tooltip } from '@dazzlabs/dazzui'

import { DesktopStage } from '../../../components/desktop-stage/desktop-stage'
import { BAR_EDGE } from '../../../components/platform'
import { useNow } from '../../../components/use-now'
import type { Capabilities, IconAnimation, TrayEntry } from '../types'
import { titleOf, type TrayActions, type TrayState, timeOf } from '../use-tray'
import { IconCanvas } from './icon-canvas'
import { TrayMenu } from './tray-menu'
import './tray-stage.css'

export interface TrayStageProps {
  caps: Capabilities
  state: TrayState
  actions: TrayActions
  /** The example's window; left out while it is hidden. */
  children?: ReactNode
}

/**
 * The desktop the example runs on, with the example's icons in its bar, live,
 * and the window on the desktop below. The icons answer the mouse as the
 * platform's do: a click, a right-click and a double-click are events, the
 * trigger opens the menu, and a press anywhere else closes it (and hides a
 * popup window).
 */
export function TrayStage({ caps, state, actions, children }: TrayStageProps) {
  const edge = BAR_EDGE[caps.platform]
  return (
    <DesktopStage
      platform={caps.platform}
      appName="Tray Icon Example"
      placement={state.placement}
      onPress={actions.desktopPress}
      hint={
        state.entries.length === 0
          ? 'The example quit and took its icons with it.'
          : state.popupMode
            ? 'Popup mode: click the tray icon to show the window.'
            : 'The window is closed: choose Show window from the tray menu.'
      }
      tray={state.entries
        .filter(entry => entry.visible)
        .map(entry => (
          <TrayItem
            key={entry.number}
            entry={entry}
            caps={caps}
            state={state}
            actions={actions}
            direction={edge === 'top' ? 'down' : 'up'}
          />
        ))}
    >
      {children}
    </DesktopStage>
  )
}

interface TrayItemProps {
  entry: TrayEntry
  caps: Capabilities
  state: TrayState
  actions: TrayActions
  direction: 'down' | 'up'
}

/** One `TrayIcon` in the bar: its live image, its title where the platform draws one, its tooltip. */
function TrayItem({ entry, caps, state, actions, direction }: TrayItemProps) {
  const now = useNow(100, entry.scene === 'download' || entry.scene === 'recording')
  const title = caps.title ? titleOf(entry, now) : null
  const menuOpen = state.menuOpenFor === entry.number
  return (
    <span className="tray-stage__item-anchor">
      <Tooltip
        label={<span className="tray-stage__tooltip">{entry.tooltip}</span>}
        disabled={!entry.tooltip || menuOpen}
        side={direction === 'down' ? 'bottom' : 'top'}
      >
        <button
          type="button"
          className="tray-stage__item"
          data-open={menuOpen ? '' : undefined}
          aria-label={`Tray icon #${entry.number}`}
          onClick={event => {
            event.stopPropagation()
            actions.trayClick(entry.number, 'clicked')
          }}
          onDoubleClick={event => {
            event.stopPropagation()
            actions.trayClick(entry.number, 'doubleClicked')
          }}
          onContextMenu={event => {
            event.preventDefault()
            event.stopPropagation()
            actions.trayClick(entry.number, 'rightClicked')
          }}
        >
          <IconCanvas
            animation={entry.animation}
            still={entry.still}
            time={() => timeOf(entry)}
            pixels={16 * entry.scale}
            size={16}
            color={entry.color}
          />
          {title && <span className="tray-stage__title">{title}</span>}
        </button>
      </Tooltip>
      {menuOpen && (
        <TrayMenu
          backend={state.backend}
          notifications={state.notifications}
          direction={direction}
          onItem={actions.menuItem}
          onCheckbox={actions.menuCheckbox}
          onSubmenuOpened={actions.menuSubmenuOpened}
          onAnimation={(animation: IconAnimation | null) => actions.menuAnimation(animation)}
        />
      )}
    </span>
  )
}
