import {
  ArrowDownload20Regular,
  CheckmarkCircle16Filled,
  Circle16Filled,
  Delete20Regular,
  Open16Regular,
  PersonKey20Regular,
  Timer16Regular,
} from '@fluentui/react-icons'
import { useState } from 'react'

import {
  Button,
  Icon,
  SegmentedControl,
  SidebarGroup,
  SidebarGroupContent,
  SidebarGroupLabel,
  SidebarMenu,
  SidebarMenuButton,
  SidebarMenuItem,
  type WindowFramePlatform,
} from '@dazzlabs/dazzui'

import { DesktopStage, DesktopWindow } from '../../components/desktop-stage'
import { EventBar } from '../../components/event-bar'
import { ExampleWindow, SidebarRowLabel } from '../../components/example-window'
import { osOf } from '../../components/platform'
import { DialogSurface } from './components/dialog-surface'
import { ComposePanel, ExtendedPanel, ResultPanel } from './components/panels'
import { BACKEND_NAMES, PRESETS, RESULT_NAMES } from './data'
import type { PresetId, Tab } from './types'
import { blocks, type MessageDialogOptions, useMessageDialog } from './use-message-dialog'
import './message-dialog-view.css'

export interface MessageDialogViewProps {
  platform: WindowFramePlatform
  options?: MessageDialogOptions
  initialTab?: Tab
}

const PRESET_ICONS: Record<PresetId, typeof Delete20Regular> = {
  update: ArrowDownload20Regular,
  delete: Delete20Regular,
  signIn: PersonKey20Regular,
}

const WINDOW = { x: 40, y: 28, width: 800, height: 520 }

/**
 * The message dialog example: compose a `MessageDialog`, open it, and see it
 * as the platform draws it — an NSAlert or a sheet, a Win32 MessageBox, a
 * WinUI 3 ContentDialog with the extended controls, GTK's message dialog.
 * A modal one blocks the example's window until a button closes it; then the
 * getters say how it was closed.
 */
export function MessageDialogView({ platform, options, initialTab = 'compose' }: MessageDialogViewProps) {
  const os = osOf(platform)
  const { state, actions, extended, lastEvent, log } = useMessageDialog(platform, options)
  const [tab, setTab] = useState<Tab>(initialTab)
  const dialog = state.open
  // open() blocks the call for a modal dialog (and every NSAlert); a ContentDialog
  // with a parent disables the parent's controls even when modeless.
  const blocking = dialog ? blocks(dialog) || (dialog.backend === 'winui3' && dialog.config.parent) : false
  // A ContentDialog with a parent sits in the parent's client area, over its smoke.
  const inWindow = dialog && (dialog.sheet || (dialog.backend === 'winui3' && dialog.config.parent))
  const activePreset = PRESETS.find(p => p.config.title === state.config.title)?.id

  const surface = dialog && (
    <DialogSurface
      platform={platform}
      dialog={dialog}
      live={state.config}
      refused={state.refused}
      onPress={actions.press}
      onEdit={actions.edit}
    />
  )

  const exampleWindow = (
    <ExampleWindow
      platform={platform}
      appName="Message Dialog"
      width={WINDOW.width}
      height={WINDOW.height}
      inactive={Boolean(dialog) && !inWindow}
      title="Message Dialog"
      subtitle={os === 'macos' ? BACKEND_NAMES[state.backend] : undefined}
      toolbar={
        <SegmentedControl<Tab>
          size="small"
          items={[
            { value: 'compose', label: 'Compose' },
            { value: 'extended', label: 'Extended' },
            { value: 'result', label: 'Result' },
          ]}
          value={tab}
          onValueChange={setTab}
        />
      }
      trailing={
        <Button size="small" variant="filled" disabled={Boolean(dialog)} onClick={() => actions.open()}>
          <Icon icon={Open16Regular} />
          Open
        </Button>
      }
      sidebar={
        <>
          <SidebarGroup>
            <SidebarGroupLabel>Presets</SidebarGroupLabel>
            <SidebarGroupContent>
              <SidebarMenu>
                {PRESETS.map(preset => (
                  <SidebarMenuItem key={preset.id}>
                    <SidebarMenuButton
                      size="large"
                      active={preset.id === activePreset}
                      icon={<Icon icon={PRESET_ICONS[preset.id]} />}
                      onClick={() => actions.preset(preset.id)}
                    >
                      <SidebarRowLabel label={preset.label} detail={preset.note} />
                    </SidebarMenuButton>
                  </SidebarMenuItem>
                ))}
              </SidebarMenu>
            </SidebarGroupContent>
          </SidebarGroup>
          {state.runs.length > 0 && (
            <SidebarGroup>
              <SidebarGroupLabel>Closed</SidebarGroupLabel>
              <SidebarGroupContent>
                <SidebarMenu>
                  {state.runs.map(run => (
                    <SidebarMenuItem key={run.number}>
                      <SidebarMenuButton
                        icon={
                          <Icon
                            icon={run.result === 'none' ? Circle16Filled : CheckmarkCircle16Filled}
                            className={run.result === 'none' ? 'message-dialog-view__plain' : 'message-dialog-view__ok'}
                          />
                        }
                        onClick={() => setTab('result')}
                      >
                        <SidebarRowLabel
                          label={`#${run.number} ${run.pressed}`}
                          detail={RESULT_NAMES[run.result].replace('MessageDialogResult::', 'Result ')}
                        />
                      </SidebarMenuButton>
                    </SidebarMenuItem>
                  ))}
                </SidebarMenu>
              </SidebarGroupContent>
            </SidebarGroup>
          )}
        </>
      }
      sidebarFooter={
        <Button size="small" variant="normal" fullWidth disabled={Boolean(dialog)} onClick={() => actions.open(2000)}>
          <Icon icon={Timer16Regular} />
          Open, close() in 2 s
        </Button>
      }
      footer={<EventBar lastEvent={lastEvent} log={log} onClear={actions.clearLog} />}
    >
      <div className="example-window__scroll">
        {tab === 'compose' && <ComposePanel state={state} actions={actions} />}
        {tab === 'extended' && <ExtendedPanel state={state} actions={actions} extended={extended} windows={os === 'windows'} />}
        {tab === 'result' && <ResultPanel state={state} actions={actions} windows={os === 'windows'} />}
      </div>
    </ExampleWindow>
  )

  const dialogWidth = dialog?.backend === 'winui3' ? 380 : dialog?.backend === 'appkit' ? 260 : dialog?.backend === 'win32' ? 380 : 360

  return (
    <DesktopStage
      platform={platform}
      appName="Message Dialog Example"
      layout="free"
      hint={
        dialog
          ? blocks(dialog)
            ? 'The dialog is modal: clicks on the example’s window are refused'
            : blocking
              ? 'The ContentDialog covers its parent: the window’s controls wait for it'
              : 'Modeless: the example’s window stays usable, and the dialog follows its changes'
          : 'Open a dialog from the example'
      }
    >
      <DesktopWindow x={WINDOW.x} y={WINDOW.y} z={1}>
        <div className="message-dialog-view__host">
          {exampleWindow}
          {blocking && (
            <div
              className="message-dialog-view__blocker"
              data-smoke={dialog?.backend === 'winui3' && dialog.config.parent ? '' : undefined}
              onPointerDown={e => {
                e.preventDefault()
                e.stopPropagation()
                actions.refuse()
              }}
            />
          )}
          {inWindow && <div className={dialog?.sheet ? 'message-dialog-view__sheet' : 'message-dialog-view__in-window'}>{surface}</div>}
        </div>
      </DesktopWindow>
      {dialog && !inWindow && (
        <DesktopWindow
          // A parentless ContentDialog gets a host window of its own, beside the example.
          x={dialog.backend === 'winui3' ? WINDOW.x + WINDOW.width + 32 : WINDOW.x + WINDOW.width / 2 - dialogWidth / 2}
          y={150}
          z={3}
        >
          {surface}
        </DesktopWindow>
      )}
    </DesktopStage>
  )
}
