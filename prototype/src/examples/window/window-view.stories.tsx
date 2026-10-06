import type { Meta, StoryObj } from '@storybook/react-vite'

import { windowPlatformOf } from '../../styles/themes'
import { WindowView } from './window-view'

/**
 * The example on the desktop the Style toolbar names, with the two windows
 * it created. Every call acts on the selected window there and then: drag
 * a title bar, pull a corner, double-click to maximize, click a minimized
 * window in the Dock or the taskbar to restore it — the events come back at
 * the window's foot.
 */
const meta = {
  id: 'examples-window-view',
  title: 'Examples/Window/Window View',
  component: WindowView,
  parameters: { layout: 'fullscreen' },
  args: { platform: 'macos' },
  argTypes: { platform: { control: false } },
  render: (args, { globals }) => {
    const platform = windowPlatformOf(String(globals.style))
    // A new platform is a new run of the example: what it supports differs.
    return <WindowView key={platform} {...args} platform={platform} />
  },
} satisfies Meta<typeof WindowView>

export default meta
type Story = StoryObj<typeof meta>

/** As the example starts: its own window selected, Window #2 and #3 beside it. */
export const Playground: Story = {}

/** Window #2 maximized over the work area, the example's window behind it. */
export const Maximized: Story = {
  args: { options: { selected: 2, tab: 'state', patch: { 2: { maximized: true, z: 20 } } } },
}

/**
 * Window #2 at 70% opacity over an Acrylic material, Window #3 dark and
 * without a shadow: the Appearance tab. Linux has no visual effects, so
 * there the material stays None.
 */
export const Translucent: Story = {
  args: {
    options: {
      selected: 2,
      tab: 'appearance',
      patch: {
        2: { opacity: 0.7, visualEffect: 'acrylic', background: 'transparent' },
        3: { hasShadow: false, background: 'dark' },
      },
    },
  },
}

/** The Behaviour tab for Window #3, which stays on top and cannot be resized. */
export const Behaviour: Story = {
  args: {
    options: { selected: 3, tab: 'behaviour', patch: { 3: { alwaysOnTop: true, resizable: false, maximizable: false } } },
  },
}

/** The Map: the display and every window to scale, Window #3 minimized into the Dock or the taskbar. */
export const Map: Story = {
  args: { options: { view: 'map', selected: 2, patch: { 3: { minimized: true } } } },
}
