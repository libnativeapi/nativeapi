import type { Preview } from '@storybook/react-vite'

import { APPEARANCES, applyTheme, DEFAULT_APPEARANCE, DEFAULT_STYLE, STYLES } from '../src/styles/themes'

// The token sheet first: it is the contract the component styles read from.
import '@dazzlabs/dazzui/style.css'
import '@dazzlabs/dazzui/components.css'
import '../src/styles/globals.css'

const preview: Preview = {
  globalTypes: {
    style: {
      description: 'DazzUI theme family, desktop or Omarchy theme',
      toolbar: {
        title: 'Style',
        icon: 'paintbrush',
        items: STYLES.map(style => ({ value: style.value, title: style.title })),
        dynamicTitle: true,
      },
    },
    appearance: {
      description: 'Light or dark appearance',
      toolbar: {
        title: 'Appearance',
        icon: 'contrast',
        items: APPEARANCES.map(appearance => ({ value: appearance.value, title: appearance.title })),
        dynamicTitle: true,
      },
    },
  },
  initialGlobals: {
    style: DEFAULT_STYLE,
    appearance: DEFAULT_APPEARANCE,
  },
  decorators: [
    (Story, context) => {
      applyTheme(document.documentElement, String(context.globals.style), String(context.globals.appearance))
      return <Story />
    },
  ],
  parameters: {
    layout: 'centered',
    backgrounds: { disable: true },
    options: {
      storySort: {
        // The examples in the order of the repository's examples/, then what they share.
        order: [
          'Introduction',
          'Examples',
          [
            'Accessibility',
            ['Accessibility View'],
            'Application',
            ['Application View'],
            'Browser Tabs',
            ['Browser Tabs View'],
            'CocoaPods',
            ['CocoaPods View'],
            'Detachable Window',
            ['Detachable Window View'],
            'Display',
            ['Display View'],
            'Drag Drop',
            ['Drag Drop View'],
            'Floating Toolbar',
            ['Floating Toolbar View'],
            'Keyboard',
            ['Keyboard View'],
            'Launch At Login',
            ['Launch At Login View'],
            'Menu',
            ['Menu View'],
            'Message Dialog',
            ['Message Dialog View'],
            'Multiple Window',
            ['Multiple Window View'],
            'Preferences',
            ['Preferences View'],
            'Shaped Window',
            ['Shaped Window View'],
            'Shortcut',
            ['Shortcut View'],
            'Storage',
            ['Storage View'],
            'Tray Icon',
            ['Tray Icon View'],
            'URL Opener',
            ['URL Opener View'],
            'View',
            ['View View'],
            'Visual Effect',
            ['Visual Effect View'],
            'Window',
            ['Window View'],
            'Window Drag Areas',
            ['Window Drag Areas View'],
            'Window Title Bar',
            ['Window Title Bar View'],
          ],
          'Shared',
        ],
      },
    },
  },
}

export default preview
