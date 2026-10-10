import type { ReactNode } from 'react'

import { Tooltip, WindowFrame } from '@dazzlabs/dazzui'

import { DesktopStage } from '../../../components/desktop-stage/desktop-stage'
import { BAR_EDGE } from '../../../components/platform'
import { useNow } from '../../../components/use-now'
import type { Capabilities, IconAnimation, TrayEntry } from '../types'
import { titleOf, type TrayActions, type TrayState, timeOf } from '../use-tray'
import { IconCanvas } from './icon-canvas'
import { TrayMenu } from './tray-menu'
import { SignArt } from './sign-art'
import { headingOf } from '../sign-data'
import './tray-stage.css'

export interface TrayStageProps {
  caps: Capabilities
  state: TrayState
  actions: TrayActions
  /** The example's window; left out while it is hidden. */
  children?: ReactNode
  onSignClick?: () => void
}

/**
 * The desktop the example runs on, with the example's icons in its bar, live,
 * and the window on the desktop below. The icons answer the mouse as the
 * platform's do: a click, a right-click and a double-click are events, the
 * trigger opens the menu, and a press anywhere else closes it (and hides a
 * popup window).
 */
export function TrayStage({ caps, state, actions, children, onSignClick }: TrayStageProps) {
  const edge = BAR_EDGE[caps.platform]
  return (
    <DesktopStage
      platform={caps.platform}
      appName="Tray Icon Example"
      className={state.entries.some(entry => entry.contentMode === 'sign') ? 'tray-stage--signs' : undefined}
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
        .filter(entry => entry.visible && (entry.contentMode === 'icon' || caps.contentView))
        .map(entry => (
          <TrayItem
            key={entry.number}
            entry={entry}
            caps={caps}
            state={state}
            actions={actions}
            direction={edge === 'top' ? 'down' : 'up'}
            onSignClick={onSignClick}
          />
        ))}
      overlay={state.previewSigns.map((number, index) => {
        const entry = state.entries.find(item => item.number === number)
        return entry && <div key={number} className="tray-stage__sign-preview"
          style={{ transform: `translate(${index * 20}px, ${index * 20}px)` }}>
          <WindowFrame platform={caps.platform} title={`${headingOf(entry.sign)} · Enlarged preview`} width={560} minWidth={240}
            controls={{ close: true }} onClose={() => actions.closeSignPreview(number)}>
            <div className="tray-stage__enlarged-sign"><SignArt entry={entry.sign} size="large" /></div>
          </WindowFrame>
        </div>
      })}
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
  onSignClick?: () => void
}

/** One `TrayIcon` in the bar: its live image, its title where the platform draws one, its tooltip. */
function TrayItem({ entry, caps, state, actions, direction, onSignClick }: TrayItemProps) {
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
          className={`tray-stage__item${entry.contentMode === 'sign' ? ' tray-stage__item--sign' : ''}`}
          data-open={menuOpen ? '' : undefined}
          aria-label={`Tray ${entry.contentMode} #${entry.number}`}
          onClick={event => {
            event.stopPropagation()
            actions.trayClick(entry.number, 'clicked')
            if (entry.contentMode === 'sign') onSignClick?.()
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
          {entry.contentMode === 'sign' ? <SignArt entry={entry.sign} size="tray" /> : <IconCanvas
            animation={entry.animation}
            still={entry.still}
            time={() => timeOf(entry)}
            pixels={16 * entry.scale}
            size={16}
            color={entry.color}
          />}
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
          onPreviewSign={entry.contentMode === 'sign' ? () => { actions.previewSign(entry.number); actions.closeMenu() } : undefined}
        />
      )}
    </span>
  )
}
