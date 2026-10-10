import {
  Delete20Regular,
} from '@fluentui/react-icons'
import { useMemo, useState } from 'react'

import {
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
import { QuickAdd } from './components/quick-add'
import { SignArt } from './components/sign-art'
import { SignPanel } from './components/sign-panel'
import { TrayStage } from './components/tray-stage'
import { animationLabel, capabilitiesOf, STILL_ICONS } from './data'
import { headingOf } from './sign-data'
import type { Tab, TrayEntry } from './types'
import { timeOf, type TrayOptions, useTray } from './use-tray'
import './tray-icon-view.css'

export interface TrayIconViewProps {
  platform: WindowFramePlatform
  /** The icons and modes the example starts with. */
  options?: TrayOptions
  initialTab?: Tab
}

const itemName = (entry: TrayEntry) => `${entry.contentMode === 'sign' ? 'Sign' : 'Icon'} #${entry.number}`

const describe = (entry: TrayEntry) =>
  entry.contentMode === 'sign' ? headingOf(entry.sign) : entry.scene
    ? `${entry.scene[0]!.toUpperCase()}${entry.scene.slice(1)} scene`
    : entry.animation
      ? `${animationLabel(entry.animation)}${entry.paused ? ' · paused' : ''}`
      : `${STILL_ICONS.find(s => s.value === entry.still)?.label ?? 'Still'} icon`

/** Icons and custom sign views share a tray, with separate content editors. */
export function TrayIconView({ platform, options, initialTab = 'content' }: TrayIconViewProps) {
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
      title={selected ? itemName(selected) : 'Tray items'}
      subtitle={selected ? `id ${selected.id}` : undefined}
      toolbar={
        selected && (
          <SegmentedControl<Tab>
            size="small"
            tint={failed ? 'danger' : 'primary'}
            items={[
              { value: 'content', label: 'Content' },
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
          <IconButton label={`Remove ${itemName(selected).toLowerCase()}`} size="small" onClick={() => actions.removeIcon()}>
            <Icon icon={Delete20Regular} />
          </IconButton>
        )
      }
      sidebar={
        <>
          {(['icon', 'sign'] as const).map(kind => {
            const entries = state.entries.filter(entry => entry.contentMode === kind)
            if (!entries.length) return null
            return <SidebarGroup key={kind}>
              <SidebarGroupLabel>{kind === 'icon' ? 'Icons' : 'Signs'}</SidebarGroupLabel>
              <SidebarGroupContent>
                <SidebarMenu>
                  {entries.map(entry => (
                    <SidebarMenuItem key={entry.number}>
                      <SidebarMenuButton
                        size="large"
                        active={entry.number === state.selected}
                        onClick={() => actions.select(entry.number)}
                        icon={entry.contentMode === 'sign' ?
                          <span className="tray-icon-view__sign-thumbnail"><SignArt entry={entry.sign} size="tile" /></span> :
                          <IconCanvas animation={entry.animation} still={entry.still} time={() => timeOf(entry)}
                            pixels={16 * entry.scale} size={18} color={entry.color} />}
                      >
                        <SidebarRowLabel label={itemName(entry)} detail={describe(entry)} />
                      </SidebarMenuButton>
                      {entry.contentMode === 'sign' && !caps.contentView ? <SidebarMenuBadge>Preview</SidebarMenuBadge> :
                        !entry.visible && <SidebarMenuBadge>Hidden</SidebarMenuBadge>}
                    </SidebarMenuItem>
                  ))}
                </SidebarMenu>
              </SidebarGroupContent>
            </SidebarGroup>
          })}
        </>
      }
      sidebarFooter={
        <QuickAdd caps={caps} actions={actions} onAdded={() => setTab('content')} />
      }
      footer={<EventBar lastEvent={state.lastEvent} log={state.log} onClear={actions.clearLog} />}
    >
      {selected ? (
        <>
          {selected.contentMode === 'sign' ? <div className="tray-sign-panel__preview" aria-label="Current sign preview">
            <SignArt entry={selected.sign} />
          </div> : <LivePreview entry={selected} actions={actions} />}
          <div className="example-window__scroll">
            {tab === 'content' && (selected.contentMode === 'sign' ?
              <SignPanel entry={selected} caps={caps} actions={actions} /> :
              <AnimatePanel entry={selected} caps={caps} actions={actions} />)}
            {tab === 'properties' && <PropertiesPanel entry={selected} caps={caps} state={state} actions={actions} />}
            {tab === 'checklist' && <ChecklistPanel checklist={state.checklist} caps={caps} actions={actions} />}
          </div>
        </>
      ) : (
        <EmptyState
          className="tray-icon-view__empty"
          title="Use Add to create your first tray item."
        />
      )}
    </ExampleWindow>
  )

  return (
    <TrayStage caps={caps} state={state} actions={actions} onSignClick={() => setTab('content')}>
      {state.windowVisible ? appWindow : null}
    </TrayStage>
  )
}
