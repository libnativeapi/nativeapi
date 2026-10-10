import type { Meta, StoryObj } from '@storybook/react-vite'

import { windowPlatformOf } from '../../styles/themes'
import { ShapedWindowView } from './shaped-window-view'

/**
 * The example on the desktop the Style toolbar names. Pick a silhouette in
 * "Window shapes" and the preview window morphs into it; drag it by its
 * handle, change its size and its contour shadow. On Linux the clip is drawn
 * by the app and core takes the input region (setInputShape).
 */
const meta = {
  id: 'examples-shaped-window-view',
  title: 'Examples/Shaped Window/Shaped Window View',
  component: ShapedWindowView,
  parameters: { layout: 'fullscreen' },
  args: { platform: 'macos' },
  argTypes: { platform: { control: false } },
  render: (args, { globals }) => {
    const platform = windowPlatformOf(String(globals.style))
    // A new platform is a new run of the example: Linux takes another path.
    return <ShapedWindowView key={platform} {...args} platform={platform} />
  },
} satisfies Meta<typeof ShapedWindowView>

export default meta
type Story = StoryObj<typeof meta>

/** As the example starts: a circle in Aurora with the Soft shadow. */
export const Playground: Story = {}

/** A star in Sunset, glowing: the shadow follows every point. */
export const StarWithGlow: Story = {
  name: 'Star With Glow',
  args: { initialShape: 'star', initialPreset: 'Glow' },
}

/** The Adjust view: every WindowShadow parameter, and what the getters return. */
export const AdjustShadow: Story = {
  name: 'Adjust Shadow',
  args: { initialShape: 'heart', initialPreset: 'Float', editingShadow: true },
}

/** setShape(null): the preview is a plain rectangle again, with no title bar. */
export const RectangleRestored: Story = {
  name: 'Rectangle Restored',
  args: { restored: true },
}
