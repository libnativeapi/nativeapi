import type { Meta, StoryObj } from '@storybook/react-vite'

import { windowPlatformOf } from '../../styles/themes'
import { BrowserTabsView } from './browser-tabs-view'

/**
 * The example on the desktop the Style toolbar names: two browser windows
 * whose tab strips are their chrome. Drag tabs with the mouse — reorder,
 * tear off, merge, move a window by its strip — and watch the
 * WindowDragSession in the HUD. Escape cancels a drag.
 */
const meta = {
  id: 'examples-browser-tabs-view',
  title: 'Examples/Browser Tabs/Browser Tabs View',
  component: BrowserTabsView,
  parameters: { layout: 'fullscreen' },
  args: { platform: 'macos' },
  argTypes: { platform: { control: false } },
  render: (args, { globals }) => {
    const platform = windowPlatformOf(String(globals.style))
    // A new platform is a new run of the example: the strip's insets differ.
    return <BrowserTabsView key={platform} {...args} platform={platform} />
  },
} satisfies Meta<typeof BrowserTabsView>

export default meta
type Story = StoryObj<typeof meta>

/** As the example starts: two windows side by side, with Tab 1–4 and Tab 5–6. */
export const Playground: Story = {}

/** One wide window with ten tabs, squeezed towards their minimum width. */
export const ManyTabs: Story = {
  name: 'Many Tabs',
  args: { scene: 'manyTabs' },
}

/** After a tear-off: Tab 6 in a window of its own, moved between windows once. */
export const TornOff: Story = {
  name: 'Torn Off',
  args: { scene: 'tornOff' },
}

/** Hyprland on Wayland: no drag session, so tabs only reorder within their strip. */
export const Wayland: Story = {
  globals: { style: 'omarchy-tokyo-night' },
}
