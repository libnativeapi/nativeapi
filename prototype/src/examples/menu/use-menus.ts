import { useCallback, useEffect, useRef, useState } from 'react'

import type { WindowFramePlatform } from '@dazzlabs/dazzui'

import { useEventLog } from '../../components/event-bar'
import { osOf } from '../../components/platform'
import { nowSeconds } from '../../components/use-now'
import {
  CONTEXT_MENU,
  FIRST_FREE_ITEM,
  initialMenus,
  ISSUE_MENU,
  issueMenu,
  ITEM,
  POSITIONING_MENU,
  STATE_NAMES,
  SUBMENU,
} from './data'
import type {
  IconAnimation,
  ItemIcon,
  ItemState,
  MenuBackend,
  MenuItemModel,
  MenuModel,
  OpenMenu,
  Placement,
  ThemeChoice,
} from './types'

export interface MenuOptions {
  backend?: MenuBackend
  theme?: ThemeChoice
  /** Start with the context menu open, as if it had been right-clicked. */
  openAtStart?: boolean
  /** Start with a round of edits made: items added and inserted, an icon animating. */
  edited?: boolean
}

/** A playing icon animation: which item it draws on, and since when. */
export interface Animation {
  kind: IconAnimation
  itemId: number
  startedAt: number
}

const PLACEMENT_ENUM = (p: Placement) => `Placement.${p}`
const BACKEND_ENUM: Record<MenuBackend, string> = { native: 'kNative', winUi3: 'kWinUI3' }

/** The first item that is not a separator: what the example's `_menuItems[0]` is. */
export const firstItemOf = (menu: MenuModel) => menu.items.find(i => i.type !== 'separator') ?? null

/**
 * The example's two menus and everything it does to them, simulated: the
 * calls it makes, the events the menus emit — opened, closed, item clicked,
 * submenu opened and closed — and the handlers that answer them, which keep
 * the checkbox and the radio group in step.
 */
export function useMenus(platform: WindowFramePlatform, options: MenuOptions = {}) {
  const os = osOf(platform)
  const winUi3 = os === 'windows'
  const accelerator = os === 'macos' ? '⌘N' : 'Ctrl+N'
  const log = useEventLog(
    [
      'Image.fromAsset("images/flutter_logo.png") → image',
      `Menu.create() → menu ${CONTEXT_MENU} · ${initialMenus(accelerator)[CONTEXT_MENU]!.items.length} items`,
      `Menu.create() → menu ${POSITIONING_MENU} · 2 items`,
      `isBackendSupported(kWinUI3) → ${winUi3}`,
      ...(winUi3 ? ['setBackend(kWinUI3) → true · both menus'] : []),
    ],
    'Right-click the area to open the menu',
  )
  const { event, call } = log

  const [menus, setMenus] = useState<Record<number, MenuModel>>(() => initialMenus(accelerator))
  const [open, setOpenState] = useState<OpenMenu | null>(null)
  const [animation, setAnimation] = useState<Animation | null>(null)
  const [backend, setBackendState] = useState<MenuBackend>(options.backend ?? (winUi3 ? 'winUi3' : 'native'))
  const [theme, setThemeState] = useState<ThemeChoice>(options.theme ?? 'system')
  const [placement, setPlacementState] = useState<Placement>('bottomStart')
  const [selectedItem, setSelectedItem] = useState<number | null>(ITEM.normal)
  const nextItem = useRef(FIRST_FREE_ITEM)
  const counters = useRef({ added: 0, submenu: 0 })

  // Timers (Rapid open/close) act after renders: they read the state through refs.
  const menusRef = useRef(menus)
  menusRef.current = menus
  const openRef = useRef(open)
  const setOpen = (value: OpenMenu | null) => {
    openRef.current = value
    setOpenState(value)
  }

  const context = menus[CONTEXT_MENU]!
  const submenu = menus[SUBMENU]!
  const submenuItem = context.items.find(i => i.id === ITEM.submenu) ?? null

  const update = (menuId: number, change: (items: MenuItemModel[]) => MenuItemModel[]) =>
    setMenus(all => ({ ...all, [menuId]: { ...all[menuId]!, items: change(all[menuId]!.items) } }))
  const updateItem = (menuId: number, id: number, change: Partial<MenuItemModel>) =>
    update(menuId, items => items.map(i => (i.id === id ? { ...i, ...change } : i)))

  const nameOf = (menuId: number) => menusRef.current[menuId]?.name ?? `Menu ${menuId}`

  // --- Opening and closing ---------------------------------------------------

  const close = useCallback(() => {
    const current = openRef.current
    if (!current) return
    setOpen(null)
    event(`${nameOf(current.menuId)} closed`, `MenuClosedEvent → menu ${current.menuId}`)
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [event])

  /** `Menu.open(strategy, placement)`; `strategy` is how the log names it. */
  const openMenu = (menuId: number, x: number, y: number, at: Placement, strategy: string) => {
    if (openRef.current) close()
    call(`${menuId === CONTEXT_MENU ? 'contextMenu' : menuId === ISSUE_MENU ? 'menu' : 'positioningMenu'}.open(${strategy}, ${PLACEMENT_ENUM(at)}) → true`)
    setOpen({ menuId, x: Math.round(x), y: Math.round(y), placement: at })
    event(`${nameOf(menuId)} opened`, `MenuOpenedEvent → menu ${menuId}`)
  }

  useEffect(() => {
    if (options.openAtStart) openMenu(CONTEXT_MENU, 420, 300, 'bottomStart', 'cursorPosition()')
    // Once, as the example starts.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [])

  // --- What the menus emit, and the example's handlers -----------------------

  const clickItem = (menuId: number, item: MenuItemModel) => {
    // A disabled item emits nothing: the platform never reports the click.
    if (!item.enabled || item.type === 'separator') return
    event(`Clicked ${item.label}`, `MenuItemClickedEvent → item ${item.id} "${item.label}"`)
    if (menuId === CONTEXT_MENU && item.id === ITEM.checkbox) {
      const next: ItemState = item.state === 'checked' ? 'unchecked' : 'checked'
      updateItem(CONTEXT_MENU, item.id, { state: next })
      call(`checkboxItem.setState(${STATE_NAMES[next]})`)
    } else if (menuId === CONTEXT_MENU && item.type === 'radio') {
      update(CONTEXT_MENU, items =>
        items.map(i => (i.type === 'radio' && i.radioGroup === item.radioGroup ? { ...i, state: i.id === item.id ? 'checked' : 'unchecked' } : i)),
      )
      call(`${item.label}.setState(kChecked) · the others kUnchecked`)
    }
    close()
  }

  const submenuOpened = (item: MenuItemModel) =>
    event(`Submenu opened: ${item.label}`, `MenuItemSubmenuOpenedEvent → item ${item.id}`)
  const submenuClosed = (item: MenuItemModel) =>
    event(`Submenu closed: ${item.label}`, `MenuItemSubmenuClosedEvent → item ${item.id}`)

  // --- Edits -----------------------------------------------------------------

  const newItem = (label: string): MenuItemModel => ({
    id: nextItem.current++,
    type: 'normal',
    label,
    state: 'unchecked',
    enabled: true,
    tooltip: null,
    radioGroup: -1,
    icon: null,
    accelerator: null,
    submenu: null,
  })

  const count = () => menusRef.current[CONTEXT_MENU]!.items.length

  const addItem = () => {
    counters.current.added += 1
    const added = newItem(`New Item ${counters.current.added}`)
    menusRef.current = {
      ...menusRef.current,
      [CONTEXT_MENU]: { ...menusRef.current[CONTEXT_MENU]!, items: [...menusRef.current[CONTEXT_MENU]!.items, added] },
    }
    update(CONTEXT_MENU, items => [...items, added])
    call(`addItem(MenuItem("${added.label}")) → itemCount ${count()}`)
  }

  const insertItem = () => {
    const inserted = newItem('Inserted Item')
    update(CONTEXT_MENU, items => [...items.slice(0, 2), inserted, ...items.slice(2)])
    call(`insertItem(2, MenuItem("Inserted Item")) → itemCount ${count() + 1}`)
  }

  const insertSeparator = () => {
    const sep: MenuItemModel = { ...newItem(''), type: 'separator' }
    update(CONTEXT_MENU, items => [...items.slice(0, 3), sep, ...items.slice(3)])
    call(`insertSeparator(3) → itemCount ${count() + 1}`)
  }

  const removeFirst = () => {
    const first = firstItemOf(context)
    if (!first) return call('removeItem(…) → no items left')
    update(CONTEXT_MENU, items => items.filter(i => i.id !== first.id))
    call(`removeItem(item ${first.id} "${first.label}") → true`)
  }

  const removeAt = (index: number) => {
    if (index < 0 || index >= context.items.length) return call(`removeItemAt(${index}) → false`)
    update(CONTEXT_MENU, items => items.filter((_, i) => i !== index))
    const removed = context.items[index]!
    call(`removeItemAt(${index}) → true · ${removed.type === 'separator' ? 'a separator' : `"${removed.label}"`}`)
  }

  const updateLabel = () => {
    const dynamic = context.items.find(i => i.id === ITEM.dynamicLabel)
    if (!dynamic) return call('setLabel(…) → the item was removed')
    const label = `Updated at ${new Date().toTimeString().slice(0, 8)}`
    updateItem(CONTEXT_MENU, ITEM.dynamicLabel, { label })
    call(`dynamicLabelItem.setLabel("${label}")`)
  }

  const checkboxMixed = () => {
    if (!context.items.some(i => i.id === ITEM.checkbox)) return call('setState(…) → the item was removed')
    updateItem(CONTEXT_MENU, ITEM.checkbox, { state: 'mixed' })
    call('checkboxItem.setState(kMixed)')
  }

  const addSubmenuItem = () => {
    counters.current.submenu += 1
    const added = newItem(`Dynamic Submenu Item ${submenu.items.length + 1}`)
    update(SUBMENU, items => [...items, added])
    call(`submenu.addItem(MenuItem("${added.label}")) → itemCount ${submenu.items.length + 1}`)
  }

  const toggleSubmenu = () => {
    if (!submenuItem) return
    const attach = submenuItem.submenu === null
    updateItem(CONTEXT_MENU, ITEM.submenu, { submenu: attach ? SUBMENU : null })
    call(attach ? `submenuItem.setSubmenu(menu ${SUBMENU})` : 'submenuItem.setSubmenu(null)')
  }

  const setIcon = (icon: ItemIcon | null) => {
    const first = firstItemOf(context)
    if (!first) return call('setIcon(…) → no items left')
    setAnimation(null)
    updateItem(CONTEXT_MENU, first.id, { icon })
    call(
      icon === 'asset'
        ? `item ${first.id}.setIcon(Image.fromAsset("images/flutter_logo.png"))`
        : icon === 'widget'
          ? `item ${first.id}.setIcon(Image.fromBase64(star_16_filled → PNG))`
          : `item ${first.id}.setIcon(null)`,
    )
  }

  const animate = (kind: IconAnimation | null) => {
    const first = firstItemOf(context)
    if (kind === null) {
      if (animation) call(`AnimatedIconGenerator.stop() · ${animation.kind}`)
      setAnimation(null)
      return
    }
    if (!first) return call('AnimatedIconGenerator → no items left')
    setAnimation({ kind, itemId: first.id, startedAt: nowSeconds() })
    call(`AnimatedIconGenerator.start${kind[0]!.toUpperCase()}${kind.slice(1)}() → item ${first.id}.setIcon(frame) every 100 ms`)
  }

  // --- Settings --------------------------------------------------------------

  const setBackend = (value: MenuBackend) => {
    if (value === backend) return
    const ok = value === 'native' || winUi3
    if (ok) setBackendState(value)
    call(`setBackend(${BACKEND_ENUM[value]}) → context ${ok}, positioning ${ok}`)
  }

  const setTheme = (value: ThemeChoice) => {
    if (value === theme) return
    setThemeState(value)
    call(`Application.setBrightness(Brightness.${value}) → true`)
    event(`Theme: ${value}`)
  }

  const setPlacement = (value: Placement) => {
    if (value === placement) return
    setPlacementState(value)
    call(`placement → ${PLACEMENT_ENUM(value)}`)
  }

  const addTen = () => {
    for (let i = 0; i < 10; i++) addItem()
  }

  /** Five opens of the positioning menu, each closed 100 ms later. */
  const rapid = (toScreen: (x: number, y: number) => { x: number; y: number }) => {
    for (let i = 0; i < 5; i++) {
      window.setTimeout(() => {
        const at = toScreen(100 + i * 50, 100 + i * 50)
        openMenu(POSITIONING_MENU, at.x, at.y, 'bottomStart', `PositioningStrategy.absolute(${100 + i * 50}, ${100 + i * 50})`)
        window.setTimeout(close, 100)
      }, i * 180)
    }
  }

  /** Issue #4: a checked item and a disabled item, built fresh and opened at (200, 200). */
  const openIssueMenu = (x: number, y: number) => {
    const menu = issueMenu(nextItem.current)
    nextItem.current += menu.items.length
    menusRef.current = { ...menusRef.current, [ISSUE_MENU]: menu }
    setMenus(all => ({ ...all, [ISSUE_MENU]: menu }))
    call(`Menu.create() → menu ${ISSUE_MENU} · Checkable kChecked, Disabled isEnabled false`)
    openMenu(ISSUE_MENU, x, y, 'bottomStart', 'PositioningStrategy.absolute(200, 200)')
  }

  // The Edited story: a round of edits already made.
  useEffect(() => {
    if (!options.edited) return
    addItem()
    addItem()
    insertItem()
    insertSeparator()
    updateLabel()
    checkboxMixed()
    addSubmenuItem()
    // After the inserts, the first item is still Normal Menu Item.
    setAnimation({ kind: 'spinner', itemId: ITEM.normal, startedAt: nowSeconds() })
    call(`AnimatedIconGenerator.startSpinner() → item ${ITEM.normal}.setIcon(frame) every 100 ms`)
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [])

  return {
    menus,
    context,
    submenu,
    submenuItem,
    open,
    animation,
    backend,
    theme,
    placement,
    selectedItem,
    winUi3,
    log,
    actions: {
      openMenu,
      close,
      clickItem,
      submenuOpened,
      submenuClosed,
      addItem,
      insertItem,
      insertSeparator,
      removeFirst,
      removeAt,
      updateLabel,
      checkboxMixed,
      addSubmenuItem,
      toggleSubmenu,
      setIcon,
      animate,
      setBackend,
      setTheme,
      setPlacement,
      addTen,
      rapid,
      openIssueMenu,
      selectItem: setSelectedItem,
    },
  }
}

export type MenuActions = ReturnType<typeof useMenus>['actions']
