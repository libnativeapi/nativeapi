import type { Meta, StoryObj } from '@storybook/react-vite'

import { windowPlatformOf } from '../../styles/themes'
import { MultipleWindowView } from './multiple-window-view'

/**
 * The example on the desktop the Style toolbar names: three windows that a
 * will-show hook places in a block 60% of the primary display's work area.
 * Show and hide them from the panel, toggle the hooks, or press the primary
 * window's Resize to see the hook put it back.
 */
const meta = {
  id: 'examples-multiple-window-view',
  title: 'Examples/Multiple Window/Multiple Window View',
  component: MultipleWindowView,
  parameters: { layout: 'fullscreen' },
  args: { platform: 'macos' },
  argTypes: { platform: { control: false } },
  render: (args, { globals }) => {
    const platform = windowPlatformOf(String(globals.style))
    return <MultipleWindowView key={platform} {...args} platform={platform} />
  },
} satisfies Meta<typeof MultipleWindowView>

export default meta
type Story = StoryObj<typeof meta>

/** As the example starts: both hooks installed, the three windows shown into their slots. */
export const Playground: Story = {}

/** No will-show hook: each window shows where the platform put it, 800 × 600, cascaded. */
export const WithoutHook: Story = {
  name: 'Without Hook',
  args: { showHook: false },
}

/** Resize to 1000 × 1000 pressed: the primary window grows, then the hook snaps it back into its slot. */
export const AfterResize: Story = {
  name: 'After Resize',
  args: { resizeAtStart: true },
}
