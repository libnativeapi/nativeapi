import type { Meta, StoryObj } from '@storybook/react-vite'

import { windowPlatformOf } from '../../styles/themes'
import { VisualEffectView } from './visual-effect-view'

/**
 * The example on the desktop the Style toolbar names. Pick a VisualEffect in
 * the card and the window's background becomes that material; the Backdrop
 * switch puts a plain red window behind it to see through. Drag either window
 * by its title bar across the wallpaper's shapes.
 */
const meta = {
  id: 'examples-visual-effect-view',
  title: 'Examples/Visual Effect/Visual Effect View',
  component: VisualEffectView,
  parameters: { layout: 'fullscreen' },
  args: { platform: 'macos' },
  argTypes: { platform: { control: false } },
  render: (args, { globals }) => {
    const platform = windowPlatformOf(String(globals.style))
    // A new platform is a new run of the example: what it supports differs.
    return <VisualEffectView key={platform} {...args} platform={platform} />
  },
} satisfies Meta<typeof VisualEffectView>

export default meta
type Story = StoryObj<typeof meta>

/** As the example starts: no effect, the window paints its canvas. */
export const Playground: Story = {}

/** The red backdrop behind the window, seen through acrylic. Try mica: it ignores the backdrop. */
export const BackdropOn: Story = {
  name: 'Backdrop On',
  args: { backdrop: true, initialEffect: 'acrylic' },
}

/** Linux: every effect but none is greyed out, and setVisualEffect returns false. */
export const Unsupported: Story = {
  args: { initialEffect: 'blur' },
  globals: { style: 'ubuntu' },
}

/** The dark appearance, on the hud material over the backdrop. */
export const Dark: Story = {
  args: { backdrop: true, initialEffect: 'hud' },
  globals: { appearance: 'dark' },
}
