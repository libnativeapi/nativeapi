import { Button, PreferenceRow, PreferenceSection, SegmentedControl } from '@dazzlabs/dazzui'

import { Panel, PanelPreferences } from '../../../components/panel'
import { ReadBack } from '../../../components/read-back'
import { ANIMATIONS, ITEM, STATE_NAMES, TYPE_NAMES } from '../data'
import type { IconAnimation, MenuItemModel, MenuModel } from '../types'
import { type Animation, firstItemOf, type MenuActions } from '../use-menus'
import './panels.css'

export interface EditPanelProps {
  context: MenuModel
  submenu: MenuModel
  submenuItem: MenuItemModel | null
  /** The item picked in the sidebar, and the menu it is in. */
  selected: { item: MenuItemModel; menu: MenuModel } | null
  animation: Animation | null
  actions: MenuActions
}

const quote = (s: string | null) => (s === null ? 'null' : `"${s}"`)

/**
 * The calls that change the context menu while it exists: add, insert and
 * remove items, change a label and a state, grow or detach the submenu, set,
 * remove and animate the first item's icon. The card on top reads the item
 * picked in the sidebar back from its getters.
 */
export function EditPanel({ context, submenu, submenuItem, selected, animation, actions }: EditPanelProps) {
  const first = firstItemOf(context)
  const item = selected?.item
  const iconOf = (i: MenuItemModel) =>
    animation?.itemId === i.id ? `frame · ${animation.kind}` : i.icon === 'asset' ? 'flutter_logo.png' : i.icon === 'widget' ? 'star (PNG)' : 'null'

  return (
    <Panel>
      {item && selected ? (
        <ReadBack
          label={`Item ${item.id} in ${selected.menu.name.toLowerCase()} · index ${selected.menu.items.indexOf(item)}`}
          keyWidth="7.5rem"
          rows={[
            ['getType', TYPE_NAMES[item.type]],
            ['getLabel', item.type === 'separator' ? 'null' : quote(item.label)],
            ['getState', STATE_NAMES[item.state]],
            ['isEnabled', String(item.enabled)],
            ['getTooltip', quote(item.tooltip)],
            ['getRadioGroup', String(item.radioGroup)],
            ['getSubmenu', item.submenu === null ? 'null' : `menu ${item.submenu}`],
            ['getIcon', iconOf(item)],
            ['getAccelerator', item.accelerator ?? 'none'],
            ['itemCount', `${selected.menu.items.length}`],
          ]}
        />
      ) : (
        <p className="example-panel__note">Pick an item in the sidebar to read it back.</p>
      )}
      <PanelPreferences>
        <PreferenceSection label="Context menu">
          <PreferenceRow title="Add" subtitle="addItem · insertItem(2) · insertSeparator(3)">
            <Button size="small" variant="normal" onClick={actions.addItem}>
              Add item
            </Button>
            <Button size="small" variant="normal" onClick={actions.insertItem}>
              Insert at 2
            </Button>
            <Button size="small" variant="normal" onClick={actions.insertSeparator}>
              Separator at 3
            </Button>
          </PreferenceRow>
          <PreferenceRow title="Remove" subtitle="removeItem · removeItemAt">
            <Button size="small" variant="normal" disabled={!first} onClick={actions.removeFirst}>
              First
            </Button>
            <Button size="small" variant="normal" disabled={context.items.length <= 2} onClick={() => actions.removeAt(2)}>
              At 2
            </Button>
            <Button
              size="small"
              variant="normal"
              disabled={context.items.length === 0}
              onClick={() => actions.removeAt(context.items.length - 1)}
            >
              Last
            </Button>
          </PreferenceRow>
          <PreferenceRow title="Properties" subtitle="setLabel on the dynamic item · setState(kMixed) on the checkbox">
            <Button
              size="small"
              variant="normal"
              disabled={!context.items.some(i => i.id === ITEM.dynamicLabel)}
              onClick={actions.updateLabel}
            >
              Update label
            </Button>
            <Button
              size="small"
              variant="normal"
              disabled={!context.items.some(i => i.id === ITEM.checkbox)}
              onClick={actions.checkboxMixed}
            >
              Checkbox mixed
            </Button>
          </PreferenceRow>
          <PreferenceRow title="Submenu" subtitle={`${submenu.items.length} items · setSubmenu`}>
            <Button size="small" variant="normal" onClick={actions.addSubmenuItem}>
              Add item
            </Button>
            <Button size="small" variant="normal" disabled={!submenuItem} onClick={actions.toggleSubmenu}>
              {submenuItem?.submenu === null ? 'Attach' : 'Detach'}
            </Button>
          </PreferenceRow>
        </PreferenceSection>
        <PreferenceSection label={first ? `Icon of the first item · "${first.label}"` : 'Icon of the first item'}>
          <PreferenceRow title="Icon" subtitle="setIcon: the app's asset, a Fluent glyph drawn to a PNG, or null">
            <Button size="small" variant="normal" disabled={!first} onClick={() => actions.setIcon('asset')}>
              Asset
            </Button>
            <Button size="small" variant="normal" disabled={!first} onClick={() => actions.setIcon('widget')}>
              Widget
            </Button>
            <Button size="small" variant="plain" disabled={!first} onClick={() => actions.setIcon(null)}>
              Remove
            </Button>
          </PreferenceRow>
          <PreferenceRow title="Animate" subtitle="A new frame every 100 ms, drawn in the menu while it is open">
            <SegmentedControl<IconAnimation | ''>
              size="small"
              items={ANIMATIONS.map(a => ({ ...a, disabled: !first }))}
              value={animation?.kind ?? ''}
              onValueChange={value => actions.animate(value || null)}
            />
            <Button size="small" variant="plain" disabled={!animation} onClick={() => actions.animate(null)}>
              Stop
            </Button>
          </PreferenceRow>
        </PreferenceSection>
      </PanelPreferences>
    </Panel>
  )
}
