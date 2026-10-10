import type { Meta, StoryObj } from '@storybook/react-vite'

import { windowPlatformOf } from '../../styles/themes'
import { TrayIconView } from './tray-icon-view'

/**
 * The example on the desktop the Style toolbar names: switch Style to move it
 * between the macOS menu bar, the Windows taskbar, GNOME, KDE and Omarchy.
 * The tray icons answer the mouse — click, right-click, double-click — and
 * the window's controls act on the selected one.
 */
const meta = {
  id: 'examples-tray-icon-view',
  title: 'Examples/Tray Icon/Tray Icon View',
  component: TrayIconView,
  parameters: { layout: 'fullscreen' },
  args: { platform: 'macos' },
  argTypes: { platform: { control: false } },
  render: (args, { globals }) => {
    const platform = windowPlatformOf(String(globals.style))
    // A new platform is a new run of the example: its capabilities differ.
    return <TrayIconView key={platform} {...args} platform={platform} />
  },
} satisfies Meta<typeof TrayIconView>

export default meta
type Story = StoryObj<typeof meta>

/** One icon with the app's asset, as the example starts. */
export const Playground: Story = {}

/** Three icons in their scenes: a download with its percentage, a recording with its timer, a sync. */
export const Scenes: Story = {
  args: {
    options: { initial: [{ scene: 'download' }, { scene: 'recording', color: 'red' }, { scene: 'syncing' }] },
  },
}

/** Popup mode: the window is hidden until the icon is clicked, and hides again when it loses focus. */
export const PopupMode: Story = {
  name: 'Popup Mode',
  args: { options: { popupMode: true, initial: [{ animation: 'pulse', trigger: 'rightClicked' }] } },
}

/** The Properties tab: what the getters return, and one row per API. */
export const Properties: Story = {
  args: { initialTab: 'properties', options: { initial: [{ animation: 'spinner', title: '42%' }] } },
}

/** The Checklist tab, before anything has been clicked. */
export const Checklist: Story = {
  args: { initialTab: 'checklist' },
}

/** No icon yet: the window offers to add one. */
export const NoIcons: Story = {
  name: 'No Icons',
  args: { options: { initial: [] } },
}
