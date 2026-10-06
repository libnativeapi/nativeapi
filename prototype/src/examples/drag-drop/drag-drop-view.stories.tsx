import type { Meta, StoryObj } from '@storybook/react-vite'

import { windowPlatformOf } from '../../styles/themes'
import { DragDropView } from './drag-drop-view'

/**
 * The example on the desktop the Style toolbar names, beside the desktop's
 * file manager and a text editor. Drag files or the selected sentence into
 * the drop area, and the note or the text card out into the other apps; the
 * window's foot logs the DropTarget and DragSource events.
 */
const meta = {
  id: 'examples-drag-drop-view',
  title: 'Examples/Drag Drop/Drag Drop View',
  component: DragDropView,
  parameters: { layout: 'fullscreen' },
  args: { platform: 'macos' },
  argTypes: { platform: { control: false } },
  render: (args, { globals }) => {
    const platform = windowPlatformOf(String(globals.style))
    // A new platform is a new run of the example: its paths differ.
    return <DragDropView key={platform} {...args} platform={platform} />
  },
} satisfies Meta<typeof DragDropView>

export default meta
type Story = StoryObj<typeof meta>

/** As the example starts: nothing dropped, nothing dragged. */
export const Playground: Story = {}

/** Two files dropped from the file manager: each by name and full path. */
export const AfterADrop: Story = {
  name: 'After a Drop',
  args: { options: { afterDrop: true } },
}

/** A file held over the drop area: Release to drop, and the pointer's position in it. A press anywhere cancels it. */
export const Hovering: Story = {
  args: { options: { hovering: true } },
}

/** The Operations tab: what each side offers and accepts, and the getters. */
export const Operations: Story = {
  args: { initialTab: 'operations' },
}
