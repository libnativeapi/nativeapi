import { useState } from 'react'

import { Button, Spinner, type WindowFramePlatform } from '@dazzlabs/dazzui'

import { DesktopStage, DesktopWindow } from '../../components/desktop-stage'
import { PreferencesWindow } from './components/preferences-window'
import { StoreWindow } from './components/store-window'
import { PLAYGROUND_STORE } from './data'
import type { Store, Tab } from './types'
import { usePreferencesApp } from './use-preferences-app'
import './preferences-view.css'

export interface PreferencesViewProps {
  platform: WindowFramePlatform
  /** What earlier runs left in the store; empty for a first launch. */
  initialStore?: Store
  /** Press Clear right after starting. */
  clearAtStart?: boolean
  initialTab?: Tab
}

/**
 * The theme the window takes on for its Theme setting: the Style toolbar's
 * family in the chosen appearance, or nothing for System.
 */
function themeAttribute(theme: string | undefined) {
  if (theme !== 'light' && theme !== 'dark') return undefined
  const root = document.documentElement.getAttribute('data-theme') ?? 'studio-light'
  return `${root.replace(/-(light|dark)$/, '')}-${theme}`
}

/**
 * The preferences example: one scoped store that outlives the app. The
 * window is a run of the app — a settings form whose every change is a
 * `set` — and beside it the system's own view of the store. Relaunch app
 * quits and starts again: the launch count goes up, and everything else
 * comes back from the store.
 */
export function PreferencesView({ platform, initialStore = PLAYGROUND_STORE, clearAtStart, initialTab }: PreferencesViewProps) {
  const { store, launches, current, run, changed, actions } = usePreferencesApp({ initialStore, clearAtStart })
  const [front, setFront] = useState<'app' | 'store'>('app')
  const dataTheme = themeAttribute(store.theme)

  const hint =
    run === 'quitting' ? (
      <span className="preferences-view__hint">
        <Spinner size="small" /> Quitting Preferences Example…
      </span>
    ) : run === 'starting' ? (
      <span className="preferences-view__hint">
        <Spinner size="small" /> Starting — reading the store…
      </span>
    ) : run === 'stopped' ? (
      <span className="preferences-view__hint">
        Preferences Example is not running.
        <Button size="small" variant="filled" onClick={actions.start}>
          Open it
        </Button>
      </span>
    ) : undefined

  return (
    <DesktopStage platform={platform} appName="Preferences Example" layout="free" hint={hint}>
      {run === 'running' && (
        <DesktopWindow x={40} y={40} z={front === 'app' ? 2 : 1} onPress={() => setFront('app')}>
          {/* The Theme setting, applied to the app's own window. */}
          <div data-theme={dataTheme} className="preferences-view__themed" style={{ colorScheme: store.theme === 'dark' ? 'dark' : undefined }}>
            <PreferencesWindow
              key={current.number}
              platform={platform}
              launch={current}
              launches={launches}
              store={store}
              initialTab={initialTab}
              onSet={actions.set}
              onClear={actions.clear}
              onRelaunch={actions.relaunch}
              onQuit={actions.quit}
            />
          </div>
        </DesktopWindow>
      )}
      <DesktopWindow x={712} y={120} z={front === 'store' ? 2 : 1} onPress={() => setFront('store')}>
        <StoreWindow platform={platform} store={store} changed={changed} inactive={front !== 'store'} />
      </DesktopWindow>
    </DesktopStage>
  )
}
