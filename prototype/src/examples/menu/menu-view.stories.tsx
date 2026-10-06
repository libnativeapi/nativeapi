import type { Meta, StoryObj } from '@storybook/react-vite'

import { windowPlatformOf } from '../../styles/themes'
import { MenuView } from './menu-view'

/**
 * The example on the desktop the Style toolbar names. Right-click the area in
 * the window to open the context menu where the pointer is, drawn over the
 * desktop as the platform draws it: checkboxes and radios toggle, the
 * submenu opens on hover, disabled items take no click, and every event
 * reaches the window's foot.
 */
const meta = {
  id: 'examples-menu-view',
  title: 'Examples/Menu/Menu View',
  component: MenuView,
  parameters: { layout: 'fullscreen' },
  args: { platform: 'macos' },
  argTypes: { platform: { control: false } },
  render: (args, { globals }) => {
    const platform = windowPlatformOf(String(globals.style))
    // A new platform is a new run of the example: its backends differ.
    return <MenuView key={platform} {...args} platform={platform} />
  },
} satisfies Meta<typeof MenuView>

export default meta
type Story = StoryObj<typeof meta>

/** As the example starts: both menus built, nothing open yet. */
export const Playground: Story = {}

/** The context menu open where it was right-clicked, at Bottom Start. */
export const MenuOpen: Story = {
  name: 'Menu Open',
  args: { options: { openAtStart: true } },
}

/** The WinUI 3 backend, Windows' default: a roomier flyout. Switch Style to Windows 11 to see it; elsewhere it is unsupported. */
export const WinUI3OnWindows: Story = {
  name: 'WinUI 3 on Windows',
  args: { options: { backend: 'winUi3', openAtStart: true }, initialTab: 'settings' },
  globals: { style: 'windows11' },
}

/** After a round of edits: items added and inserted, a separator, a new label, the checkbox mixed, a spinner on the first icon. */
export const EditedMenu: Story = {
  name: 'Edited Menu',
  args: { options: { edited: true, openAtStart: true }, initialTab: 'edit' },
}
