import type { PointerEvent as ReactPointerEvent, ReactNode } from 'react'

import { Badge, Button, Callout, Card, Divider, SectionLabel, Switch, Tag, WindowFrame, type WindowFramePlatform } from '@dazzlabs/dazzui'

import { osOf } from '../../../components/platform'
import { EFFECTS, effectOf, unsupportedReason, WINDOW_SIZE } from '../data'
import { drawnAs, simulateIsVisualEffectSupported } from '../set-visual-effect'
import type { VisualEffect } from '../types'
import './material-window.css'

export interface MaterialWindowProps {
  platform: WindowFramePlatform
  /** What `visualEffect` reads back. */
  effect: VisualEffect
  /** What the last call left: `Applied mica`, `Refused blur`. */
  note: string
  /** The key window; Windows draws a solid colour in place of a backdrop while it is not. */
  active: boolean
  backdrop: boolean
  onApply: (effect: VisualEffect) => void
  onToggleBackdrop: () => void
  /** A press on the title bar: the window manager drags the window. */
  onTitlePress: (event: ReactPointerEvent) => void
  /** Over the bare area's foot: the read-back. */
  readBack: ReactNode
  /** The window's foot: the event bar. */
  footer: ReactNode
}

/**
 * The example's one window, whose background is the material. The content
 * takes the title bar in where the platform allows it (macOS), paints a
 * raised card for the controls, and leaves the rest of the window bare: that
 * is where the material shows. With no effect the window paints the canvas.
 */
export function MaterialWindow({
  platform,
  effect,
  note,
  active,
  backdrop,
  onApply,
  onToggleBackdrop,
  onTitlePress,
  readBack,
  footer,
}: MaterialWindowProps) {
  const os = osOf(platform)
  const underTitleBar = os === 'macos'
  const drawn = drawnAs(effect, os)
  const reason = unsupportedReason(os)
  const refused = note.startsWith('Refused')
  const info = effectOf(effect)
  const nativeName = os === 'macos' ? info.macos : os === 'windows' ? info.windows : 'Background colour'
  // Windows' system backdrops fall back to a solid colour while the window is
  // not the key window; AppKit keeps its material in the background.
  const fallback = os === 'windows' && !active && effect !== 'none'

  return (
    <div
      className="material-window"
      onPointerDown={event => {
        const target = event.target as HTMLElement
        if (target.closest('button, [role="switch"]')) return
        if (target.closest('.dz-window-frame__titlebar, .material-window__drag')) onTitlePress(event)
      }}
    >
      <WindowFrame
        platform={platform}
        title="Visual effect"
        fullSizeContent={underTitleBar}
        width={WINDOW_SIZE.width}
        height={WINDOW_SIZE.height}
        inactive={!active}
        className="material-window__frame"
        data-material={fallback ? 'fallback' : drawn}
      >
        {underTitleBar && <div className="material-window__drag" />}
        <div className="material-window__content" data-under-title-bar={underTitleBar ? '' : undefined}>
          <Card variant="raised" className="material-window__card">
            <div className="material-window__head">
              <span className="material-window__title">Effect: {effect}</span>
              <Badge size="small" variant="tinted" tint={refused ? 'danger' : effect !== 'none' ? 'success' : 'neutral'}>
                {note}
              </Badge>
            </div>
            <SectionLabel>Window.setVisualEffect</SectionLabel>
            <div className="material-window__chips">
              {EFFECTS.map(({ value }) => {
                const supported = simulateIsVisualEffectSupported(value, os)
                return (
                  <Tag
                    key={value}
                    size="medium"
                    variant="outlined"
                    selected={value === effect}
                    disabled={!supported}
                    title={supported ? effectOf(value).what : (reason ?? undefined)}
                    onSelectedChange={() => onApply(value)}
                  >
                    {value}
                  </Tag>
                )
              })}
            </div>
            {reason ? (
              <Callout
                size="small"
                tint="info"
                title="No visual effects here"
                action={
                  <Button size="small" variant="normal" onClick={() => onApply('blur')}>
                    Try blur
                  </Button>
                }
              >
                {reason} setVisualEffect returns false and the background colour stays.
              </Callout>
            ) : (
              <p className="material-window__note">
                {info.what} {effect !== 'none' && <span className="material-window__native">{nativeName}</span>}
              </p>
            )}
            <Divider />
            <div className="material-window__row">
              <div className="material-window__row-text">
                <span>Backdrop</span>
                <span className="material-window__note">A plain red window behind this one, to see through</span>
              </div>
              <Switch checked={backdrop} onCheckedChange={onToggleBackdrop} aria-label="Backdrop" />
            </div>
          </Card>
          {/* Bare window: the content paints nothing here, so the material shows. */}
          <div className="material-window__bare">
            <span>
              {fallback
                ? 'Inactive: Windows shows the solid fallback colour'
                : effect === 'none'
                  ? 'No effect: the window paints its canvas'
                  : effect === 'mica' || effect === 'micaAlt'
                    ? 'Unpainted: tinted by the wallpaper, not the windows behind'
                    : 'Unpainted: the material shows here'}
            </span>
          </div>
          {readBack}
        </div>
        <div className="material-window__foot">{footer}</div>
      </WindowFrame>
    </div>
  )
}
