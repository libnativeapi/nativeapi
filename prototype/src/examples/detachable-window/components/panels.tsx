import { Layer20Regular } from '@fluentui/react-icons'
import { useLayoutEffect, useRef } from 'react'

import {
  Button,
  FormField,
  Icon,
  SectionLabel,
  Slider,
  Switch,
  Table,
  TableCell,
  TableHead,
  TableRow,
  TextField,
} from '@dazzlabs/dazzui'

import { useNow } from '../../../components/use-now'
import { LAYER_COUNT } from '../data'
import type { DetachSimulation } from '../simulate-detach'
import type { PanelId } from '../types'
import { useElementSize } from '../use-element-size'
import { PanelFrame } from './panel-frame'
import './panels.css'

/** A panel's content, wherever it is shown: in a slot, or in a window of its own. */
export function PanelContent({ detach, id }: { detach: DetachSimulation; id: PanelId }) {
  return (
    <PanelFrame detach={detach} id={id}>
      {id === 'inspector' ? <InspectorPanel detach={detach} /> : <StopwatchPanel detach={detach} />}
    </PanelFrame>
  )
}

/**
 * A layer inspector with state a rebuilt panel would lose: an edited name, a
 * counter, a switch, a slider and a scrolled list. Narrow, it stacks; wide
 * and short, as in a bottom or top strip, the list goes beside the controls.
 */
function InspectorPanel({ detach }: { detach: DetachSimulation }) {
  const state = detach.inspector
  const [body, size] = useElementSize()
  const layers = useRef<HTMLDivElement>(null)
  const wide = size.width >= 400

  // The list comes back scrolled where it was, in any window.
  useLayoutEffect(() => {
    if (layers.current) layers.current.scrollTop = state.scroll
  }, [state, wide])

  const set = (change: Partial<typeof state>) => {
    Object.assign(state, change)
    detach.touch()
  }

  const controls = (
    <div className="inspector-panel__controls">
      <FormField label="Layer name">
        <TextField size="small" value={state.name} onChange={event => set({ name: (event.target as HTMLInputElement).value })} />
      </FormField>
      <div className="inspector-panel__row">
        <span className="inspector-panel__clicks">Clicks: {state.clicks}</span>
        <Button size="small" variant="tinted" onClick={() => set({ clicks: state.clicks + 1 })}>
          +1
        </Button>
      </div>
      <label className="inspector-panel__row">
        <span>Visible</span>
        <Switch size="small" checked={state.visible} onCheckedChange={visible => set({ visible })} />
      </label>
      <div className="inspector-panel__opacity">
        <span>Opacity {state.opacity}%</span>
        <Slider
          size="small"
          aria-label="Opacity"
          value={state.opacity}
          onValueChange={value => set({ opacity: Math.round(Array.isArray(value) ? value[0]! : (value as number)) })}
        />
      </div>
    </div>
  )
  const list = (
    <div
      ref={layers}
      className="inspector-panel__layers"
      onScroll={event => {
        state.scroll = event.currentTarget.scrollTop
      }}
    >
      <Table>
        {Array.from({ length: LAYER_COUNT }, (_, i) => (
          <TableRow key={i} interactive active={i === state.selected} onClick={() => set({ selected: i })}>
            <TableCell>
              <span className="inspector-panel__layer">
                <Icon icon={Layer20Regular} size={16} />
                Layer {i + 1}
              </span>
            </TableCell>
          </TableRow>
        ))}
      </Table>
    </div>
  )

  return (
    <div ref={body} className="inspector-panel" data-wide={wide ? '' : undefined}>
      {wide ? (
        <>
          <div className="inspector-panel__scroll">{controls}</div>
          {list}
        </>
      ) : (
        <div className="inspector-panel__scroll">
          {controls}
          <SectionLabel>Layers (scroll position is kept too)</SectionLabel>
          {list}
        </div>
      )}
    </div>
  )
}

const format = (ms: number) => {
  const two = (v: number) => String(Math.floor(v)).padStart(2, '0')
  return `${two(ms / 60000)}:${two((ms / 1000) % 60)}.${two((ms % 1000) / 10)}`
}

/**
 * A stopwatch that keeps running while the panel moves: proof that nothing
 * was recreated. Wide and short, the clock goes beside the laps.
 */
function StopwatchPanel({ detach }: { detach: DetachSimulation }) {
  const state = detach.stopwatch
  const running = state.startedAt !== null
  useNow(40, running)
  const [body, size] = useElementSize()
  const wide = size.width > size.height * 1.6
  const elapsed = state.elapsed + (state.startedAt === null ? 0 : performance.now() - state.startedAt)

  const toggle = () => {
    if (state.startedAt === null) state.startedAt = performance.now()
    else {
      state.elapsed += performance.now() - state.startedAt
      state.startedAt = null
    }
    detach.touch()
  }

  return (
    <div ref={body} className="stopwatch-panel" data-wide={wide ? '' : undefined}>
      <div className="stopwatch-panel__clock">
        <span className="stopwatch-panel__time">{format(elapsed)}</span>
        <span className="stopwatch-panel__hint">Keeps running while the panel moves</span>
        <div className="stopwatch-panel__actions">
          <Button size="small" variant="filled" onClick={toggle}>
            {running ? 'Pause' : 'Start'}
          </Button>
          <Button
            size="small"
            variant="normal"
            onClick={() => {
              state.laps.unshift(elapsed)
              detach.touch()
            }}
          >
            Lap
          </Button>
          <Button
            size="small"
            variant="plain"
            onClick={() => {
              state.elapsed = 0
              state.startedAt = running ? performance.now() : null
              state.laps = []
              detach.touch()
            }}
          >
            Reset
          </Button>
        </div>
      </div>
      <div className="stopwatch-panel__laps">
        {state.laps.length === 0 ? (
          <p className="stopwatch-panel__empty">No laps yet</p>
        ) : (
          <Table>
            <TableHead>
              <TableCell head className="stopwatch-panel__lap-number">
                #
              </TableCell>
              <TableCell head>Time</TableCell>
            </TableHead>
            {state.laps.map((lap, i) => (
              <TableRow key={state.laps.length - i}>
                <TableCell className="stopwatch-panel__lap-number">#{state.laps.length - i}</TableCell>
                <TableCell className="stopwatch-panel__lap-time">{format(lap)}</TableCell>
              </TableRow>
            ))}
          </Table>
        )}
      </div>
    </div>
  )
}
