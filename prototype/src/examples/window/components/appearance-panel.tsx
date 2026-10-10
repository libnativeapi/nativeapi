import {
  Button,
  Callout,
  PreferenceRow,
  PreferenceSection,
  SegmentedControl,
  Select,
  Slider,
  Switch,
  Toggle,
} from '@dazzlabs/dazzui'

import { Panel, PanelPreferences } from '../../../components/panel'
import { osOf, PLATFORM_NAMES } from '../../../components/platform'
import { ReadBack } from '../../../components/read-back'
import { BACKGROUNDS, backgroundOf, TITLE_PRESETS, VISUAL_EFFECTS } from '../data'
import type { VisualEffect } from '../types'
import { buttonsVisibleOf } from '../use-windows'
import type { WindowPanelProps } from './state-panel'
import './panels.css'

/** Title and title bar, shadow, opacity, the material behind the window and its background. */
export function AppearancePanel({ w, platform, actions }: WindowPanelProps) {
  const os = osOf(platform)
  const id = w.id
  const effect = VISUAL_EFFECTS.find(e => e.value === w.visualEffect)!
  const background = backgroundOf(w.background)

  const effectNote =
    os === 'linux'
      ? `${PLATFORM_NAMES[platform]} has no visual effects: always false`
      : w.visualEffect === 'none'
        ? 'Replaces the background with a material'
        : os === 'macos'
          ? `NSVisualEffectView: ${effect.macos}`
          : effect.windows

  return (
    <Panel>
      <ReadBack
        keyWidth="14rem"
        columns={1}
        rows={[
          ['getTitle', `"${w.title}"`],
          ['getTitleBarStyle', `TitleBarStyle.${w.titleBarStyle}`],
          ['isContentUnderTitleBar', String(w.contentUnderTitleBar)],
          ['isWindowControlButtonsVisible', String(buttonsVisibleOf(w, os))],
          ['hasShadow', String(w.hasShadow)],
          ['getOpacity', w.opacity.toFixed(2)],
          ['getVisualEffect', `VisualEffect.${w.visualEffect}`],
          ['getBackgroundColor', `${background.rgba} · ${background.label}`],
        ]}
      />
      {w.kind === 'example' && (
        <Callout size="small" tint="info" title="This window draws its own chrome">
          Its title bar settings change what the getters return, not its look, and its content covers the
          background. Window #2 and #3 show every one of them.
        </Callout>
      )}
      <PanelPreferences>
        <PreferenceSection label="Title bar">
          <PreferenceRow title="Title" subtitle={`"${w.title}"`}>
            <SegmentedControl
              size="small"
              items={TITLE_PRESETS.map(t => ({ value: t, label: t }))}
              value={TITLE_PRESETS.find(t => t === w.title) ?? ''}
              onValueChange={t => actions.setTitle(id, t)}
            />
          </PreferenceRow>
          <PreferenceRow title="Title bar hidden" subtitle="No title bar and no buttons: custom chrome moves it">
            <Switch
              checked={w.titleBarStyle === 'hidden'}
              onCheckedChange={hidden => actions.setTitleBarStyle(id, hidden ? 'hidden' : 'normal')}
            />
          </PreferenceRow>
          <PreferenceRow
            title="Content under the title bar"
            subtitle={os === 'macos' ? 'The bar turns transparent; its buttons stay' : 'macOS only: returns false here'}
          >
            <Switch
              disabled={os !== 'macos'}
              checked={w.contentUnderTitleBar}
              onCheckedChange={value => actions.setContentUnderTitleBar(id, value)}
            />
          </PreferenceRow>
          <PreferenceRow
            title="Title bar colours"
            subtitle={os === 'windows' ? 'Windows, WinUI 3 backend only' : 'Windows WinUI 3 only: returns false here'}
          >
            <div className="example-panel__actions">
              <Button size="small" variant="normal" disabled={os !== 'windows'} onClick={() => actions.setTitleBarColors(id, true)}>
                Blue
              </Button>
              <Button
                size="small"
                variant="plain"
                disabled={os !== 'windows' || !w.titleBarColors}
                onClick={() => actions.setTitleBarColors(id, false)}
              >
                Reset
              </Button>
            </div>
          </PreferenceRow>
        </PreferenceSection>
        <PreferenceSection label="Surface">
          <PreferenceRow title="Shadow">
            <Switch checked={w.hasShadow} onCheckedChange={value => actions.setHasShadow(id, value)} />
          </PreferenceRow>
          <PreferenceRow title="Opacity" subtitle="The whole window, content included">
            <Slider
              className="window-panel__opacity"
              size="small"
              min={0.1}
              max={1}
              step={0.05}
              showValue
              formatValue={([v]) => v!.toFixed(2)}
              value={w.opacity}
              onValueChange={value => actions.setOpacity(id, value as number, false)}
              onValueCommitted={value => actions.setOpacity(id, value as number, true)}
              aria-label="Opacity"
            />
          </PreferenceRow>
          <PreferenceRow title="Visual effect" subtitle={effectNote}>
            <Select<VisualEffect>
              className="window-panel__effect"
              size="small"
              aria-label="Visual effect"
              options={VISUAL_EFFECTS.map(e => ({ value: e.value, label: e.label }))}
              value={w.visualEffect}
              onValueChange={value => actions.setVisualEffect(id, value)}
            />
          </PreferenceRow>
          <PreferenceRow
            title="Background colour"
            subtitle={w.visualEffect === 'none' ? `${background.label} · ${background.rgba}` : 'Kept, but the effect stands in for it'}
          >
            <div className="example-panel__actions">
              {BACKGROUNDS.map(b => (
                <Toggle
                  key={b.value}
                  size="small"
                  variant="normal"
                  label={b.label}
                  pressed={w.background === b.value}
                  onPressedChange={() => actions.setBackgroundColor(id, b.value)}
                >
                  <span
                    className="window-panel__swatch"
                    data-transparent={b.value === 'transparent' ? '' : undefined}
                    style={b.value === 'transparent' ? undefined : { background: b.token }}
                  />
                </Toggle>
              ))}
            </div>
          </PreferenceRow>
        </PreferenceSection>
      </PanelPreferences>
    </Panel>
  )
}
