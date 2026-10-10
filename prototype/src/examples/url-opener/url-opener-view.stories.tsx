import type { Meta, StoryObj } from '@storybook/react-vite'

import { windowPlatformOf } from '../../styles/themes'
import { UrlOpenerView } from './url-opener-view'

/**
 * The example on the desktop the Style toolbar names. Open URL hands the
 * field to the system: the app that takes it opens beside the example, and
 * the result `open` returns is read back under the field.
 */
const meta = {
  id: 'examples-url-opener-view',
  title: 'Examples/URL Opener/URL Opener View',
  component: UrlOpenerView,
  parameters: { layout: 'fullscreen' },
  args: { platform: 'macos' },
  argTypes: { platform: { control: false } },
  render: (args, { globals }) => {
    const platform = windowPlatformOf(String(globals.style))
    return <UrlOpenerView key={platform} {...args} platform={platform} />
  },
} satisfies Meta<typeof UrlOpenerView>

export default meta
type Story = StoryObj<typeof meta>

/** As the example starts: the field holds https://flutter.dev, nothing opened yet. */
export const Playground: Story = {}

/** A web page opened: the default browser comes forward with it. */
export const Opened: Story = {
  args: { openAtStart: true },
}

/** A mailto: link: the mail client composes to the address. */
export const MailLink: Story = {
  name: 'Mail Link',
  args: { initialUrl: 'mailto:hello@example.com', openAtStart: true },
}

/** No scheme: open fails with kInvalidUrlMissingScheme and nothing opens. */
export const Failed: Story = {
  args: { initialUrl: 'not a url', openAtStart: true },
}

/** Every UrlOpenErrorCode, with a URL that comes back with each. */
export const ErrorCodes: Story = {
  name: 'Error Codes',
  args: { initialTab: 'errors' },
}
