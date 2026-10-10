import type { Meta, StoryObj } from '@storybook/react-vite'

import { windowPlatformOf } from '../../styles/themes'
import { DisplayView } from './display-view'

/**
 * The example on the desktop the Style toolbar names, which stands for the
 * primary display. The arrangement draws every display to scale with this
 * window and the cursor over it, live; Simulate plugs displays in and out,
 * turns and rescales them, and the `DisplayManager` events update the window.
 */
const meta = {
  id: 'examples-display-view',
  title: 'Examples/Display/Display View',
  component: DisplayView,
  parameters: { layout: 'fullscreen' },
  args: { platform: 'macos' },
  argTypes: { platform: { control: false } },
  render: (args, { globals }) => {
    const platform = windowPlatformOf(String(globals.style))
    // A new platform is a new run of the example: its displays differ.
    return <DisplayView key={platform} {...args} platform={platform} />
  },
} satisfies Meta<typeof DisplayView>

export default meta
type Story = StoryObj<typeof meta>

/** A built-in panel as the primary and an external display to its right, offset upwards. */
export const Playground: Story = {}

/** One display only: Unplug has nothing to take, the table has one row. */
export const SingleDisplay: Story = {
  name: 'Single Display',
  args: { scene: 'single' },
}

/** Three displays, one on each side of the primary, in the table. */
export const ThreeDisplays: Story = {
  name: 'Three Displays',
  args: { scene: 'three', initialTab: 'table' },
}

/** The external display turned to portrait: kPortrait, its size swapped. */
export const Rotated: Story = {
  args: { scene: 'rotated' },
}

/** The selected display's getters: basic, hardware and geometry. */
export const Details: Story = {
  args: { initialTab: 'details' },
}
