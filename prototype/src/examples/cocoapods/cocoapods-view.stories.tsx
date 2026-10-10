import type { Meta, StoryObj } from '@storybook/react-vite'

import { windowPlatformOf } from '../../styles/themes'
import { CocoapodsView } from './cocoapods-view'

/**
 * The example on the desktop the Style toolbar names: a build smoke test
 * for nativeapi through CocoaPods. The six checks run as the app starts; on
 * a desktop other than macOS the window says the example is not built there.
 */
const meta = {
  id: 'examples-cocoapods-view',
  title: 'Examples/CocoaPods/CocoaPods View',
  component: CocoapodsView,
  parameters: { layout: 'fullscreen' },
  args: { platform: 'macos' },
  argTypes: { platform: { control: false } },
  render: (args, { globals }) => {
    const platform = windowPlatformOf(String(globals.style))
    return <CocoapodsView key={platform} {...args} platform={platform} />
  },
} satisfies Meta<typeof CocoapodsView>

export default meta
type Story = StoryObj<typeof meta>

/** As the example starts: the checks run one after another, then 6/6 passed. */
export const Playground: Story = {}

/** Partway through a run: three checks passed, the display check running, two waiting. */
export const Running: Story = {
  args: { freezeAfter: 3 },
}

/** DisplayManager's symbols missing from the build: that check fails with the error it threw. */
export const AFailedCheck: Story = {
  name: 'A Failed Check',
  args: { failing: ['display'] },
}

/** The Build tab: pubspec.yaml, the Podfile, the command and the build's steps. */
export const Build: Story = {
  args: { initialTab: 'build' },
}
