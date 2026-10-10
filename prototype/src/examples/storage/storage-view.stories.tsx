import type { Meta, StoryObj } from '@storybook/react-vite'

import { windowPlatformOf } from '../../styles/themes'
import { EMPTY_ENTRIES } from './data'
import { StorageView } from './storage-view'

/**
 * The example on the desktop the Style toolbar names: four stores —
 * Preferences and SecureStorage, unscoped and scoped — in one window. The
 * Backend tab follows the desktop: a plist on macOS, the registry on
 * Windows, an XDG config file on Linux, and each one's secret store.
 */
const meta = {
  id: 'examples-storage-view',
  title: 'Examples/Storage/Storage View',
  component: StorageView,
  parameters: { layout: 'fullscreen' },
  args: { platform: 'macos' },
  argTypes: { platform: { control: false } },
  render: (args, { globals }) => {
    const platform = windowPlatformOf(String(globals.style))
    return <StorageView key={platform} {...args} platform={platform} />
  },
} satisfies Meta<typeof StorageView>

export default meta
type Story = StoryObj<typeof meta>

/** As the example starts: a few entries in each store, the default Preferences selected. */
export const Playground: Story = {}

/** Nothing stored yet: the table offers to add an entry or ten samples. */
export const EmptyStore: Story = {
  name: 'Empty Store',
  args: { initialEntries: EMPTY_ENTRIES },
}

/** Every test case run against user_settings, each with what it found. */
export const TestsRun: Story = {
  name: 'Tests Run',
  args: { initialStore: 'scoped_preferences', initialTab: 'tests', runTestsAtStart: true },
}

/** SecureStorage scoped to api_credentials: encrypted entries, and the keychain items behind them. */
export const SecureStore: Story = {
  name: 'Secure Store',
  args: { initialStore: 'scoped_secure_storage' },
}

/** Where the default Preferences live on this desktop, redrawn as entries change. */
export const Backend: Story = {
  args: { initialTab: 'backend' },
}
