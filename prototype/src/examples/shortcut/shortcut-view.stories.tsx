import type { Meta, StoryObj } from '@storybook/react-vite'

import { osOf } from '../../components/platform'
import { windowPlatformOf } from '../../styles/themes'
import { TAKEN_SAMPLE } from './data'
import { ShortcutView } from './shortcut-view'

/**
 * The example on the desktop the Style toolbar names, with Ctrl+Shift+A and
 * Ctrl+Shift+B registered as it starts. Press them on the page: they fire.
 * Register more from the Register tab — record one, or type it — and click
 * the desktop to see which still fire with the app in the background.
 */
const meta = {
  id: 'examples-shortcut-view',
  title: 'Examples/Shortcut/Shortcut View',
  component: ShortcutView,
  parameters: { layout: 'fullscreen' },
  args: { platform: 'macos' },
  argTypes: { platform: { control: false } },
  render: (args, { globals }) => {
    const platform = windowPlatformOf(String(globals.style))
    return <ShortcutView key={platform} {...args} platform={platform} />
  },
} satisfies Meta<typeof ShortcutView>

export default meta
type Story = StoryObj<typeof meta>

/** As the example starts: Ctrl+Shift+A with a counting callback, Ctrl+Shift+B with options. */
export const Playground: Story = {}

/** A combination the system holds: isAvailable says yes, register() returns null with ShortcutRegistrationFailedEvent. */
export const RegistrationFailed: Story = {
  name: 'Registration Failed',
  args: { initialTab: 'register' },
  render: (args, { globals }) => {
    const platform = windowPlatformOf(String(globals.style))
    const attempt = { accelerator: TAKEN_SAMPLE[osOf(platform)], description: 'Quick search', scope: 'global' as const, enabled: true }
    return <ShortcutView key={platform} {...args} platform={platform} options={{ attempt }} />
  },
}

/** The app in the background, with an Application-scope Ctrl+Shift+C beside the two Global ones: only the Global ones fire. */
export const AppUnfocused: Story = {
  name: 'App Unfocused',
  args: {
    options: {
      focused: false,
      extra: [{ accelerator: 'Ctrl+Shift+C', description: 'Only while focused', scope: 'application', enabled: true }],
    },
  },
}

/** The Manager tab: processing on or off, the focus, getAll / getByScope / get, and Unregister all. */
export const Manager: Story = {
  args: { initialTab: 'manager' },
}
