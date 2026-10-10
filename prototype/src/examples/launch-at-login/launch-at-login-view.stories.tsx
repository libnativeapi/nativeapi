import type { Meta, StoryObj } from '@storybook/react-vite'

import { windowPlatformOf } from '../../styles/themes'
import { LaunchAtLoginView } from './launch-at-login-view'

/**
 * The example on the desktop the Style toolbar names, beside the system's
 * own view of the registration: Login Items on macOS, Task Manager and the
 * Run key on Windows, the autostart file on Linux. Log out and back in
 * plays the next login.
 */
const meta = {
  id: 'examples-launch-at-login-view',
  title: 'Examples/Launch At Login/Launch At Login View',
  component: LaunchAtLoginView,
  parameters: { layout: 'fullscreen' },
  args: { platform: 'macos' },
  argTypes: { platform: { control: false } },
  render: (args, { globals }) => {
    const platform = windowPlatformOf(String(globals.style))
    return <LaunchAtLoginView key={platform} {...args} platform={platform} />
  },
} satisfies Meta<typeof LaunchAtLoginView>

export default meta
type Story = StoryObj<typeof meta>

/** As the example starts: supported, not registered yet. */
export const Playground: Story = {}

/** Registered, with --minimized recorded where the platform keeps arguments: the system lists the app. */
export const Enabled: Story = {
  args: { initialEnabled: true, initialArguments: ['--minimized'] },
}

/** On Windows, the Run key in Registry Editor and the StartupApproved flag beside it. */
export const WindowsRegistryView: Story = {
  name: 'Windows Registry View',
  globals: { style: 'windows11' },
  args: { initialEnabled: true, initialArguments: ['--minimized'], initialWindowsView: 'registry' },
}

/** isSupported() is false, as on Android, iOS and OpenHarmony: every control is off. */
export const Unsupported: Story = {
  args: { supported: false },
}
