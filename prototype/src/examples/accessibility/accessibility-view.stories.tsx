import type { Meta, StoryObj } from '@storybook/react-vite'

import { windowPlatformOf } from '../../styles/themes'
import { AccessibilityView } from './accessibility-view'

/**
 * The example on the desktop the Style toolbar names. On macOS, Request
 * access raises the system's prompt and opens System Settings on the
 * Accessibility list; switch the example on there, then click its window:
 * it re-checks on activation. On Windows and Linux there is nothing to grant.
 */
const meta = {
  id: 'examples-accessibility-view',
  title: 'Examples/Accessibility/Accessibility View',
  component: AccessibilityView,
  parameters: { layout: 'fullscreen' },
  args: { platform: 'macos' },
  argTypes: { platform: { control: false } },
  render: (args, { globals }) => {
    const platform = windowPlatformOf(String(globals.style))
    return <AccessibilityView key={platform} {...args} platform={platform} />
  },
} satisfies Meta<typeof AccessibilityView>

export default meta
type Story = StoryObj<typeof meta>

/** As the example starts on a Mac that has not trusted it: isEnabled is false, the keyboard monitor would be blocked. */
export const Playground: Story = {}

/** enable() was called: System Settings is open on Privacy & Security ▸ Accessibility, the example listed and switched off. */
export const SettingsOpen: Story = {
  name: 'Settings Open',
  args: { options: { settingsOpen: true } },
}

/** The user allowed it: isEnabled is true, and what needs it is ready. */
export const Trusted: Story = {
  args: { options: { granted: true } },
}

/** What AccessibilityManager does on each platform. */
export const Platforms: Story = {
  args: { initialTab: 'platforms' },
}
