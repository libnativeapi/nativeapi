import { ChevronRight12Regular } from '@fluentui/react-icons'
import { useState } from 'react'

import { cx, Icon } from '@dazzlabs/dazzui'

import { ANIMATIONS } from '../data'
import type { IconAnimation, MenuBackend } from '../types'
import './tray-menu.css'

export interface TrayMenuProps {
  backend: MenuBackend
  notifications: boolean
  /** The menu grows away from the bar: down under a top bar, up over a taskbar. */
  direction: 'down' | 'up'
  onItem: (label: string) => void
  onCheckbox: () => void
  onSubmenuOpened: () => void
  onAnimation: (animation: IconAnimation | null) => void
}

/**
 * The tray icon's context menu (`context_menu.dart`): small on purpose, but
 * it holds every item kind the checklist asks for — a normal item, a
 * checkbox, a disabled item, separators and a submenu that switches the
 * icon's animation from the tray. It is the platform's menu, not the app's,
 * drawn in the theme's popup sheet; WinUI 3 is the roomier one.
 */
export function TrayMenu({ backend, notifications, direction, onItem, onCheckbox, onSubmenuOpened, onAnimation }: TrayMenuProps) {
  const [submenu, setSubmenu] = useState(false)

  const openSubmenu = () => {
    if (submenu) return
    setSubmenu(true)
    onSubmenuOpened()
  }

  const item = (label: string, onSelect = () => onItem(label), extra?: { check?: boolean; disabled?: boolean }) => (
    <button
      type="button"
      role="menuitem"
      className="dz-popup__item tray-menu__item"
      data-disabled={extra?.disabled ? '' : undefined}
      disabled={extra?.disabled}
      onPointerEnter={() => setSubmenu(false)}
      onClick={event => {
        event.stopPropagation()
        onSelect()
      }}
    >
      <span className="dz-popup__check" style={{ visibility: extra?.check ? 'visible' : undefined }}>
        ✓
      </span>
      <span className="dz-popup__label">{label}</span>
    </button>
  )

  return (
    <div
      role="menu"
      className={cx('dz-popup', 'tray-menu', `tray-menu--${direction}`)}
      data-backend={backend}
      onClick={event => event.stopPropagation()}
    >
      {item('Show window')}
      <div className="tray-menu__separator" role="separator" />
      <div className="tray-menu__submenu-anchor" onPointerEnter={openSubmenu}>
        <button
          type="button"
          role="menuitem"
          aria-haspopup="menu"
          aria-expanded={submenu}
          className="dz-popup__item tray-menu__item"
          data-highlighted={submenu ? '' : undefined}
          onClick={event => {
            event.stopPropagation()
            openSubmenu()
          }}
        >
          <span className="dz-popup__check" />
          <span className="dz-popup__label">Animate</span>
          <Icon icon={ChevronRight12Regular} size={12} />
        </button>
        {submenu && (
          <div role="menu" className={cx('dz-popup', 'tray-menu', 'tray-menu__flyout')} data-backend={backend}>
            {ANIMATIONS.map(({ value, label }) => (
              <button
                key={value}
                type="button"
                role="menuitem"
                className="dz-popup__item tray-menu__item"
                onClick={event => {
                  event.stopPropagation()
                  onAnimation(value)
                }}
              >
                <span className="dz-popup__check" />
                <span className="dz-popup__label">{label}</span>
              </button>
            ))}
            <div className="tray-menu__separator" role="separator" />
            <button
              type="button"
              role="menuitem"
              className="dz-popup__item tray-menu__item"
              onClick={event => {
                event.stopPropagation()
                onAnimation(null)
              }}
            >
              <span className="dz-popup__check" />
              <span className="dz-popup__label">Stop</span>
            </button>
          </div>
        )}
      </div>
      {item('Notifications', onCheckbox, { check: notifications })}
      {item('Check for updates', undefined, { disabled: true })}
      {item('About')}
      <div className="tray-menu__separator" role="separator" />
      {item('Quit')}
    </div>
  )
}
