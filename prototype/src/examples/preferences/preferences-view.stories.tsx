import type { Meta, StoryObj } from '@storybook/react-vite'

import { windowPlatformOf } from '../../styles/themes'
import { CLEARED_STORE, PLAYGROUND_STORE } from './data'
import { PreferencesView } from './preferences-view'

/**
 * The example on the desktop the Style toolbar names: a settings form bound
 * to one scoped Preferences store, and the system's view of that store
 * beside it — `defaults read` on macOS, Registry Editor on Windows, the XDG
 * config file on Linux. Relaunch app shows the values surviving a restart.
 */
const meta = {
  id: 'examples-preferences-view',
  title: 'Examples/Preferences/Preferences View',
  component: PreferencesView,
  parameters: { layout: 'fullscreen' },
  args: { platform: 'macos' },
  argTypes: { platform: { control: false } },
  render: (args, { globals }) => {
    const platform = windowPlatformOf(String(globals.style))
    return <PreferencesView key={platform} {...args} platform={platform} />
  },
} satisfies Meta<typeof PreferencesView>

export default meta
type Story = StoryObj<typeof meta>

/** The fourth launch: the greeting, the dark theme and the tips setting all came back from the store. */
export const Playground: Story = {}

/** Nothing stored yet: every get returns its default, and the launch count starts at 1. */
export const FirstLaunch: Story = {
  name: 'First Launch',
  args: { initialStore: CLEARED_STORE },
}

/** Clear pressed: the store is empty, the form is back on the defaults, and Read back shows it. */
export const AfterClear: Story = {
  name: 'After Clear',
  args: { initialStore: PLAYGROUND_STORE, clearAtStart: true, initialTab: 'readback' },
}
