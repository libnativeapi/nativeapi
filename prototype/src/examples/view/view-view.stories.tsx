import type { Meta, StoryObj } from '@storybook/react-vite'

import { windowPlatformOf } from '../../styles/themes'
import { SAMPLE_TASKS } from './data'
import { ViewView } from './view-view'

/**
 * The example on the desktop the Style toolbar names: a workbench of the
 * platform's own controls — AppKit, Win32, GTK 3 — built from a plain Dart
 * program, and the View Inspector beside it reading the tree back. Pick a
 * view in the inspector to outline it; drag the workbench's corner to make
 * every flexible view lay itself out again.
 */
const meta = {
  id: 'examples-view-view',
  title: 'Examples/View/View View',
  component: ViewView,
  parameters: { layout: 'fullscreen' },
  args: { platform: 'macos' },
  argTypes: { platform: { control: false } },
  render: (args, { globals }) => {
    const platform = windowPlatformOf(String(globals.style))
    return <ViewView key={platform} {...args} platform={platform} />
  },
} satisfies Meta<typeof ViewView>

export default meta
type Story = StoryObj<typeof meta>

/** As the example starts with the inspector open: an empty form, no tasks, the playground in a row. */
export const Playground: Story = {
  args: { start: { inspectorOpen: true, selected: 'playground.stage' } },
}

/** Signed in as ada with three tasks, one done; the inspector outlines the sign-in button. */
export const SignedInWithTasks: Story = {
  name: 'Signed In With Tasks',
  args: {
    start: {
      inspectorOpen: true,
      signedInAs: 'ada',
      tasks: SAMPLE_TASKS,
      notes: 'Ship the view example before Friday.',
      selected: 'signIn.submit',
    },
  },
}

/** The workbench alone, as `dart run` opens it: the footer offers to open the inspector. */
export const InspectorClosed: Story = {
  name: 'Inspector Closed',
  args: { start: { inspectorOpen: false } },
}

/**
 * The playground's stage in a column, boxes centred, the window dragged
 * taller: fixed A and D keep their 40 px, B and C share the rest 1 : 2.
 */
export const LayoutColumn: Story = {
  name: 'Layout Column',
  args: {
    start: { inspectorOpen: true, layout: 'column', alignment: 'center', windowHeight: 720, selected: 'playground.C' },
  },
}
