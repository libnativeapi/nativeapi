import {
  Add20Regular,
  ArrowDownload20Regular,
  ArrowSync20Regular,
  Delete20Regular,
  Record20Regular,
  SquareMultiple20Regular,
} from '@fluentui/react-icons'
import { useMemo, useState } from 'react'

import {
  Button,
  EmptyState,
  Icon,
  IconButton,
  SegmentedControl,
  SidebarGroup,
  SidebarGroupContent,
  SidebarGroupLabel,
  SidebarMenu,
  SidebarMenuBadge,
  SidebarMenuButton,
  SidebarMenuItem,
  type WindowFramePlatform,
} from '@dazzlabs/dazzui'

import { AnimatePanel } from './components/animate-panel'
import { ChecklistPanel } from './components/checklist-panel'
import { EventBar } from '../../components/event-bar/event-bar'
import { ExampleWindow, SidebarRowLabel } from '../../components/example-window/example-window'
import { IconCanvas } from './components/icon-canvas'
import { LivePreview } from './components/live-preview'
import { PropertiesPanel } from './components/properties-panel'
import { TrayStage } from './components/tray-stage'
import { animationLabel, capabilitiesOf, SCENES, STILL_ICONS } from './data'
import type { Scene, Tab, TrayEntry } from './types'
import { timeOf, type TrayOptions, useTray } from './use-tray'
import './tray-icon-view.css'

export interface TrayIconViewProps {
  platform: WindowFramePlatform
  /** The icons and modes the example starts with. */
  options?: TrayOptions
  initialTab?: Tab
}

const SCENE_ICONS: Record<Scene, typeof Record20Regular> = {
  download: ArrowDownload20Regular,
  recording: Record20Regular,
  syncing: ArrowSync20Regular,
}

const describe = (entry: TrayEntry) =>
  entry.scene
    ? `${entry.scene[0]!.toUpperCase()}${entry.scene.slice(1)} scene`
    : entry.animation
      ? `${animationLabel(entry.animation)}${entry.paused ? ' · paused' : ''}`
      : `${STILL_ICONS.find(s => s.value === entry.still)?.label ?? 'Still'} icon`

/**
 * The tray icon example: a playground for `TrayIcon` and `TrayManager` on
 * the desktop it runs on. The sidebar holds the icons the example created;
 * the pane acts on the selected one — the live preview of its frames over
 * three tabs (Animate, Properties, Checklist) — and the foot shows the last
 * event. The icons themselves are in the bar above or below.
 */
export function TrayIconView({ platform, options, initialTab = 'animate' }: TrayIconViewProps) {
  const caps = useMemo(() => capabilitiesOf(platform), [platform])
  const { state, selected, actions } = useTray(caps, options)
  const [tab, setTab] = useState<Tab>(initialTab)

  const passed = state.checklist.filter(i => i.status === 'pass').length
  const failed = state.checklist.some(i => i.status === 'fail')

  const appWindow = (
    <ExampleWindow
      platform={platform}
      appName="Tray Icon"
      className="tray-icon-view"
      onClose={actions.hideWindow}
      title={selected ? `Icon #${selected.number}` : 'Tray icon example'}
      subtitle={selected ? `id ${selected.id}` : undefined}
      toolbar={
        selected && (
          <SegmentedControl<Tab>
            size="small"
            tint={failed ? 'danger' : 'primary'}
            items={[
              { value: 'animate', label: 'Animate' },
              { value: 'properties', label: 'Properties' },
              { value: 'checklist', label: `Checklist ${passed}/${state.checklist.length}` },
            ]}
            value={tab}
            onValueChange={setTab}
          />
        )
      }
      trailing={
        selected && (
          <IconButton label={`Remove icon #${selected.number}`} size="small" onClick={() => actions.removeIcon()}>
            <Icon icon={Delete20Regular} />
          </IconButton>
        )
      }
      sidebar={
        <>
          <SidebarGroup>
            <SidebarGroupLabel>Tray icons</SidebarGroupLabel>
            <SidebarGroupContent>
              <SidebarMenu>
                {state.entries.map(entry => (
                  <SidebarMenuItem key={entry.number}>
                    <SidebarMenuButton
                      size="large"
                      active={entry.number === state.selected}
                      onClick={() => actions.select(entry.number)}
                      icon={
                        <IconCanvas
                          animation={entry.animation}
                          still={entry.still}
                          time={() => timeOf(entry)}
                          pixels={16 * entry.scale}
                          size={18}
                          color={entry.color}
                        />
                      }
                    >
                      <SidebarRowLabel label={`Icon #${entry.number}`} detail={describe(entry)} />
                    </SidebarMenuButton>
                    {!entry.visible && <SidebarMenuBadge>Hidden</SidebarMenuBadge>}
                  </SidebarMenuItem>
                ))}
              </SidebarMenu>
            </SidebarGroupContent>
          </SidebarGroup>
          {/* Canned uses of an icon: animation, title and tooltip moving
              together on the selected one, or three icons at once. */}
          <SidebarGroup>
            <SidebarGroupLabel>Scenes</SidebarGroupLabel>
            <SidebarGroupContent>
              <SidebarMenu>
                {SCENES.map(({ value, label }) => (
                  <SidebarMenuItem key={value}>
                    <SidebarMenuButton
                      disabled={!selected}
                      active={selected?.scene === value}
                      icon={<Icon icon={SCENE_ICONS[value]} />}
                      onClick={() => (selected?.scene === value ? actions.resetScene() : actions.playScene(value))}
                    >
                      {label}
                    </SidebarMenuButton>
                  </SidebarMenuItem>
                ))}
                <SidebarMenuItem>
                  <SidebarMenuButton icon={<Icon icon={SquareMultiple20Regular} />} onClick={actions.playThreeAtOnce}>
                    Three icons
                  </SidebarMenuButton>
                </SidebarMenuItem>
              </SidebarMenu>
            </SidebarGroupContent>
          </SidebarGroup>
        </>
      }
      sidebarFooter={
        <Button size="small" variant="normal" fullWidth onClick={actions.addIcon}>
          <Icon icon={Add20Regular} />
          Add icon
        </Button>
      }
      footer={<EventBar lastEvent={state.lastEvent} log={state.log} onClear={actions.clearLog} />}
    >
      {selected ? (
        <>
          <LivePreview entry={selected} actions={actions} />
          <div className="example-window__scroll">
            {tab === 'animate' && <AnimatePanel entry={selected} caps={caps} actions={actions} />}
            {tab === 'properties' && <PropertiesPanel entry={selected} caps={caps} state={state} actions={actions} />}
            {tab === 'checklist' && <ChecklistPanel checklist={state.checklist} caps={caps} actions={actions} />}
          </div>
        </>
      ) : (
        <EmptyState
          className="tray-icon-view__empty"
          title="No tray icon. Add one to start."
          action={
            <Button size="small" variant="filled" onClick={actions.addIcon}>
              Add icon
            </Button>
          }
        />
      )}
    </ExampleWindow>
  )

  return (
    <TrayStage caps={caps} state={state} actions={actions}>
      {state.windowVisible ? appWindow : null}
    </TrayStage>
  )
}
