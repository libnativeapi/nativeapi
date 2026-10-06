import type { Meta, StoryObj } from '@storybook/react-vite'

import { windowPlatformOf } from '../../styles/themes'
import { DetachableWindowView } from './detachable-window-view'

/**
 * The example on the desktop the Style toolbar names: two main windows with
 * panel slots, and two panels that move between them. Drag a panel header
 * 8px to tear it off, over an empty slot to dock it; Escape cancels a drag.
 * The WindowDragSession and its calls are in the HUD.
 */
const meta = {
  id: 'examples-detachable-window-view',
  title: 'Examples/Detachable Window/Detachable Window View',
  component: DetachableWindowView,
  parameters: { layout: 'fullscreen' },
  args: { platform: 'macos' },
  argTypes: { platform: { control: false } },
  render: (args, { globals }) => {
    const platform = windowPlatformOf(String(globals.style))
    return <DetachableWindowView key={platform} {...args} platform={platform} />
  },
} satisfies Meta<typeof DetachableWindowView>

export default meta
type Story = StoryObj<typeof meta>

/** As the example starts: the Inspector in Window A's sidebar, the Stopwatch in its bottom panel. */
export const Playground: Story = {}

/** The Stopwatch in a window of its own, the size of the bottom panel it left, still running. */
export const FloatingPanel: Story = {
  name: 'Floating Panel',
  args: { layout: 'floating' },
}

/** Both panels docked in Window B: the Inspector wide in the sidebar, the Stopwatch in the top strip. */
export const AllInWindowB: Story = {
  name: 'All In Window B',
  args: { layout: 'windowB' },
}

/** Hyprland on Wayland: a drag ends at once, so panels move with Open in a window and Dock back. */
export const Wayland: Story = {
  globals: { style: 'omarchy-tokyo-night' },
}
