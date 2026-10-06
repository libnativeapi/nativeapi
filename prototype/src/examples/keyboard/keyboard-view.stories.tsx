import type { Meta, StoryObj } from '@storybook/react-vite'

import { windowPlatformOf } from '../../styles/themes'
import { KeyboardView } from './keyboard-view'

/**
 * The example on the desktop the Style toolbar names. Start the monitor and
 * press real keys: each arrives with the platform's raw keycode, and the
 * modifier mask lights its flags. Click the Notes window and keep typing —
 * a global monitor still sees every key.
 */
const meta = {
  id: 'examples-keyboard-view',
  title: 'Examples/Keyboard/Keyboard View',
  component: KeyboardView,
  parameters: { layout: 'fullscreen' },
  args: { platform: 'macos' },
  argTypes: { platform: { control: false } },
  render: (args, { globals }) => {
    const platform = windowPlatformOf(String(globals.style))
    return <KeyboardView key={platform} {...args} platform={platform} />
  },
} satisfies Meta<typeof KeyboardView>

export default meta
type Story = StoryObj<typeof meta>

/** As the example opens: a monitor made, not started. */
export const Playground: Story = {}

/** start() has run: press keys on the page and watch them arrive. */
export const Monitoring: Story = {
  args: { options: { startAtStart: true } },
}

/** On a Mac that has not trusted the example: start() leaves isMonitoring false until Accessibility is granted. */
export const PermissionNeeded: Story = {
  name: 'Permission Needed',
  args: { options: { trusted: false, startAtStart: true } },
}

/** Monitoring with the Notes window in front: what is typed there still reaches the monitor. */
export const Unfocused: Story = {
  args: { options: { startAtStart: true, focus: 'notes' } },
}

/** Every key's raw code on macOS, Windows and X11, this platform's column marked. */
export const Keycodes: Story = {
  args: { initialTab: 'keycodes' },
}
