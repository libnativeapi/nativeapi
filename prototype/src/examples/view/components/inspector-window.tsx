import {
  Button16Regular,
  Image16Regular,
  Square16Regular,
  Textbox16Regular,
  TextT16Regular,
} from '@fluentui/react-icons'
import { useEffect, useMemo, useRef, useState } from 'react'

import { Badge, Icon, Tree, type TreeNode, WindowFrame, type WindowFramePlatform } from '@dazzlabs/dazzui'

import { frameText } from '../stack-layout'
import type { LogEntry, ViewKind, ViewNode, ViewTree } from '../types'
import { flattenTree } from '../view-tree'
import { NButton, NField, NLabel, NView } from './native-view'
import './inspector-window.css'

export interface InspectorWindowProps {
  platform: WindowFramePlatform
  tree: ViewTree
  entries: readonly LogEntry[]
  selected: string | null
  onSelect: (name: string) => void
  query: string
  /** What the finder says under the field: the match, no match, or the hint. */
  found: string
  onQuery: (query: string) => void
  onFlash: () => void
  onRefresh: () => void
  onClearLog: () => void
  inactive?: boolean
  onClose: () => void
}

const KIND_ICONS: Record<ViewKind, typeof Square16Regular> = {
  View: Square16Regular,
  Label: TextT16Regular,
  Button: Button16Regular,
  TextField: Textbox16Regular,
  ImageView: Image16Regular,
}

/** What `describeTree()` prints after a view's name and type, frame aside. */
function detailsOf(node: ViewNode) {
  const parts: string[] = []
  if (node.layout) parts.push(`${node.layout} gap ${node.gap ?? 0}`)
  if (node.flex > 0) parts.push(`flex ${node.flex}`)
  if (node.hidden) parts.push('hidden')
  if (node.disabled) parts.push('disabled')
  if (node.focused) parts.push('focused')
  if (node.text) parts.push(`"${node.text}"`)
  return parts.join(' · ')
}

function toTreeNode(node: ViewNode): TreeNode {
  const details = detailsOf(node)
  return {
    id: node.name,
    text: node.name,
    icon: <Icon icon={KIND_ICONS[node.kind]} size={14} />,
    label: (
      <span className="inspector-window__node" data-hidden={node.hidden ? '' : undefined}>
        <span className="inspector-window__node-name">{node.name}</span>
        <span className="inspector-window__node-kind">{node.kind}</span>
        {details && <span className="inspector-window__node-details">{details}</span>}
      </span>
    ),
    meta: <span className="inspector-window__frame">{frameText(node.frame)}</span>,
    children: node.children.length > 0 ? node.children.map(toTreeNode) : undefined,
  }
}

/**
 * View Inspector: a second window, also native views, that watches the
 * workbench — the live tree of its views with every frame, a finder that
 * flashes a view by name, and every view event in order, newest first.
 */
export function InspectorWindow({
  platform,
  tree,
  entries,
  selected,
  onSelect,
  query,
  found,
  onQuery,
  onFlash,
  onRefresh,
  onClearLog,
  inactive,
  onClose,
}: InspectorWindowProps) {
  const items = useMemo(() => (tree.root ? [toTreeNode(tree.root)] : []), [tree])
  const branches = useMemo(() => flattenTree(tree.root).filter(n => n.children.length > 0).map(n => n.name), [tree])
  const [collapsed, setCollapsed] = useState<readonly string[]>([])
  const expanded = branches.filter(name => !collapsed.includes(name))
  // A view picked by the finder, or at start, is scrolled to.
  const treeBox = useRef<HTMLDivElement | null>(null)
  useEffect(() => {
    treeBox.current?.querySelector('[aria-selected="true"]')?.scrollIntoView({ block: 'nearest' })
  }, [selected, items.length > 0])

  return (
    <WindowFrame
      platform={platform}
      title="View Inspector"
      width={420}
      height={600}
      inactive={inactive}
      className="inspector-window"
      onClose={onClose}
    >
      <NView name="inspector.root" gap={8} padding={14} fill className="inspector-window__root">
        <NView name="inspector.header" layout="row" gap={8}>
          <NLabel name="inspector.heading" text="VIEW TREE" tone="accent" size="small" alignment="center" className="inspector-window__heading" />
          <NLabel
            name="inspector.stats"
            text={`${tree.count} views, ${tree.depth} levels deep`}
            tone="muted"
            size="small"
            flex={1}
            alignment="center"
          />
          <NButton name="inspector.refresh" text="Refresh" onPress={onRefresh} />
        </NView>
        <div ref={treeBox} className="inspector-window__box inspector-window__tree">
          <Tree
            size="small"
            items={items}
            expanded={expanded}
            onExpandedChange={open => setCollapsed(branches.filter(name => !open.includes(name)))}
            selected={selected}
            onSelectedChange={onSelect}
            aria-label="View tree"
          />
        </div>
        <NView name="inspector.finder" layout="row" gap={8}>
          <NField
            name="inspector.query"
            text={query}
            placeholder='Find a view by name, e.g. "submit" or "box"'
            flex={1}
            alignment="center"
            onChanged={onQuery}
            onSubmitted={onFlash}
          />
          <NButton name="inspector.flash" text="Flash" onPress={onFlash} />
        </NView>
        <NLabel name="inspector.found" text={found} tone="muted" size="small" />
        <NView name="inspector.logHeader" layout="row" gap={8}>
          <NLabel name="inspector.eventsHeading" text="EVENTS" tone="accent" size="small" flex={1} alignment="center" className="inspector-window__heading" />
          <NButton name="inspector.clear" text="Clear" onPress={onClearLog} />
        </NView>
        <ol className="inspector-window__box inspector-window__log">
          {entries.length === 0 && <li className="inspector-window__empty">No events.</li>}
          {[...entries].reverse().map(entry => (
            <li key={entry.id} data-note={entry.type ? undefined : ''} title={`${entry.source}  ${entry.message}`}>
              <span className="inspector-window__time">{entry.time}</span>
              <span className="inspector-window__source">{entry.source}</span>
              <span className="inspector-window__message">{entry.message}</span>
              {entry.repeats > 1 && (
                <Badge size="small" variant="tinted" tint="neutral">
                  ×{entry.repeats}
                </Badge>
              )}
            </li>
          ))}
        </ol>
      </NView>
    </WindowFrame>
  )
}
