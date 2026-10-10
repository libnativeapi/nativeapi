import {
  Add16Regular,
  Database20Regular,
  Delete16Regular,
  Folder20Regular,
  Key20Regular,
  LockClosed20Regular,
  Settings20Regular,
} from '@fluentui/react-icons'
import { useState } from 'react'

import {
  Badge,
  Button,
  Card,
  Dialog,
  DialogFooter,
  DialogHeader,
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

import { DesktopStage } from '../../components/desktop-stage'
import { EventBar } from '../../components/event-bar'
import { ExampleWindow } from '../../components/example-window'
import { osOf } from '../../components/platform'
import { BackendPanel } from './components/backend-panel'
import { EntriesPanel } from './components/entries-panel'
import { EntryDialog } from './components/entry-dialog'
import { LookupPanel } from './components/lookup-panel'
import { TestsPanel } from './components/tests-panel'
import { classOf, locationOf, PLAYGROUND_ENTRIES, SECURE_BACKENDS, STORES } from './data'
import type { StoreId, Stores, Tab } from './types'
import { useStorage } from './use-storage'
import './storage-view.css'

export interface StorageViewProps {
  platform: WindowFramePlatform
  /** What each store holds as the example starts. */
  initialEntries?: Stores
  initialStore?: StoreId
  initialTab?: Tab
  /** What `SecureStorage.isAvailable()` returns; false behaves as core's stub does today. */
  secureAvailable?: boolean
  /** Run every test case as the example starts. */
  runTestsAtStart?: boolean
}

const STORE_ICONS: Record<StoreId, typeof Settings20Regular> = {
  preferences: Settings20Regular,
  scoped_preferences: Folder20Regular,
  secure_storage: LockClosed20Regular,
  scoped_secure_storage: Key20Regular,
}

/**
 * The storage example: four stores side by side — Preferences and
 * SecureStorage, each unscoped and scoped. Pick one in the sidebar; the pane
 * reads and edits its entries, looks keys up one at a time, runs the test
 * cases against it, and shows where its data actually lives on this desktop.
 */
export function StorageView({
  platform,
  initialEntries = PLAYGROUND_ENTRIES,
  initialStore,
  initialTab = 'entries',
  secureAvailable = true,
  runTestsAtStart,
}: StorageViewProps) {
  const os = osOf(platform)
  const state = useStorage({ os, initial: initialEntries, initialStore, secureAvailable, runTestsAtStart })
  const { info, entries, stores, tests, actions, log } = state
  const [tab, setTab] = useState<Tab>(initialTab)
  const [editing, setEditing] = useState<{ key?: string } | null>(null)
  const [confirmClear, setConfirmClear] = useState(false)

  const count = Object.keys(entries).length
  const results = Object.values(tests)
  const passed = results.filter(r => r.status === 'pass').length
  const failed = results.some(r => r.status === 'fail')
  const ran = results.some(r => r.status !== 'open')
  const storeName = `${classOf(info)} · ${info.label}`

  const group = (label: string, kind: 'preferences' | 'secure') => (
    <SidebarGroup>
      <SidebarGroupLabel>{label}</SidebarGroupLabel>
      <SidebarGroupContent>
        <SidebarMenu>
          {STORES.filter(s => s.kind === kind).map(store => (
            <SidebarMenuItem key={store.id}>
              <SidebarMenuButton
                active={store.id === info.id}
                icon={<Icon icon={STORE_ICONS[store.id]} />}
                onClick={() => actions.select(store.id)}
              >
                {store.label}
              </SidebarMenuButton>
              <SidebarMenuBadge>{Object.keys(stores[store.id]).length}</SidebarMenuBadge>
            </SidebarMenuItem>
          ))}
        </SidebarMenu>
      </SidebarGroupContent>
    </SidebarGroup>
  )

  const appWindow = (
    <ExampleWindow
      platform={platform}
      appName="Storage"
      width={880}
      height={580}
      className="storage-view"
      title={info.label}
      toolbar={
        <SegmentedControl<Tab>
          size="small"
          tint={failed ? 'danger' : 'primary'}
          items={[
            { value: 'entries', label: 'Entries' },
            { value: 'lookup', label: 'Lookup' },
            { value: 'tests', label: ran ? `Tests ${passed}/${results.length}` : 'Tests' },
            { value: 'backend', label: 'Backend' },
          ]}
          value={tab}
          onValueChange={setTab}
        />
      }
      trailing={
        <>
          <IconButton
            label="Clear all"
            size="small"
            variant="plain"
            tint="danger"
            disabled={count === 0}
            onClick={() => setConfirmClear(true)}
          >
            <Icon icon={Delete16Regular} />
          </IconButton>
          {/* Where the caption buttons share the band, Add entry gives up its label. */}
          {os === 'macos' || platform === 'omarchy' ? (
            <Button size="small" variant="filled" onClick={() => setEditing({})}>
              <Icon icon={Add16Regular} />
              Add entry
            </Button>
          ) : (
            <IconButton label="Add entry" size="small" variant="filled" onClick={() => setEditing({})}>
              <Icon icon={Add16Regular} />
            </IconButton>
          )}
        </>
      }
      sidebar={
        <>
          {group('Preferences', 'preferences')}
          {group('SecureStorage', 'secure')}
        </>
      }
      sidebarFooter={
        <Card variant="sunken" size="small" className="storage-view__secure">
          <span className="storage-view__secure-head">
            <Icon icon={Database20Regular} size={16} />
            SecureStorage
          </span>
          <Badge size="small" variant="tinted" tint={secureAvailable ? 'success' : 'warning'}>
            isAvailable → {String(secureAvailable)}
          </Badge>
          <span className="storage-view__secure-note">
            {(['macos', 'windows', 'linux'] as const).map((o, i) => (
              <span key={o} data-current={o === os || undefined}>
                {SECURE_BACKENDS[o]}
                {i < 2 ? ' · ' : ''}
              </span>
            ))}
          </span>
        </Card>
      }
      footer={<EventBar lastEvent={log.lastEvent} log={log.log} onClear={log.clear} />}
    >
      {/* What the store is, whichever tab is showing. */}
      <div className="storage-view__strip">
        <span className="storage-view__class">{classOf(info)}</span>
        <Badge size="small" variant="outlined">
          {info.scoped ? `scope: ${info.scope}` : 'no scope'}
        </Badge>
        {info.kind === 'secure' && (
          <Badge size="small" variant="tinted" tint={secureAvailable ? 'success' : 'warning'}>
            {secureAvailable ? 'encrypted' : 'not available'}
          </Badge>
        )}
        <span className="storage-view__count">
          {count} {count === 1 ? 'item' : 'items'}
        </span>
        <span className="storage-view__location" title={locationOf(info, os)}>
          {locationOf(info, os)}
        </span>
      </div>
      {tab === 'entries' ? (
        <EntriesPanel
          entries={entries}
          storeLabel={info.label}
          changed={state.changed?.keys}
          flash={state.changed?.n}
          onEdit={key => setEditing({ key })}
          onRemove={actions.remove}
          onAddSamples={() => actions.runTest('bulk')}
        />
      ) : (
        <div className="example-window__scroll">
          {tab === 'lookup' && <LookupPanel state={state} />}
          {tab === 'tests' && (
            <TestsPanel
              tests={tests}
              storeLabel={info.label}
              onRun={actions.runTest}
              onRunAll={actions.runAll}
              onReset={actions.resetTests}
            />
          )}
          {tab === 'backend' && <BackendPanel state={state} />}
        </div>
      )}

      {editing && (
        <EntryDialog
          storeName={storeName}
          initialKey={editing.key}
          initialValue={editing.key !== undefined ? entries[editing.key] : ''}
          onClose={() => setEditing(null)}
          onSet={actions.set}
        />
      )}
      {confirmClear && (
        <Dialog open tone="danger" onOpenChange={open => !open && setConfirmClear(false)} width={360}>
          <DialogHeader title={`Clear ${info.label}?`} subtitle={`Removes all ${count} entries from ${storeName}.`} />
          <DialogFooter>
            <Button variant="normal" onClick={() => setConfirmClear(false)}>
              Cancel
            </Button>
            <Button
              variant="filled"
              tint="danger"
              onClick={() => {
                actions.clear()
                setConfirmClear(false)
              }}
            >
              Clear all
            </Button>
          </DialogFooter>
        </Dialog>
      )}
    </ExampleWindow>
  )

  return (
    <DesktopStage platform={platform} appName="Storage Example">
      {appWindow}
    </DesktopStage>
  )
}
