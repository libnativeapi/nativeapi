import { ChevronRight12Regular } from '@fluentui/react-icons'
import { useEffect, useLayoutEffect, useRef, useState } from 'react'

import { cx, Icon } from '@dazzlabs/dazzui'

import type { Os } from '../../../components/platform'
import type { Animation } from '../use-menus'
import type { MenuBackend, MenuItemModel, MenuModel, Placement } from '../types'
import { MenuIcon } from './menu-icon'
import './native-menu.css'

export interface NativeMenuProps {
  menus: Record<number, MenuModel>
  menuId: number
  /** Where `open` was asked to put it, in the layer's coordinates. */
  anchor: { x: number; y: number }
  placement: Placement
  backend: MenuBackend
  os: Os
  animation: Animation | null
  /** The appearance `Application.setBrightness` forced, as a `data-theme`. */
  theme?: string
  onItem: (menuId: number, item: MenuItemModel) => void
  onSubmenuOpened: (item: MenuItemModel) => void
  onSubmenuClosed: (item: MenuItemModel) => void
}

/** Where a menu of `w` × `h` stands for a placement around the anchor. */
function placeAt(placement: Placement, ax: number, ay: number, w: number, h: number) {
  const side = placement.replace(/Start|End/, '') as 'top' | 'right' | 'bottom' | 'left'
  const align = placement.endsWith('Start') ? 'start' : placement.endsWith('End') ? 'end' : 'center'
  if (side === 'top' || side === 'bottom') {
    return {
      left: align === 'start' ? ax : align === 'end' ? ax - w : ax - w / 2,
      top: side === 'top' ? ay - h : ay,
    }
  }
  return {
    left: side === 'left' ? ax - w : ax,
    top: align === 'start' ? ay : align === 'end' ? ay - h : ay - h / 2,
  }
}

/**
 * A native menu as the platform draws it — NSMenu, a Win32 or WinUI 3
 * flyout, a GTK menu — over everything on the screen, at the placement asked
 * for and moved back inside the screen where it would cross an edge. Items
 * highlight under the pointer, a submenu opens on hover, a tooltip after a
 * pause; a disabled item takes no click.
 */
export function NativeMenu({ menus, menuId, anchor, placement, backend, os, animation, theme, ...handlers }: NativeMenuProps) {
  const ref = useRef<HTMLDivElement>(null)
  const [pos, setPos] = useState<{ left: number; top: number } | null>(null)
  const menu = menus[menuId]

  useLayoutEffect(() => {
    const el = ref.current
    const layer = el?.parentElement
    if (!el || !layer) return
    const w = el.offsetWidth
    const h = el.offsetHeight
    const at = placeAt(placement, anchor.x, anchor.y, w, h)
    // Kept on screen: the platform slides a menu back inside the edges.
    setPos({
      left: Math.max(4, Math.min(at.left, layer.clientWidth - w - 4)),
      top: Math.max(4, Math.min(at.top, layer.clientHeight - h - 4)),
    })
  }, [anchor.x, anchor.y, placement, backend, menu?.items.length])

  if (!menu) return null
  return (
    <div
      ref={ref}
      role="menu"
      className="dz-popup native-menu"
      data-backend={backend}
      data-os={os}
      data-theme={theme}
      style={{ left: pos?.left ?? anchor.x, top: pos?.top ?? anchor.y, visibility: pos ? undefined : 'hidden' }}
      onPointerDown={event => event.stopPropagation()}
      onContextMenu={event => event.preventDefault()}
    >
      <MenuItems menu={menu} menus={menus} backend={backend} os={os} animation={animation} theme={theme} {...handlers} />
    </div>
  )
}

type ItemsProps = Omit<NativeMenuProps, 'menuId' | 'anchor' | 'placement'> & { menu: MenuModel }

function MenuItems({ menu, menus, backend, os, animation, theme, onItem, onSubmenuOpened, onSubmenuClosed }: ItemsProps) {
  const [hovered, setHovered] = useState<number | null>(null)
  // A tooltip shows after the pointer rests on its item, as the platform's does.
  const [tip, setTip] = useState<number | null>(null)
  const timer = useRef(0)
  useEffect(() => () => clearTimeout(timer.current), [])
  // The menu reserves an icon column as soon as one item has an icon.
  const icons = menu.items.some(i => i.icon || animation?.itemId === i.id)
  const radioGlyph = os === 'macos' ? '✓' : '●'

  return (
    <>
      {menu.items.map(item => {
        if (item.type === 'separator') return <div key={item.id} role="separator" className="native-menu__separator" />
        const sub = item.type === 'submenu' && item.submenu !== null ? menus[item.submenu] : undefined
        const highlighted = hovered === item.id && item.enabled
        const check =
          item.state === 'mixed' ? '–' : item.state === 'checked' ? (item.type === 'radio' ? radioGlyph : '✓') : ''
        const button = (
          <button
            type="button"
            role={item.type === 'checkbox' ? 'menuitemcheckbox' : item.type === 'radio' ? 'menuitemradio' : 'menuitem'}
            aria-checked={item.type === 'checkbox' || item.type === 'radio' ? item.state === 'checked' : undefined}
            aria-haspopup={sub ? 'menu' : undefined}
            aria-disabled={!item.enabled || undefined}
            className="dz-popup__item native-menu__item"
            data-highlighted={highlighted ? '' : undefined}
            data-disabled={item.enabled ? undefined : ''}
            onPointerEnter={() => {
              setHovered(item.id)
              setTip(null)
              clearTimeout(timer.current)
              if (item.tooltip) timer.current = window.setTimeout(() => setTip(item.id), 600)
            }}
            onPointerLeave={() => {
              clearTimeout(timer.current)
              setTip(null)
            }}
            onClick={() => {
              // A submenu item only opens its submenu.
              if (!sub) onItem(menu.id, item)
            }}
          >
            <span className="dz-popup__check native-menu__check" style={{ visibility: check ? 'visible' : undefined }}>
              {check}
            </span>
            {icons && (
              <span className="native-menu__icon">
                <MenuIcon icon={item.icon} animation={animation?.itemId === item.id ? animation : null} />
              </span>
            )}
            <span className="dz-popup__label">{item.label}</span>
            {item.accelerator && <span className="dz-popup__shortcut">{item.accelerator}</span>}
            {sub && <Icon icon={ChevronRight12Regular} size={12} />}
          </button>
        )
        return (
          <div key={item.id} className="native-menu__anchor">
            {button}
            {tip === item.id && item.tooltip && (
              <span role="tooltip" className="dz-tooltip native-menu__tooltip">
                {item.tooltip}
              </span>
            )}
            {sub && highlighted && (
              <Flyout
                item={item}
                menu={sub}
                menus={menus}
                backend={backend}
                os={os}
                animation={animation}
                theme={theme}
                onItem={onItem}
                onSubmenuOpened={onSubmenuOpened}
                onSubmenuClosed={onSubmenuClosed}
              />
            )}
          </div>
        )
      })}
      {menu.items.length === 0 && <div className="dz-popup__empty">No items</div>}
    </>
  )
}

/**
 * A submenu, beside its item: to the right where it fits, to the left where
 * the screen ends. It reports opening and closing on the submenu item.
 */
function Flyout({ item, ...props }: ItemsProps & { item: MenuItemModel }) {
  const ref = useRef<HTMLDivElement>(null)
  const [flip, setFlip] = useState(false)
  const opened = useRef(props.onSubmenuOpened)
  const closed = useRef(props.onSubmenuClosed)

  useEffect(() => {
    opened.current(item)
    return () => closed.current(item)
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [item.id])

  useLayoutEffect(() => {
    const el = ref.current
    const layer = el?.closest('.menu-view__layer')
    if (!el || !layer) return
    setFlip(el.getBoundingClientRect().right > layer.getBoundingClientRect().right - 4)
  }, [])

  return (
    <div
      ref={ref}
      role="menu"
      className={cx('dz-popup', 'native-menu', 'native-menu__flyout', flip && 'native-menu__flyout--flip')}
      data-backend={props.backend}
      data-os={props.os}
      data-theme={props.theme}
    >
      <MenuItems {...props} />
    </div>
  )
}
