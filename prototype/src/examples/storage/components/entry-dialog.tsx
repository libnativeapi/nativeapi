import { useState } from 'react'

import { Button, Dialog, DialogBody, DialogFooter, DialogHeader, FormField, TextField } from '@dazzlabs/dazzui'

export interface EntryDialogProps {
  /** Where the entry goes, under the title: "Preferences · user_settings". */
  storeName: string
  /** Editing an existing entry: its key is fixed. Absent to add one. */
  initialKey?: string
  initialValue?: string
  onClose: () => void
  /** `set(key, value)`; the dialog closes after. */
  onSet: (key: string, value: string) => void
}

/** Add or edit one entry. The key is fixed once an entry exists; Set is `set(key, value)`. */
export function EntryDialog({ storeName, initialKey, initialValue = '', onClose, onSet }: EntryDialogProps) {
  const editing = initialKey !== undefined
  const [key, setKey] = useState(initialKey ?? '')
  const [value, setValue] = useState(initialValue)
  const [missing, setMissing] = useState(false)

  const save = () => {
    if (!key.trim()) {
      setMissing(true)
      return
    }
    onSet(key, value)
    onClose()
  }

  return (
    <Dialog open onOpenChange={open => !open && onClose()} width={400}>
      <DialogHeader title={editing ? 'Edit entry' : 'Add entry'} subtitle={storeName} />
      <DialogBody>
        <FormField label="Key" state={missing ? 'error' : 'default'} hint={missing ? 'Key cannot be empty' : undefined}>
          <TextField
            mono
            autoFocus={!editing}
            disabled={editing}
            value={key}
            placeholder="e.g. theme"
            onChange={event => {
              setKey(event.target.value)
              setMissing(false)
            }}
            onKeyDown={event => event.key === 'Enter' && save()}
          />
        </FormField>
        <FormField label="Value">
          <TextField
            mono
            multiline
            rows={3}
            autoFocus={editing}
            value={value}
            placeholder="Any string"
            onChange={event => setValue(event.target.value)}
          />
        </FormField>
      </DialogBody>
      <DialogFooter>
        <Button variant="normal" onClick={onClose}>
          Cancel
        </Button>
        <Button variant="filled" onClick={save}>
          Set
        </Button>
      </DialogFooter>
    </Dialog>
  )
}
