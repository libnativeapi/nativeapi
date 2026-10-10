import { Badge, Button, Callout, SectionLabel, SegmentedControl, Switch } from '@dazzlabs/dazzui'

import { EventBar } from '../../../components/event-bar'
import { CREATION_ORDER, WINDOWS } from '../data'
import type { HooksSimulation } from '../simulate-hooks'
import type { HideHook } from '../types'
import './hook-panel.css'

export interface HookPanelProps {
  hooks: HooksSimulation
  macos: boolean
}

/**
 * Laid over the desktop beside the block: the hooks to install, a show and a
 * hide for each window (each goes through the hooks), what the hooks saw, and
 * the calls and events so far.
 */
export function HookPanel({ hooks, macos }: HookPanelProps) {
  return (
    <div className="hook-panel">
      <SectionLabel>WindowManager hooks</SectionLabel>
      <label className="hook-panel__row">
        <span className="hook-panel__label">
          Will-show hook installed
          <small>Without it, windows show at their own 800 × 600 cascade</small>
        </span>
        <Switch size="small" checked={hooks.showHook} onCheckedChange={on => hooks.setShowHook(on)} />
      </label>
      <div className="hook-panel__field">
        <span className="hook-panel__label">
          Will-hide hook
          <small>
            {hooks.hideHook === 'swallow'
              ? hooks.replaces
                ? 'Logs only, as in the Flutter example: the hide never happens'
                : 'Logs only; GTK only reports the hide, so it happens anyway'
              : hooks.hideHook === 'pass'
                ? 'Logs, then callOriginalHide, as in the Deno and GPUI examples'
                : 'No hook: hide() hides'}
          </small>
        </span>
        <SegmentedControl<HideHook>
          size="small"
          stretch
          items={[
            { value: 'none', label: 'None' },
            { value: 'swallow', label: 'Logs only' },
            { value: 'pass', label: 'Passes on' },
          ]}
          value={hooks.hideHook}
          onValueChange={mode => hooks.setHideHook(mode)}
        />
      </div>

      <SectionLabel>Windows</SectionLabel>
      <ul className="hook-panel__windows">
        {[...CREATION_ORDER].reverse().map(key => {
          const window = hooks.windows[key]
          return (
            <li key={key}>
              <span className="hook-panel__label">
                {WINDOWS[key].title}
                <small>#{window.id} · {WINDOWS[key].slot}</small>
              </span>
              <Badge size="small" variant="tinted" tint={window.visible ? 'success' : 'neutral'}>
                {window.visible ? 'Visible' : 'Hidden'}
              </Badge>
              <Button
                size="small"
                variant="normal"
                onClick={() => (window.visible ? hooks.hide(key) : hooks.show(key))}
              >
                {window.visible ? 'Hide' : 'Show'}
              </Button>
            </li>
          )
        })}
      </ul>

      <SectionLabel>Hook log</SectionLabel>
      <ol className="hook-panel__log">
        {hooks.hookLog.length === 0 ? (
          <li>Nothing yet</li>
        ) : (
          hooks.hookLog.slice(0, 6).map((line, i) => <li key={hooks.hookLog.length - i}>{line}</li>)
        )}
      </ol>

      {macos && (
        <p className="hook-panel__note">
          On macOS the hook fires twice per show: setIsVisible: goes through makeKeyAndOrderFront: too. The layout is
          the same both times, so it is harmless.
        </p>
      )}
      {hooks.wayland && (
        <Callout size="small" tint="warning" title="Wayland: the hook cannot place windows">
          The hooks fire, but Hyprland tiles the windows and bounds reads x and y as 0.
        </Callout>
      )}
      <EventBar lastEvent={hooks.lastEvent} log={hooks.log} onClear={() => hooks.clearLog()} />
    </div>
  )
}
