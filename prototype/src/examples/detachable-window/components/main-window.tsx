import { ArrowDownload24Regular, Board24Regular, Open16Regular, Pin16Regular } from '@fluentui/react-icons'

import { Badge, Card, Icon, SectionLabel, WindowFrame, type WindowFramePlatform } from '@dazzlabs/dazzui'

import { MAIN_SIZE, PANELS, SLOTS } from '../data'
import type { DetachSimulation } from '../simulate-detach'
import type { MainWindow as MainWindowModel, SlotId } from '../types'
import { useElementSize } from '../use-element-size'
import { PanelContent } from './panels'
import './main-window.css'

export interface MainWindowProps {
  platform: WindowFramePlatform
  detach: DetachSimulation
  window: MainWindowModel
  inactive?: boolean
}

/**
 * A main window: a workspace with a panel slot on two of its sides. Window A
 * has a sidebar and a bottom panel, Window B a top strip and a wide sidebar,
 * so a panel visibly takes the shape of the slot it docks into.
 */
export function MainWindow({ platform, detach, window, inactive }: MainWindowProps) {
  const slot = (id: SlotId) => <DockSlot detach={detach} id={id} />
  return (
    <WindowFrame
      platform={platform}
      title={`nativeapi · Window ${window.id}`}
      width={MAIN_SIZE.width}
      height={MAIN_SIZE.height}
      inactive={inactive}
      className="main-window"
      data-detach-window={window.nativeId}
      onClose={() => detach.closeMain(window.id)}
    >
      {window.id === 'A' ? (
        <div className="main-window__row">
          <div className="main-window__slot main-window__slot--sidebar">{slot('A-left')}</div>
          <div className="main-window__column">
            <Workspace detach={detach} window={window} />
            <div className="main-window__slot main-window__slot--bottom">{slot('A-bottom')}</div>
          </div>
        </div>
      ) : (
        <div className="main-window__column">
          <div className="main-window__slot main-window__slot--top">{slot('B-top')}</div>
          <div className="main-window__row">
            <Workspace detach={detach} window={window} />
            <div className="main-window__slot main-window__slot--wide">{slot('B-right')}</div>
          </div>
        </div>
      )}
    </WindowFrame>
  )
}

/** A place where one panel can dock: the panel, or a free slot that is a drop target while a panel's window is dragged. */
function DockSlot({ detach, id }: { detach: DetachSimulation; id: SlotId }) {
  const panel = detach.itemInSlot(id)
  const [ref, size] = useElementSize()
  const target = detach.hovered === id
  const lit = detach.isMovingWindow
  return (
    <div ref={ref} className="dock-slot" data-slot={id} data-target={target ? '' : undefined} data-lit={lit ? '' : undefined}>
      {panel ? (
        <PanelContent detach={detach} id={panel} />
      ) : (
        // Edge to edge, with no margin: the highlight is exactly what a docked panel, and its torn-off window, occupies.
        <div className="dock-slot__empty">
          <Icon icon={target ? ArrowDownload24Regular : Board24Regular} size={28} />
          <strong>{SLOTS[id].label}</strong>
          <code>{target ? 'Release to dock' : `${size.width} × ${size.height}`}</code>
        </div>
      )}
    </div>
  )
}

function Workspace({ detach, window }: { detach: DetachSimulation; window: MainWindowModel }) {
  return (
    <div className="workspace">
      <h2 className="workspace__title">Window {window.id}</h2>
      <p className="workspace__text">
        Drag a panel by its header to pop it out, and onto any empty slot to dock it. Panels keep their state
        throughout; closing a window pops its panels out.
      </p>
      <div className="workspace__badges">
        {(['inspector', 'stopwatch'] as const).map(id => {
          const place = detach.places[id]
          const floating = place.kind === 'floating'
          return (
            <Badge key={id} size="small" variant="tinted" tint={floating ? 'info' : 'neutral'}>
              <Icon icon={floating ? Open16Regular : Pin16Regular} size={12} />
              {PANELS[id].title}: {place.kind === 'docked' ? `docked ${place.slot}` : 'floating'}
            </Badge>
          )
        })}
      </div>
      <SectionLabel>Activity</SectionLabel>
      <Card variant="sunken" size="small" className="workspace__activity">
        {detach.activity.length === 0 ? (
          <p className="workspace__empty">Nothing yet — try dragging a panel header.</p>
        ) : (
          <ol>
            {detach.activity.map((line, i) => (
              <li key={detach.activity.length - i}>{line}</li>
            ))}
          </ol>
        )}
      </Card>
    </div>
  )
}
