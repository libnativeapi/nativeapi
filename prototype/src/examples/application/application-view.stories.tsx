import type { Meta, StoryObj } from '@storybook/react-vite'

import { windowPlatformOf } from '../../styles/themes'
import { ApplicationView } from './application-view'

/**
 * The example on the desktop the Style toolbar names. Click the desktop and
 * the app deactivates, click its window and it activates again; the badge,
 * the progress and the icon land on the Dock tile or the taskbar button, and
 * Quit plays QuitRequested → Exiting until the window is gone.
 */
const meta = {
  id: 'examples-application-view',
  title: 'Examples/Application/Application View',
  component: ApplicationView,
  parameters: { layout: 'fullscreen' },
  args: { platform: 'macos' },
  argTypes: { platform: { control: false } },
  render: (args, { globals }) => {
    const platform = windowPlatformOf(String(globals.style))
    return <ApplicationView key={platform} {...args} platform={platform} />
  },
} satisfies Meta<typeof ApplicationView>

export default meta
type Story = StoryObj<typeof meta>

/** As the example starts: Started and Activated have arrived, the menu bar and the primary window are set. */
export const Playground: Story = {}

/** A badge of 3 and 60% progress on the Dock tile, the taskbar button or the Ubuntu Dock. */
export const BadgeAndProgress: Story = {
  name: 'Badge and Progress',
  args: { initialTab: 'dock', options: { badge: '3', progress: 0.6 } },
}

/** quit(3): QuitRequested, then Exiting, and run() returns 3 — the window is gone, Relaunch starts it again. */
export const Quit: Story = {
  args: { options: { quitAtStart: 3 } },
}

/** setBrightness(Dark): the app is dark whatever the Appearance toolbar says. */
export const Dark: Story = {
  args: { initialTab: 'appearance', options: { brightness: 'dark' } },
}
