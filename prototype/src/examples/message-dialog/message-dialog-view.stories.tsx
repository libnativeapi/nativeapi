import type { Meta, StoryObj } from '@storybook/react-vite'

import { windowPlatformOf } from '../../styles/themes'
import { MessageDialogView } from './message-dialog-view'

/**
 * The example on the desktop the Style toolbar names. Compose a dialog, or
 * pick a preset, and Open it: it is drawn as the platform draws it, a modal
 * one refuses clicks on the example's window, and its buttons close it —
 * the Result tab reads back how.
 */
const meta = {
  id: 'examples-message-dialog-view',
  title: 'Examples/Message Dialog/Message Dialog View',
  component: MessageDialogView,
  parameters: { layout: 'fullscreen' },
  args: { platform: 'macos' },
  argTypes: { platform: { control: false } },
  render: (args, { globals }) => {
    const platform = windowPlatformOf(String(globals.style))
    return <MessageDialogView key={platform} {...args} platform={platform} />
  },
} satisfies Meta<typeof MessageDialogView>

export default meta
type Story = StoryObj<typeof meta>

/** As the example starts: "Update Available", application-modal, nothing opened yet. */
export const Playground: Story = {}

/** The dialog open and application-modal: the example's window refuses clicks until a button closes it. */
export const OpenAlert: Story = {
  name: 'Open Alert',
  args: { options: { openAtStart: true } },
}

/** Window modality on macOS: an NSAlert sheet hung from the example's window. */
export const WindowSheet: Story = {
  name: 'Window Sheet',
  args: { options: { preset: 'delete', openAtStart: true } },
}

/** WinUI 3's ContentDialog, modeless in its own window, with every extended control: drag the progress in the Extended tab and it follows. */
export const WindowsExtended: Story = {
  name: 'Windows Extended',
  args: {
    initialTab: 'extended',
    options: { preset: 'signIn', config: { checkboxLabel: 'Remember this device', checked: true, progress: 0.4, modality: 'none', parent: false }, openAtStart: true },
  },
  globals: { style: 'windows11' },
}
