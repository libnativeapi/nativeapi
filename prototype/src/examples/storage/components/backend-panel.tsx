import { Eye16Regular, EyeOff16Regular, Folder16Regular, Key16Regular, LockClosed16Regular } from '@fluentui/react-icons'
import { useState } from 'react'

import {
  Badge,
  Button,
  Callout,
  Card,
  Icon,
  SectionLabel,
  Table,
  TableCell,
  TableHead,
  TableRow,
} from '@dazzlabs/dazzui'

import { Panel } from '../../../components/panel'
import { MACOS_GLOBAL_KEYS, SECURE_BACKENDS } from '../data'
import type { StorageState } from '../use-storage'
import './panels.css'

export interface BackendPanelProps {
  state: StorageState
}

const xml = (s: string) => s.replaceAll('&', '&amp;').replaceAll('<', '&lt;').replaceAll('>', '&gt;')
const short = (s: string, n = 48) => (s.length > n ? `${s.slice(0, n - 1)}…` : s)

/**
 * Where the selected store's entries actually live on this desktop, drawn as
 * the system's own tools show them — the suite's plist, the registry key,
 * the XDG config file, the keychain's items — and redrawn on every call, the
 * keys the last call touched picked out.
 */
export function BackendPanel({ state }: BackendPanelProps) {
  const { info, os, entries, changed, secureAvailable } = state
  const touched = changed?.keys ?? []
  const flash = changed?.n ?? 0
  // A row written again is a new row, so its flash plays again.
  const rowKey = (key: string) => (touched.includes(key) ? `${key}#${flash}` : key)
  // Registry Editor, `plutil` and the Linux writer all list keys sorted.
  const sorted = Object.entries(entries).sort(([a], [b]) => a.localeCompare(b))

  if (info.kind === 'secure') {
    return (
      <Panel>
        <SecureView state={state} sorted={sorted} touched={touched} rowKey={rowKey} available={secureAvailable} />
      </Panel>
    )
  }

  if (os === 'windows') {
    const path = `Computer\\HKEY_CURRENT_USER\\Software\\NativeAPI\\Preferences\\${info.scope}`
    return (
      <Panel>
        <p className="example-panel__note">
          Registry Editor: one <code>REG_SZ</code> value per key under the scope's key. clear() deletes the key and makes
          it again.
        </p>
        <Card variant="outlined" size="small" className="storage-backend">
          <div className="storage-backend__address">{path}</div>
          <Table className="storage-backend__table">
            <TableHead>
              <TableCell head>Name</TableCell>
              <TableCell head className="storage-backend__type">
                Type
              </TableCell>
              <TableCell head>Data</TableCell>
            </TableHead>
            <TableRow className="storage-backend__row">
              <TableCell>(Default)</TableCell>
              <TableCell className="storage-backend__type">REG_SZ</TableCell>
              <TableCell className="storage-backend__faint">(value not set)</TableCell>
            </TableRow>
            {sorted.map(([key, value]) => (
              <TableRow key={rowKey(key)} className="storage-backend__row" data-changed={touched.includes(key) || undefined}>
                <TableCell>{key}</TableCell>
                <TableCell className="storage-backend__type">REG_SZ</TableCell>
                <TableCell>{short(value)}</TableCell>
              </TableRow>
            ))}
          </Table>
        </Card>
      </Panel>
    )
  }

  if (os === 'macos') {
    const domain = `com.nativeapi.preferences.${info.scope}`
    const lines: [string, string | null][] = [
      ['<?xml version="1.0" encoding="UTF-8"?>', null],
      ['<plist version="1.0">', null],
      ['<dict>', null],
      ...sorted.flatMap(([key, value]): [string, string][] => [
        [`\t<key>${xml(key)}</key>`, key],
        [`\t<string>${xml(short(value, 56))}</string>`, key],
      ]),
      ['</dict>', null],
      ['</plist>', null],
    ]
    return (
      <Panel>
        <FileView
          title={`~/Library/Preferences/${domain}.plist`}
          language="plist"
          note="plutil -convert xml1"
          flash={flash}
          lines={lines.map(([text, key]) => ({ text, changed: key !== null && touched.includes(key) }))}
        />
        <Callout size="small" tint="warning" title="getKeys() and getAll() see more than this file">
          NSUserDefaults merges NSGlobalDomain into the suite's dictionaryRepresentation, so they also return{' '}
          {MACOS_GLOBAL_KEYS.slice(0, 3)
            .map(([k]) => k)
            .join(', ')}{' '}
          and {MACOS_GLOBAL_KEYS.length - 3} more — {sorted.length + MACOS_GLOBAL_KEYS.length} in all.
        </Callout>
      </Panel>
    )
  }

  // Linux: one key=value line per entry, newlines escaped, under a comment naming the scope.
  return (
    <Panel>
      <FileView
        title={`~/.config/nativeapi/preferences_${info.scope}.conf`}
        language="conf"
        note="$XDG_CONFIG_HOME"
        flash={flash}
        lines={[
          { text: `# NativeAPI Preferences - ${info.scope}`, changed: false },
          ...sorted.map(([key, value]) => ({
            text: `${key}=${short(value.replaceAll('\n', '\\n'), 60)}`,
            changed: touched.includes(key),
          })),
        ]}
      />
      <p className="example-panel__note">
        The whole file is rewritten on every set, remove and clear; a value's newlines are stored as \n.
      </p>
    </Panel>
  )
}

interface FileViewProps {
  title: string
  language: string
  note?: string
  lines: { text: string; changed: boolean }[]
  flash: number
}

/** A file's text, a line per row, the lines the last call wrote picked out. */
function FileView({ title, language, note, lines, flash }: FileViewProps) {
  return (
    <Card variant="outlined" size="small" className="storage-file">
      <div className="storage-file__head">
        <Icon icon={Folder16Regular} />
        <span className="storage-file__title">{title}</span>
        {note && <span className="storage-file__note">{note}</span>}
        <span className="storage-file__language">{language}</span>
      </div>
      <ol className="storage-file__lines">
        {lines.map((line, i) => (
          <li key={line.changed ? `${i}-${line.text}-${flash}` : `${i}-${line.text}`} data-changed={line.changed || undefined}>
            <span className="storage-file__gutter">{i + 1}</span>
            <span>{line.text}</span>
          </li>
        ))}
      </ol>
    </Card>
  )
}

interface SecureViewProps {
  state: StorageState
  sorted: [string, string][]
  touched: string[]
  rowKey: (key: string) => string
  available: boolean
}

/** The secret store's own list: Keychain Access, Credential Manager or Seahorse, a row per item. */
function SecureView({ state, sorted, touched, rowKey, available }: SecureViewProps) {
  const { info, os } = state
  const [reveal, setReveal] = useState(false)
  const backend = SECURE_BACKENDS[os]

  if (!available) {
    return (
      <Callout size="small" tint="warning" title={`SecureStorage.isAvailable() → false`}>
        Nothing reaches {backend}: set() returns false and every read returns its default. Core's SecureStorage is a stub
        on every platform for now.
      </Callout>
    )
  }

  const columns =
    os === 'macos'
      ? { app: 'Keychain Access · login', name: 'Name', account: 'Account', kind: 'Kind' }
      : os === 'windows'
        ? { app: 'Credential Manager · Generic Credentials', name: 'Internet or network address', account: 'User name', kind: 'Persistence' }
        : { app: 'Passwords and Keys · Login keyring', name: 'Label', account: 'Attributes', kind: 'Kind' }

  const nameOf = (key: string) =>
    os === 'macos' ? `nativeapi.${info.scope}` : os === 'windows' ? `nativeapi/${info.scope}/${key}` : `${info.scope}: ${key}`
  const accountOf = (key: string) => (os === 'linux' ? `key=${key}` : key)
  const kind = os === 'macos' ? 'application password' : os === 'windows' ? 'Local computer' : 'Password'

  return (
    <>
      <div className="storage-panel__head">
        <SectionLabel>{columns.app}</SectionLabel>
        <Badge size="small" variant="tinted" tint="success">
          <Icon icon={LockClosed16Regular} size={12} /> Encrypted at rest
        </Badge>
        <Button size="small" variant="plain" onClick={() => setReveal(r => !r)}>
          <Icon icon={reveal ? EyeOff16Regular : Eye16Regular} />
          {reveal ? 'Hide secrets' : 'Show secrets'}
        </Button>
      </div>
      <Card variant="outlined" size="small" className="storage-backend">
        <Table className="storage-backend__table">
          <TableHead>
            <TableCell head>{columns.name}</TableCell>
            <TableCell head>{columns.account}</TableCell>
            <TableCell head className="storage-backend__kind">
              {columns.kind}
            </TableCell>
            <TableCell head>{os === 'macos' ? 'Password' : os === 'windows' ? 'Password' : 'Secret'}</TableCell>
          </TableHead>
          {sorted.length === 0 && (
            <TableRow className="storage-backend__row">
              <TableCell className="storage-backend__faint">No items for {info.scope}</TableCell>
            </TableRow>
          )}
          {sorted.map(([key, value]) => (
            <TableRow key={rowKey(key)} className="storage-backend__row" data-changed={touched.includes(key) || undefined}>
              <TableCell>
                <span className="storage-backend__item">
                  <Icon icon={Key16Regular} size={14} />
                  {nameOf(key)}
                </span>
              </TableCell>
              <TableCell>{accountOf(key)}</TableCell>
              <TableCell className="storage-backend__kind">{kind}</TableCell>
              <TableCell>{reveal ? short(value, 28) : '••••••••'}</TableCell>
            </TableRow>
          ))}
        </Table>
      </Card>
      <p className="example-panel__note">
        {os === 'macos'
          ? 'One generic password per key, the scope as its service and the key as its account.'
          : os === 'windows'
            ? 'One generic credential per key, protected with DPAPI for this user.'
            : 'One secret per key in the default collection, found again by its attributes.'}
      </p>
    </>
  )
}
