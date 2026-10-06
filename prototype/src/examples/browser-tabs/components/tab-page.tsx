import {
  ArrowClockwise20Regular,
  ArrowLeft20Regular,
  ArrowRight20Regular,
  LockClosed16Regular,
  ThumbLike20Regular,
} from '@fluentui/react-icons'
import { type CSSProperties, useLayoutEffect, useRef } from 'react'

import { Button, Icon, IconButton, TextField } from '@dazzlabs/dazzui'

import { useNow } from '../../../components/use-now'
import { PARAGRAPHS, tabColor } from '../data'
import type { BrowserTab } from '../types'
import './tab-page.css'

export interface TabPageProps {
  tab: BrowserTab
  /** The page's state changed: the address, the likes. */
  onChange: () => void
}

/**
 * A stand-in for a web page, with the state a rebuilt page would lose: an
 * edited address, a like counter, a scroll position and the time it has been
 * open. It is drawn afresh in whichever window shows the tab; the state lives
 * on the tab, so `Page state #n` never changes and `moved between windows`
 * counts the moves.
 */
export function TabPage({ tab, onChange }: TabPageProps) {
  const now = useNow(1000)
  const scroller = useRef<HTMLDivElement>(null)
  const { page } = tab
  const seconds = Math.max(0, Math.floor(now - page.openedAt))

  // The scroll position comes back with the tab, in any window.
  useLayoutEffect(() => {
    if (scroller.current) scroller.current.scrollTop = page.scroll
  }, [page])

  return (
    <div className="tab-page" style={{ '--tab-color': tabColor(tab.hue) } as CSSProperties}>
      {/* Back, forward and reload have nowhere to go on a stand-in page; the
          address is a real field whose edits travel with the tab. */}
      <div className="tab-page__toolbar">
        <IconButton label="Back" size="small" variant="plain" disabled>
          <Icon icon={ArrowLeft20Regular} />
        </IconButton>
        <IconButton label="Forward" size="small" variant="plain" disabled>
          <Icon icon={ArrowRight20Regular} />
        </IconButton>
        <IconButton label="Reload" size="small" variant="plain" disabled>
          <Icon icon={ArrowClockwise20Regular} />
        </IconButton>
        <span className="tab-page__address">
          <Icon icon={LockClosed16Regular} size={14} className="tab-page__lock" />
          <TextField
            size="small"
            value={page.address}
            aria-label="Address"
            onChange={event => {
              page.address = (event.target as HTMLInputElement).value
              onChange()
            }}
          />
        </span>
      </div>
      <div
        ref={scroller}
        className="tab-page__scroll"
        onScroll={event => {
          page.scroll = event.currentTarget.scrollTop
        }}
      >
        <header className="tab-page__head">
          <h2 className="tab-page__title">{tab.title}</h2>
          <code className="tab-page__state">
            Page state #{page.instance} · open for {seconds}s · moved between windows {page.moves}×
          </code>
          <div className="tab-page__actions">
            <Button
              size="small"
              variant="tinted"
              onClick={() => {
                page.likes++
                onChange()
              }}
            >
              <Icon icon={ThumbLike20Regular} />
              Like ({page.likes})
            </Button>
            <span className="tab-page__hint">
              Drag the tab to reorder, pull it down to tear it off, drop it on another strip to merge.
            </span>
          </div>
        </header>
        {Array.from({ length: PARAGRAPHS }, (_, i) => (
          <div key={i} className="tab-page__paragraph" data-shade={i % 3}>
            {tab.title} · paragraph {i + 1}
          </div>
        ))}
      </div>
    </div>
  )
}
