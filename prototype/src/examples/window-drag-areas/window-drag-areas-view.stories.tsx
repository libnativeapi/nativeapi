import type { Meta, StoryObj } from '@storybook/react-vite'

import { windowPlatformOf } from '../../styles/themes'
import { WindowDragAreasView } from './window-drag-areas-view'

/**
 * The example on the desktop the Style toolbar names: a window with no
 * native title bar, moved by its coloured bar and resized by the tinted
 * frame inside its edge. The HUD shows the call each gesture made.
 */
const meta = {
  id: 'examples-window-drag-areas-view',
  title: 'Examples/Window Drag Areas/Window Drag Areas View',
  component: WindowDragAreasView,
  parameters: { layout: 'fullscreen' },
  args: { platform: 'macos' },
  argTypes: { platform: { control: false } },
  render: (args, { globals }) => {
    const platform = windowPlatformOf(String(globals.style))
    return <WindowDragAreasView key={platform} {...args} platform={platform} />
  },
} satisfies Meta<typeof WindowDragAreasView>

export default meta
type Story = StoryObj<typeof meta>

/** As the example starts: 720 × 480, all eight handles. */
export const Playground: Story = {}

/** Edges: right and bottom — three of the eight handles, the rest of the frame inert. */
export const ThreeEdges: Story = {
  name: 'Three Edges',
  args: { options: { limited: true } },
}

/** Maximized from a double-click on the bar: the handles still resize it, a drag restores it. */
export const Maximized: Story = {
  args: { options: { maximized: true } },
}
