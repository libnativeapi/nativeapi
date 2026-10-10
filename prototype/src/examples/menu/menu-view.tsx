import { type MouseEvent, useEffect, useRef, useState } from 'react'

import {
  Badge,
  SegmentedControl,
  SidebarGroup,
  SidebarGroupContent,
  SidebarGroupLabel,
  Tree,
  type TreeNode,
  type WindowFramePlatform,
} from '@dazzlabs/dazzui'

import { DesktopStage, DesktopWindow } from '../../components/desktop-stage'
import { EventBar } from '../../components/event-bar'
import { ExampleWindow } from '../../components/example-window'
import { osOf } from '../../components/platform'
import { EditPanel } from './components/edit-panel'
import { NativeMenu } from './components/native-menu'
import { OpenPanel } from './components/open-panel'
import { SettingsPanel } from './components/settings-panel'
import { CONTEXT_MENU, POSITIONING_MENU } from './data'
import type { MenuModel, Tab } from './types'
import { type MenuOptions, useMenus } from './use-menus'
import './menu-view.css'

export interface MenuViewProps {
  platform: WindowFramePlatform
  options?: MenuOptions
  initialTab?: Tab
}

const TITLE = 'Menu Example'

/** The sidebar's tree of a menu: its items in order, the submenu's under its item. */
function treeOf(menu: MenuModel, menus: Record<number, MenuModel>): TreeNode[] {
  return menu.items.map(item => {
    const id = `${menu.id}:${item.id}`
    if (item.type === 'separator') {
      return { id, label: <span className="menu-view__separator-node">Separator</span>, text: 'Separator', disabled: true }
    }
    const mark =
      item.state === 'mixed' ? '–' : item.state === 'checked' ? (item.type === 'radio' ? '●' : '✓') : item.enabled ? '' : 'off'
    const sub = item.submenu !== null ? menus[item.submenu] : undefined
    return {
      id,
      label: item.label,
      meta: mark ? <span className="menu-view__node-meta">{mark}</span> : undefined,
      ...(item.type === 'submenu' ? { children: sub ? treeOf(sub, menus) : [] } : {}),
    }
  })
}

/** The `data-theme` an appearance forced by `Application.setBrightness` stands for. */
function themeFor(choice: 'system' | 'light' | 'dark') {
  if (choice === 'system' || typeof document === 'undefined') return undefined
  const current = document.documentElement.getAttribute('data-theme') ?? 'studio-light'
  return `${current.replace(/-(light|dark)$/, '')}-${choice}`
}

/**
 * The menu example: two native menus and everything that can be done to
 * them. Right-clicking the region opens the context menu at the pointer,
 * drawn over the desktop as the platform draws it; the positioning menu
 * opens at points on the screen. The sidebar holds both menus' items as a
 * tree, the Edit tab changes them while they exist, and Settings picks the
 * backend and the appearance the menus follow.
 */
export function MenuView({ platform, options, initialTab = 'open' }: MenuViewProps) {
  const os = osOf(platform)
  const { menus, context, submenu, submenuItem, open, animation, backend, theme, placement, selectedItem, winUi3, log, actions } =
    useMenus(platform, options)
  const [tab, setTab] = useState<Tab>(initialTab)
  const [selected, setSelected] = useState<string | null>(`${CONTEXT_MENU}:${selectedItem}`)
  const layer = useRef<HTMLDivElement>(null)
  const cursor = useRef({ x: 0, y: 0 })
  const forced = themeFor(theme)

  /** Screen points (the stage's top-left is the screen's) to the layer the menus stand in. */
  const offset = () => {
    const el = layer.current
    const stage = el?.closest('.desktop-stage')
    if (!el || !stage) return { x: 0, y: 0, layer: new DOMRect(), stage: new DOMRect() }
    const l = el.getBoundingClientRect()
    const s = stage.getBoundingClientRect()
    return { x: l.left - s.left, y: l.top - s.top, layer: l, stage: s }
  }
  const screenOf = (event: MouseEvent) => {
    const { stage } = offset()
    return { x: Math.round(event.clientX - stage.left), y: Math.round(event.clientY - stage.top) }
  }

  // The cursor, for PositioningStrategy.cursorPosition().
  useEffect(() => {
    const stage = layer.current?.closest('.desktop-stage') as HTMLElement | null
    if (!stage) return
    const move = (event: PointerEvent) => {
      const s = stage.getBoundingClientRect()
      cursor.current = { x: event.clientX - s.left, y: event.clientY - s.top }
    }
    stage.addEventListener('pointermove', move)
    return () => stage.removeEventListener('pointermove', move)
  }, [])

  // Escape closes an open menu, as the platform's menu loop does.
  useEffect(() => {
    if (!open) return
    const key = (event: KeyboardEvent) => event.key === 'Escape' && actions.close()
    window.addEventListener('keydown', key)
    return () => window.removeEventListener('keydown', key)
  }, [open, actions])

  const openContext = (event: MouseEvent) => {
    const at = screenOf(event)
    const o = offset()
    actions.openMenu(CONTEXT_MENU, at.x - o.x, at.y - o.y, placement, 'cursorPosition()')
  }
  const openAt = (x: number, y: number) => {
    const o = offset()
    actions.openMenu(POSITIONING_MENU, x - o.x, y - o.y, 'bottomStart', `PositioningStrategy.absolute(${x}, ${y})`)
  }

  const pick = (id: string) => {
    setSelected(id)
    actions.selectItem(Number(id.split(':')[1]))
  }
  const selection = (() => {
    if (!selected) return null
    const [menuId, itemId] = selected.split(':').map(Number)
    // A submenu's items sit under their item in the context menu's tree.
    for (const menu of Object.values(menus)) {
      const item = menu.items.find(i => i.id === itemId)
      if (item) return { item, menu }
    }
    void menuId
    return null
  })()

  const exampleWindow = (
    <ExampleWindow
      platform={platform}
      appName={TITLE}
      width={860}
      height={580}
      title="Context menu"
      subtitle={`menu ${CONTEXT_MENU}`}
      toolbar={
        <SegmentedControl<Tab>
          size="small"
          items={[
            { value: 'open', label: 'Open' },
            { value: 'edit', label: 'Edit' },
            { value: 'settings', label: 'Settings' },
          ]}
          value={tab}
          onValueChange={setTab}
        />
      }
      trailing={
        <Badge size="small" variant="tinted" tint={backend === 'winUi3' ? 'info' : 'neutral'}>
          {backend === 'winUi3' ? 'WinUI 3' : 'Native'}
        </Badge>
      }
      sidebar={
        <>
          <SidebarGroup>
            <SidebarGroupLabel>Context menu · {context.items.length}</SidebarGroupLabel>
            <SidebarGroupContent>
              <Tree
                size="small"
                aria-label="Context menu items"
                items={treeOf(context, menus)}
                defaultExpanded={[`${CONTEXT_MENU}:13`]}
                selected={selected}
                onSelectedChange={pick}
              />
            </SidebarGroupContent>
          </SidebarGroup>
          <SidebarGroup>
            <SidebarGroupLabel>Positioning menu · {menus[POSITIONING_MENU]!.items.length}</SidebarGroupLabel>
            <SidebarGroupContent>
              <Tree
                size="small"
                aria-label="Positioning menu items"
                items={treeOf(menus[POSITIONING_MENU]!, menus)}
                selected={selected}
                onSelectedChange={pick}
              />
            </SidebarGroupContent>
          </SidebarGroup>
        </>
      }
      footer={<EventBar lastEvent={log.lastEvent} log={log.log} onClear={log.clear} />}
    >
      <div className="example-window__scroll">
        {tab === 'open' && (
          <OpenPanel
            context={context}
            submenu={submenu}
            submenuItem={submenuItem}
            placement={placement}
            menuOpen={open?.menuId === CONTEXT_MENU}
            onPlacement={actions.setPlacement}
            onRegion={openContext}
            onOpenAt={openAt}
            onOpenAtCursor={() => {
              const o = offset()
              actions.openMenu(
                POSITIONING_MENU,
                cursor.current.x - o.x,
                cursor.current.y - o.y,
                'bottomStart',
                'PositioningStrategy.cursorPosition()',
              )
            }}
          />
        )}
        {tab === 'edit' && (
          <EditPanel
            context={context}
            submenu={submenu}
            submenuItem={submenuItem}
            selected={selection}
            animation={animation}
            actions={actions}
          />
        )}
        {tab === 'settings' && (
          <SettingsPanel
            os={os}
            backend={backend}
            theme={theme}
            winUi3={winUi3}
            actions={actions}
            onRapid={() => actions.rapid((x, y) => ({ x: x - offset().x, y: y - offset().y }))}
            onIssueMenu={() => actions.openIssueMenu(200 - offset().x, 200 - offset().y)}
          />
        )}
      </div>
    </ExampleWindow>
  )

  return (
    <DesktopStage
      platform={platform}
      appName={TITLE}
      layout="free"
      hint="Right-click the area in the window: the menu opens over the desktop, where the pointer is."
      overlay={
        <div ref={layer} className="menu-view__layer">
          {open && (
            <>
              {/* A press anywhere outside the menu dismisses it; a right-click on the region opens it again. */}
              <div
                className="menu-view__catcher"
                onPointerDown={() => actions.close()}
                onContextMenu={event => {
                  event.preventDefault()
                  const under = document.elementsFromPoint(event.clientX, event.clientY)
                  if (under.some(el => el.closest('.menu-panel__region'))) openContext(event)
                }}
              />
              <NativeMenu
                key={`${open.menuId}-${open.x}-${open.y}`}
                menus={menus}
                menuId={open.menuId}
                anchor={{ x: open.x, y: open.y }}
                placement={open.placement}
                backend={backend}
                os={os}
                animation={animation}
                theme={forced}
                onItem={actions.clickItem}
                onSubmenuOpened={actions.submenuOpened}
                onSubmenuClosed={actions.submenuClosed}
              />
            </>
          )}
        </div>
      }
    >
      <DesktopWindow x={40} y={28}>
        <div className="menu-view__frame" data-theme={forced}>
          {exampleWindow}
        </div>
      </DesktopWindow>
    </DesktopStage>
  )
}
