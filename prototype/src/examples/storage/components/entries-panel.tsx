import { Delete16Regular, Edit16Regular } from '@fluentui/react-icons'

import { Button, EmptyState, Icon, IconButton, Table, TableCell, TableHead, TableRow } from '@dazzlabs/dazzui'

import type { Entries } from '../types'
import './panels.css'

export interface EntriesPanelProps {
  entries: Entries
  storeLabel: string
  /** The key a call just touched: its row is picked out. */
  changed?: readonly string[]
  /** Bumps on every call, so a row written twice flashes twice. */
  flash?: number
  onEdit: (key?: string) => void
  onRemove: (key: string) => void
  onAddSamples: () => void
}

/**
 * The selected store's entries — what `getAll()` returns for its own
 * domain — one row each. A row opens the edit dialog; its trailing buttons
 * edit or remove it.
 */
export function EntriesPanel({ entries, storeLabel, changed = [], flash = 0, onEdit, onRemove, onAddSamples }: EntriesPanelProps) {
  const rows = Object.entries(entries)

  if (rows.length === 0) {
    return (
      <EmptyState
        className="storage-panel__empty"
        title={`No entries in ${storeLabel}`}
        action={
          <>
            <Button size="small" variant="filled" onClick={() => onEdit()}>
              Add entry
            </Button>
            <Button size="small" variant="normal" onClick={onAddSamples}>
              Add 10 samples
            </Button>
          </>
        }
      />
    )
  }

  return (
    <Table className="storage-table">
      <TableHead>
        <TableCell head className="storage-table__index">
          #
        </TableCell>
        <TableCell head className="storage-table__key">
          Key
        </TableCell>
        <TableCell head className="storage-table__value">
          Value
        </TableCell>
        <TableCell head align="end" className="storage-table__actions">
          Actions
        </TableCell>
      </TableHead>
      <div className="storage-table__body">
        {rows.map(([key, value], index) => (
          <TableRow
            key={changed.includes(key) ? `${key}#${flash}` : key}
            interactive
            data-changed={changed.includes(key) || undefined}
            className="storage-table__row"
            onClick={() => onEdit(key)}
          >
            <TableCell className="storage-table__index">{index + 1}</TableCell>
            <TableCell className="storage-table__key">{key}</TableCell>
            <TableCell className="storage-table__value" data-empty={value === '' || undefined}>
              {value === '' ? '(empty)' : value}
            </TableCell>
            <TableCell align="end" className="storage-table__actions">
              <span className="storage-table__buttons" onClick={event => event.stopPropagation()}>
                <IconButton label={`Edit ${key}`} size="tiny" variant="plain" onClick={() => onEdit(key)}>
                  <Icon icon={Edit16Regular} />
                </IconButton>
                <IconButton label={`Remove ${key}`} size="tiny" variant="plain" tint="danger" onClick={() => onRemove(key)}>
                  <Icon icon={Delete16Regular} />
                </IconButton>
              </span>
            </TableCell>
          </TableRow>
        ))}
      </div>
    </Table>
  )
}
