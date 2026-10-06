import type { CSSProperties } from 'react'

import {
  Button,
  Callout,
  Divider,
  OptionCard,
  SectionLabel,
  SegmentedControl,
  Slider,
  Switch,
  Tag,
  WindowFooter,
  WindowFrame,
  type WindowFramePlatform,
} from '@dazzlabs/dazzui'

import { EventBar } from '../../../components/event-bar'
import { ReadBack } from '../../../components/read-back'
import { CONTROLS_SIZE, LOOKS, NIGHT, SHADOW_COLORS, SHAPES } from '../data'
import { clipPathOf, shapePoints } from '../shape-geometry'
import type { ShadowColor, ShadowPreset } from '../types'
import { presetOf, type ShapedWindow } from '../use-shaped-window'
import { ShapeArt } from './shape-art'
import './controls-window.css'

export interface ControlsWindowProps {
  platform: WindowFramePlatform
  sw: ShapedWindow
  inactive?: boolean
}

const PRESETS: readonly ShadowPreset[] = ['None', 'Soft', 'Float', 'Sharp', 'Glow']
const THUMB = 46

/**
 * The "Window shapes" window: the playground's banner, the twelve
 * silhouettes, what to do with the preview window — shape it, restore the
 * rectangle, toggle its size — and its shadow, by preset or adjusted by
 * hand. The foot carries the shape's status and the call log.
 */
export function ControlsWindow({ platform, sw, inactive }: ControlsWindowProps) {
  const look = LOOKS[sw.shape]
  const preset = presetOf(sw.shadow)

  return (
    <WindowFrame
      platform={platform}
      title="Window shapes"
      width={CONTROLS_SIZE.width}
      height={CONTROLS_SIZE.height}
      inactive={inactive}
      className="controls-window"
    >
      <div
        className="controls-window__banner"
        style={{ '--banner-from': NIGHT, '--banner-to': look.colors[0] } as CSSProperties}
      >
        <span className="controls-window__headline">Outside the box.</span>
        <span className="controls-window__kicker">SHAPE PLAYGROUND</span>
      </div>

      <div className="controls-window__heading">
        <SectionLabel>{sw.editingShadow ? 'Custom shadow' : 'Shape collection'}</SectionLabel>
        {sw.editingShadow ? (
          <Button size="tiny" variant="filled" onClick={() => sw.setEditingShadow(false)}>
            Done
          </Button>
        ) : (
          <span className="controls-window__mono">{SHAPES.length} silhouettes</span>
        )}
      </div>

      {sw.editingShadow ? (
        <ShadowControls sw={sw} />
      ) : (
        <>
          {/* Four by three, never scrolling. */}
          <div className="controls-window__gallery">
            {SHAPES.map(shape => (
              <OptionCard
                key={shape}
                className="controls-window__card"
                selected={!sw.restored && shape === sw.shape}
                onClick={() => sw.selectShape(shape)}
                title={
                  <span className="controls-window__card-content">
                    <ShapeArt
                      look={LOOKS[shape]}
                      className="controls-window__thumb"
                      style={{ clipPath: clipPathOf(shapePoints(shape, THUMB)) }}
                    />
                    <span>{shape}</span>
                  </span>
                }
              />
            ))}
          </div>
          <Divider />
          <div className="controls-window__row">
            <span className="controls-window__label">Window</span>
            <Button size="small" variant="normal" onClick={() => sw.selectShape(sw.shape)}>
              Apply shape
            </Button>
            <Button size="small" variant="normal" onClick={sw.restoreRectangle}>
              Restore rectangle
            </Button>
          </div>
          <div className="controls-window__row">
            <span className="controls-window__label">Size</span>
            <Button size="small" variant="normal" onClick={sw.toggleSize}>
              Toggle size
            </Button>
            <span className="controls-window__mono">
              {sw.toSize} × {sw.toSize} · 450 ms
            </span>
          </div>
        </>
      )}

      <div className="controls-window__heading controls-window__heading--shadow">
        <SectionLabel>Shadow</SectionLabel>
        <span className="controls-window__mono">{preset}</span>
        <Tag
          size="medium"
          variant="outlined"
          className="controls-window__adjust"
          selected={sw.editingShadow}
          onSelectedChange={() => sw.setEditingShadow(!sw.editingShadow)}
        >
          {sw.editingShadow ? 'Back to shapes' : 'Adjust…'}
        </Tag>
      </div>
      <div className="controls-window__presets">
        <SegmentedControl<string>
          size="small"
          stretch
          items={PRESETS.map(name => ({ value: name, label: name }))}
          // A slider-made shadow is none of them.
          value={preset === 'Custom' ? '' : preset}
          onValueChange={value => sw.selectPreset(value as ShadowPreset)}
        />
      </div>

      <WindowFooter className="controls-window__foot">
        <EventBar lastEvent={sw.log.lastEvent} log={sw.log.log} onClear={sw.log.clear} />
      </WindowFooter>
    </WindowFrame>
  )
}

const SLIDERS = [
  { key: 'opacity', label: 'Opacity', min: 0, max: 1, step: 0.01 },
  { key: 'blur', label: 'Blur radius', min: 0, max: 64, step: 1 },
  { key: 'x', label: 'Horizontal', min: -64, max: 64, step: 1 },
  { key: 'y', label: 'Vertical', min: -64, max: 64, step: 1 },
] as const

/** The Adjust view: every `WindowShadow` parameter, and what the getters return. */
function ShadowControls({ sw }: { sw: ShapedWindow }) {
  const { shadow } = sw
  return (
    <div className="controls-window__shadow">
      <Divider />
      <div className="controls-window__row">
        <span className="controls-window__label">Shadow</span>
        <Switch size="small" checked={shadow.enabled} onCheckedChange={sw.toggleShadow} aria-label="Shadow" />
        <span className="controls-window__hint">Contour shadow</span>
      </div>
      <div className="controls-window__row">
        <span className="controls-window__label">Colour</span>
        <div className="controls-window__chips">
          {(Object.keys(SHADOW_COLORS) as ShadowColor[]).map(color => (
            <Tag
              key={color}
              size="medium"
              variant="outlined"
              selected={shadow.color === color}
              onSelectedChange={() => sw.changeShadow({ color })}
              icon={<span className="controls-window__swatch" style={{ background: SHADOW_COLORS[color] }} />}
            >
              {color}
            </Tag>
          ))}
        </div>
      </div>
      {SLIDERS.map(({ key, label, min, max, step }) => (
        <div key={key} className="controls-window__row">
          <span className="controls-window__label">{label}</span>
          <Slider
            className="controls-window__slider"
            size="small"
            min={min}
            max={max}
            step={step}
            value={shadow[key]}
            showValue
            formatValue={([v = 0]) => (key === 'opacity' ? `${Math.round(v * 100)}%` : `${Math.round(v)} px`)}
            aria-label={label}
            onValueChange={value => sw.changeShadow({ [key]: Array.isArray(value) ? value[0] : value }, { log: false })}
            onValueCommitted={value => sw.changeShadow({ [key]: Array.isArray(value) ? value[0] : value })}
          />
        </div>
      ))}
      <div className="controls-window__row">
        <span className="controls-window__label">Defaults</span>
        <Button size="small" variant="normal" onClick={sw.resetShadow}>
          Reset shadow parameters
        </Button>
      </div>
      {!shadow.enabled && (
        <Callout size="small" tint="info">
          Shadow hidden. Changes appear when enabled.
        </Callout>
      )}
      <ReadBack
        columns={2}
        keyWidth="7.75rem"
        rows={[
          ['hasShadow', String(shadow.enabled)],
          ['blurRadius', String(Math.round(shadow.blur))],
          ['color', `${shadow.color.toLowerCase()} ${Math.round(shadow.opacity * 100)}%`],
          ['offset', `${Math.round(shadow.x)}, ${Math.round(shadow.y)}`],
          ['isShaped', String(!sw.restored && !sw.usesInputShape)],
          ['isInputShaped', String(!sw.restored && sw.usesInputShape)],
        ]}
      />
    </div>
  )
}
