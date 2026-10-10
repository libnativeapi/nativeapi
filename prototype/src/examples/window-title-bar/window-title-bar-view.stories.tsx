import type { Meta, StoryObj } from '@storybook/react-vite'

import { windowPlatformOf } from '../../styles/themes'
import { WindowTitleBarView } from './window-title-bar-view'

/**
 * The example on the desktop the Style toolbar names: one window, its title
 * bar in each of its states. Drag the strip to move it, double-click the
 * strip to maximize; the panel reads back what the getters return.
 */
const meta = {
  id: 'examples-window-title-bar-view',
  title: 'Examples/Window Title Bar/Window Title Bar View',
  component: WindowTitleBarView,
  parameters: { layout: 'fullscreen' },
  args: { platform: 'macos' },
  argTypes: { platform: { control: false } },
  render: (args, { globals }) => {
    const platform = windowPlatformOf(String(globals.style))
    return <WindowTitleBarView key={platform} {...args} platform={platform} />
  },
} satisfies Meta<typeof WindowTitleBarView>

export default meta
type Story = StoryObj<typeof meta>

/** As the example starts: a normal title bar over the strip, content 620 × 520. */
export const Playground: Story = {}

/**
 * Content under the title bar: the bar stops drawing and the strip runs up
 * behind the buttons. macOS only; elsewhere the chip is greyed out and the
 * window stays Normal.
 */
export const UnderTitleBar: Story = {
  name: 'Under Title Bar',
  args: { options: { state: 'under' } },
}

/** Hidden: no title bar and no buttons; the strip is the only way to move the window. */
export const Hidden: Story = {
  args: { options: { state: 'hidden' } },
}

/** A normal title bar with its buttons hidden: macOS only, the getter stays true elsewhere. */
export const ButtonsHidden: Story = {
  name: 'Buttons Hidden',
  args: { options: { buttonsHidden: true } },
}
