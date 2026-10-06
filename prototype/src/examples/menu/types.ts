/** `MenuItemType` (`menu.h`). */
export type ItemType = 'normal' | 'checkbox' | 'radio' | 'separator' | 'submenu'

/** `MenuItemState`: mixed is a checkbox's indeterminate state. */
export type ItemState = 'unchecked' | 'checked' | 'mixed'

/** The still images the example hands to `MenuItem.setIcon`: its asset, or a Fluent glyph drawn to a PNG. */
export type ItemIcon = 'asset' | 'widget'

/** What `AnimatedIconGenerator` plays on the first item's icon (`animated_icon_generator.dart`). */
export type IconAnimation = 'spinner' | 'pulse' | 'blink' | 'progress' | 'wave' | 'rotate'

/** `MenuBackend`; WinUI 3 exists on Windows only. */
export type MenuBackend = 'native' | 'winUi3'

/** The example's theme choice, handed to `Application.setBrightness`. */
export type ThemeChoice = 'system' | 'light' | 'dark'

/** `Placement` (`placement.h`): a side of the anchor, and where along it the menu aligns. */
export type Placement =
  | 'top'
  | 'topStart'
  | 'topEnd'
  | 'right'
  | 'rightStart'
  | 'rightEnd'
  | 'bottom'
  | 'bottomStart'
  | 'bottomEnd'
  | 'left'
  | 'leftStart'
  | 'leftEnd'

/** One `MenuItem`, as its getters would read it. */
export interface MenuItemModel {
  /** `getId()`. */
  id: number
  type: ItemType
  label: string
  state: ItemState
  enabled: boolean
  tooltip: string | null
  /** `getRadioGroup()`: -1 when the item is in none. */
  radioGroup: number
  icon: ItemIcon | null
  /** `getAccelerator()`, as the platform draws it. */
  accelerator: string | null
  /** `getSubmenu()`: the id of the attached `Menu`, or null. */
  submenu: number | null
}

/** One `Menu`: its id and its items, separators included, as `getItemAt` indexes them. */
export interface MenuModel {
  id: number
  /** How the example refers to it. */
  name: string
  items: MenuItemModel[]
}

/** A menu open on the screen, at a point in screen coordinates. */
export interface OpenMenu {
  menuId: number
  x: number
  y: number
  placement: Placement
}

export type Tab = 'open' | 'edit' | 'settings'
