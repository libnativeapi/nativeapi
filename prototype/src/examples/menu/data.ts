import type { IconAnimation, ItemType, MenuItemModel, MenuModel, Placement } from './types'

/** The menus the example creates, by `getId()`. */
export const CONTEXT_MENU = 1
export const POSITIONING_MENU = 2
export const SUBMENU = 3
/** The menu Issue #4 builds on demand: a checked item and a disabled one. */
export const ISSUE_MENU = 4

/** The items whose handlers the example keeps, by the id they are created with. */
export const ITEM = {
  normal: 1,
  checkbox: 3,
  radio1: 4,
  radio2: 5,
  radio3: 6,
  disabled: 7,
  disabledCheckbox: 8,
  dynamicLabel: 10,
  tooltip: 11,
  submenu: 13,
} as const

export const INITIAL_LABEL = 'Dynamic Label Item'
export const SPECIAL_LABEL = 'Special: 中文 日本語 🎉 @#$%'

const item = (id: number, type: ItemType, label: string, extra: Partial<MenuItemModel> = {}): MenuItemModel => ({
  id,
  type,
  label,
  state: 'unchecked',
  enabled: true,
  tooltip: null,
  radioGroup: -1,
  icon: null,
  accelerator: null,
  submenu: null,
  ...extra,
})

const separator = (id: number) => item(id, 'separator', '')

/**
 * The example's menus as it builds them (`_setupContextMenu`,
 * `_setupPositioningMenu`): every item kind in the context menu — normal,
 * checkbox, a radio group, disabled ones, a label that changes, a tooltip, a
 * submenu, labels outside ASCII — and two plain items in the positioning
 * menu. Ids follow creation order, separators included.
 */
export function initialMenus(accelerator: string): Record<number, MenuModel> {
  return {
    [CONTEXT_MENU]: {
      id: CONTEXT_MENU,
      name: 'Context menu',
      items: [
        item(ITEM.normal, 'normal', 'Normal Menu Item', { accelerator }),
        separator(2),
        item(ITEM.checkbox, 'checkbox', 'Checkbox Item'),
        item(ITEM.radio1, 'radio', 'Radio Option 1', { radioGroup: 1, state: 'checked' }),
        item(ITEM.radio2, 'radio', 'Radio Option 2', { radioGroup: 1 }),
        item(ITEM.radio3, 'radio', 'Radio Option 3', { radioGroup: 1 }),
        item(ITEM.disabled, 'normal', 'Disabled Item', { enabled: false }),
        item(ITEM.disabledCheckbox, 'checkbox', 'Disabled Checkbox', { enabled: false, state: 'checked' }),
        separator(9),
        item(ITEM.dynamicLabel, 'normal', INITIAL_LABEL),
        item(ITEM.tooltip, 'normal', 'Item with Tooltip', { tooltip: 'This is a helpful tooltip message' }),
        separator(12),
        item(ITEM.submenu, 'submenu', 'Submenu', { submenu: SUBMENU }),
        separator(14),
        item(15, 'normal', SPECIAL_LABEL),
      ],
    },
    [SUBMENU]: {
      id: SUBMENU,
      name: 'Submenu',
      items: [
        item(16, 'normal', 'Submenu Item 1'),
        item(17, 'normal', 'Submenu Item 2'),
        separator(18),
        item(19, 'normal', 'Submenu Item 3'),
      ],
    },
    [POSITIONING_MENU]: {
      id: POSITIONING_MENU,
      name: 'Positioning menu',
      items: [item(20, 'normal', 'Positioning Menu Item 1'), item(21, 'normal', 'Positioning Menu Item 2')],
    },
  }
}

/** The first id free after the initial items. */
export const FIRST_FREE_ITEM = 22

/** The menu Issue #4 opens: a checked checkbox and a disabled item must look it on Windows. */
export function issueMenu(firstId: number): MenuModel {
  return {
    id: ISSUE_MENU,
    name: 'Issue #4 menu',
    items: [
      item(firstId, 'checkbox', 'Checkable', { state: 'checked' }),
      item(firstId + 1, 'normal', 'Disabled', { enabled: false }),
      separator(firstId + 2),
      item(firstId + 3, 'normal', 'Close Menu'),
    ],
  }
}

/** The placement pad, side by side: what each `Placement` is called. */
export const PLACEMENT_NAMES: Record<Placement, string> = {
  top: 'Top',
  topStart: 'Top Start',
  topEnd: 'Top End',
  right: 'Right',
  rightStart: 'Right Start',
  rightEnd: 'Right End',
  bottom: 'Bottom',
  bottomStart: 'Bottom Start',
  bottomEnd: 'Bottom End',
  left: 'Left',
  leftStart: 'Left Start',
  leftEnd: 'Left End',
}

export const ANIMATIONS: readonly { value: IconAnimation; label: string }[] = [
  { value: 'spinner', label: 'Spinner' },
  { value: 'pulse', label: 'Pulse' },
  { value: 'blink', label: 'Blink' },
  { value: 'progress', label: 'Progress' },
  { value: 'wave', label: 'Wave' },
  { value: 'rotate', label: 'Rotate' },
]

/** The Open at buttons: screen points for the positioning menu, edges included. */
export const OPEN_POINTS: readonly { label: string; x: number; y: number; note?: string }[] = [
  { label: 'Pos (100, 100)', x: 100, y: 100 },
  { label: 'Pos (300, 200)', x: 300, y: 200 },
  { label: 'Top-left edge', x: 10, y: 10, note: 'top-left' },
  { label: 'Bottom-right edge', x: 1500, y: 900, note: 'bottom-right' },
]

export const TYPE_NAMES: Record<ItemType, string> = {
  normal: 'kNormal',
  checkbox: 'kCheckbox',
  radio: 'kRadio',
  separator: 'kSeparator',
  submenu: 'kSubmenu',
}

export const STATE_NAMES = { unchecked: 'kUnchecked', checked: 'kChecked', mixed: 'kMixed' } as const
