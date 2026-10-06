import type { Meta, StoryObj } from '@storybook/react-vite'

import { windowPlatformOf } from '../../styles/themes'
import { FloatingToolbarView } from './floating-toolbar-view'

/**
 * The example on the desktop the Style toolbar names. Drag the main window
 * by its title bar: the attached toolbar follows. Detach, hide or show it
 * from the main window, minimize the main window from its title bar, and
 * pick a colour or stamp from the pill — both windows share one model.
 */
const meta = {
  id: 'examples-floating-toolbar-view',
  title: 'Examples/Floating Toolbar/Floating Toolbar View',
  component: FloatingToolbarView,
  parameters: { layout: 'fullscreen' },
  args: { platform: 'macos' },
  argTypes: { platform: { control: false } },
  render: (args, { globals }) => {
    const platform = windowPlatformOf(String(globals.style))
    // A new platform is a new run of the example: Wayland cannot place windows.
    return <FloatingToolbarView key={platform} {...args} platform={platform} />
  },
} satisfies Meta<typeof FloatingToolbarView>

export default meta
type Story = StoryObj<typeof meta>

/** As the example starts: the toolbar attached, centred 10 px above the main window. */
export const Playground: Story = {}

/** Detached, then the main window moved: the toolbar stayed where it was. */
export const Detached: Story = {
  args: { attached: false, mainOffset: { x: 120, y: 90 } },
}

/** The toolbar hidden; Show toolbar brings it back with showInactive, the main window stays key. */
export const Hidden: Story = {
  args: { toolbarVisible: false },
}

/** Hyprland: the toolbar stands where the compositor put it, and the pill drags itself. */
export const Wayland: Story = {
  globals: { style: 'omarchy-tokyo-night' },
}
