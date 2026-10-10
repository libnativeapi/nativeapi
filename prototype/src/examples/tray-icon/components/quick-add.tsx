import { Add20Regular } from '@fluentui/react-icons'
import { useState } from 'react'

import { Button, Icon, Popover, SectionLabel } from '@dazzlabs/dazzui'

import { SCENES } from '../data'
import { TEMPLATES } from '../sign-data'
import type { Capabilities } from '../types'
import type { TrayActions } from '../use-tray'
import './quick-add.css'

export function QuickAdd({ caps, actions, onAdded }: { caps: Capabilities; actions: TrayActions; onAdded: () => void }) {
  const [open, setOpen] = useState(false)
  const add = (action: () => void) => {
    action()
    setOpen(false)
    onAdded()
  }

  return <Popover title="Add tray item" side="top" align="start" width={260}
    open={open} onOpenChange={setOpen}
    trigger={<Button size="small" variant="normal" fullWidth><Icon icon={Add20Regular} />Add</Button>}>
    <div className="tray-quick-add">
      <SectionLabel>Icons</SectionLabel>
      <div className="tray-quick-add__items">
        <Button size="small" variant="normal" fullWidth onClick={() => add(actions.addIcon)}>Default icon</Button>
        {SCENES.map(scene => <Button key={scene.value} size="small" variant="normal" fullWidth
          onClick={() => add(() => actions.addScene(scene.value))}>{scene.label}</Button>)}
        <Button size="small" variant="normal" fullWidth onClick={() => add(actions.addThreeIcons)}>Three icons</Button>
      </div>
      <SectionLabel>Signs</SectionLabel>
      <div className="tray-quick-add__items">
        {TEMPLATES.map(template => <Button key={template.value} size="small" variant="normal" fullWidth
          disabled={!caps.contentView} title={caps.contentView ? undefined : 'Custom tray content is available on macOS.'}
          onClick={() => add(() => actions.addSign(template.value))}>{template.label}</Button>)}
      </div>
    </div>
  </Popover>
}
