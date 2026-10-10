import { useRef } from 'react'

import {
  Button,
  OptionCard,
  SectionLabel,
  PreferenceGroup,
  PreferenceRow,
  Preferences,
  PreferenceSection,
  SegmentedControl,
  ToggleGroup,
} from '@dazzlabs/dazzui'

import { ANIMATIONS, COLORS, ICON_POINTS, RATES, SCALES, SCENES, STILL_ICONS } from '../data'
import type { Capabilities, IconColor, StillIcon, TrayEntry } from '../types'
import { nowSeconds, type TrayActions } from '../use-tray'
import { IconCanvas } from './icon-canvas'
import './panels.css'

export interface AnimatePanelProps {
  entry: TrayEntry
  caps: Capabilities
  actions: TrayActions
}

/**
 * The gallery: every tile is alive, one click plays it in the tray. Under it,
 * what the frames are made of — the still images, the rate, the resolution,
 * the colour.
 */
export function AnimatePanel({ entry, caps, actions }: AnimatePanelProps) {
  // One clock for all tiles. They only preview the look; the tray's frames
  // come from the selected icon's own animator.
  const start = useRef(nowSeconds())
  const tileTime = () => nowSeconds() - start.current

  return (
    <div className="tray-panel">
      <SectionLabel>Animations</SectionLabel>
      <div className="tray-panel__gallery">
        {ANIMATIONS.map(({ value, label }) => (
          <OptionCard
            key={value}
            className="tray-panel__tile"
            selected={entry.animation === value}
            onClick={() => actions.play(value)}
            title={
              <span className="tray-panel__tile-content">
                <IconCanvas animation={value} still={null} time={tileTime} pixels={44} size={22} />
                {label}
              </span>
            }
          />
        ))}
      </div>
      <SectionLabel>Icon presets</SectionLabel>
      <div className="tray-panel__presets">
        {SCENES.map(({ value, label }) => <Button key={value} size="small"
          variant={entry.scene === value ? 'tinted' : 'normal'} aria-pressed={entry.scene === value}
          onClick={() => entry.scene === value ? actions.resetScene() : actions.playScene(value)}>{label}</Button>)}
        <Button size="small" variant="normal" onClick={actions.playThreeAtOnce}>Three icons</Button>
      </div>
      <Preferences className="tray-panel__preferences">
        <PreferenceSection>
          <PreferenceGroup title="Frames">
            <PreferenceRow title="Still icon">
              <ToggleGroup<StillIcon>
                size="small"
                items={STILL_ICONS}
                value={entry.contentMode === 'icon' && entry.still ? [entry.still] : []}
                onValueChange={([still]) => still && actions.setStill(still)}
              />
            </PreferenceRow>
            <PreferenceRow title="Rate">
              <SegmentedControl
                size="small"
                items={RATES.map(fps => ({ value: String(fps), label: `${fps} fps` }))}
                value={String(entry.fps)}
                onValueChange={fps => actions.setFps(Number(fps))}
              />
            </PreferenceRow>
            <PreferenceRow title="Resolution">
              <SegmentedControl
                size="small"
                items={SCALES.map(scale => ({ value: String(scale), label: `${scale}x · ${ICON_POINTS * scale} px` }))}
                value={String(entry.scale)}
                onValueChange={scale => actions.setScale(Number(scale))}
              />
            </PreferenceRow>
            <PreferenceRow
              title="Colour"
              subtitle={caps.os === 'macos' ? 'Auto is a template image: the menu bar tints it' : undefined}
            >
              <SegmentedControl<IconColor>
                size="small"
                items={COLORS}
                value={entry.color}
                onValueChange={actions.setColor}
              />
            </PreferenceRow>
          </PreferenceGroup>
        </PreferenceSection>
      </Preferences>
    </div>
  )
}
