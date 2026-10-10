import {
  CheckmarkCircle16Filled,
  DismissCircle16Filled,
  Globe20Regular,
  Link20Regular,
  Mail20Regular,
  Open16Regular,
} from '@fluentui/react-icons'
import { useEffect, useState } from 'react'

import {
  Badge,
  Button,
  Callout,
  Icon,
  PreferenceRow,
  PreferenceSection,
  SegmentedControl,
  SidebarGroup,
  SidebarGroupContent,
  SidebarGroupLabel,
  SidebarMenu,
  SidebarMenuButton,
  SidebarMenuItem,
  TextField,
  type WindowFramePlatform,
} from '@dazzlabs/dazzui'

import { DesktopStage, DesktopWindow } from '../../components/desktop-stage'
import { EventBar, useEventLog } from '../../components/event-bar'
import { ExampleWindow, SidebarRowLabel } from '../../components/example-window'
import { Panel, PanelPreferences } from '../../components/panel'
import { osOf } from '../../components/platform'
import { ReadBack } from '../../components/read-back'
import { HandlerWindow } from './components/handler-window'
import { ERROR_CODES, errorName, INITIAL_URL, PRESETS } from './data'
import { schemeOf, simulateCanOpen, simulateOpen } from './open-url'
import type { OpenAttempt, Tab } from './types'
import './url-opener-view.css'

export interface UrlOpenerViewProps {
  platform: WindowFramePlatform
  /** What the field holds when the example starts. */
  initialUrl?: string
  /** Open the initial URL straight away, as if Open URL had been pressed. */
  openAtStart?: boolean
  initialTab?: Tab
}

/** A URL that comes back with each error code, for the Error codes tab's Try buttons. */
const SAMPLE_FOR: Partial<Record<OpenAttempt['result']['errorCode'], string>> = {
  none: 'https://flutter.dev',
  invalidUrlEmpty: '',
  invalidUrlMissingScheme: 'not a url',
  invalidUrlUnsupportedScheme: 'gopher://example.com',
  invocationFailed: 'tel:+15551234567',
}

/**
 * The URL opener example: hand a URL to the system and see who takes it.
 * The window holds the field, the presets and every attempt so far; what
 * `open` returns is read back under the field, and the app the system
 * handed the URL to opens on the desktop beside the example.
 */
export function UrlOpenerView({ platform, initialUrl = INITIAL_URL, openAtStart, initialTab = 'open' }: UrlOpenerViewProps) {
  const os = osOf(platform)
  const { lastEvent, log, event, call, clear } = useEventLog(['isSupported() → true'], 'No URL opened yet')
  const [url, setUrl] = useState(initialUrl)
  useEffect(() => {
    if (!openAtStart) return
    const { result } = simulateOpen(initialUrl, os)
    call(`open("${initialUrl}") → ${result.success} · ${errorName(result.errorCode)}`)
    event(result.success ? `Opened ${initialUrl}` : `Could not open ${initialUrl}`)
    // Once, as the example starts.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [])
  const [tab, setTab] = useState<Tab>(initialTab)
  const [attempts, setAttempts] = useState<OpenAttempt[]>(() => {
    if (!openAtStart) return []
    const { result, handler } = simulateOpen(initialUrl, os)
    return [{ number: 1, url: initialUrl, result, handler }]
  })
  const [shown, setShown] = useState<number | null>(attempts[0]?.number ?? null)
  const [handlerOpen, setHandlerOpen] = useState(Boolean(attempts[0]?.handler))
  const [front, setFront] = useState<'example' | 'handler'>('handler')

  const current = attempts.find(a => a.number === shown) ?? null
  const latest = attempts[0] ?? null
  const canOpen = simulateCanOpen(url, os)
  const scheme = schemeOf(url)

  const open = () => {
    const { result, handler } = simulateOpen(url, os)
    const attempt: OpenAttempt = { number: (latest?.number ?? 0) + 1, url, result, handler }
    setAttempts(list => [attempt, ...list].slice(0, 12))
    setShown(attempt.number)
    call(`open("${url}") → ${result.success} · ${errorName(result.errorCode)}`)
    if (result.success) {
      event(`Opened ${url}`)
      setHandlerOpen(true)
      setFront('handler')
    } else {
      event(`Could not open ${url || 'an empty URL'}`)
    }
  }

  const fill = (value: string) => {
    setUrl(value)
    call(`canOpen("${value}") → ${simulateCanOpen(value, os)}`)
  }

  const exampleWindow = (
    <ExampleWindow
      platform={platform}
      appName="URL Opener"
      width={700}
      height={500}
      inactive={handlerOpen && front === 'handler'}
      title="Open a URL"
      subtitle="UrlOpener"
      toolbar={
        <SegmentedControl<Tab>
          size="small"
          items={[
            { value: 'open', label: 'Open' },
            { value: 'errors', label: 'Error codes' },
          ]}
          value={tab}
          onValueChange={setTab}
        />
      }
      trailing={
        <Badge size="small" variant="tinted" tint="success">
          Supported
        </Badge>
      }
      sidebar={
        <>
          <SidebarGroup>
            <SidebarGroupLabel>Presets</SidebarGroupLabel>
            <SidebarGroupContent>
              <SidebarMenu>
                {PRESETS.map(preset => (
                  <SidebarMenuItem key={preset.url}>
                    <SidebarMenuButton
                      size="large"
                      active={preset.url === url}
                      icon={<Icon icon={preset.url.startsWith('mailto') ? Mail20Regular : preset.url.startsWith('http') ? Globe20Regular : Link20Regular} />}
                      onClick={() => fill(preset.url)}
                    >
                      <SidebarRowLabel label={preset.note} detail={preset.url} />
                    </SidebarMenuButton>
                  </SidebarMenuItem>
                ))}
              </SidebarMenu>
            </SidebarGroupContent>
          </SidebarGroup>
          {attempts.length > 0 && (
            <SidebarGroup>
              <SidebarGroupLabel>Opened</SidebarGroupLabel>
              <SidebarGroupContent>
                <SidebarMenu>
                  {attempts.map(attempt => (
                    <SidebarMenuItem key={attempt.number}>
                      <SidebarMenuButton
                        active={attempt.number === shown}
                        icon={
                          <Icon
                            icon={attempt.result.success ? CheckmarkCircle16Filled : DismissCircle16Filled}
                            className={attempt.result.success ? 'url-opener-view__ok' : 'url-opener-view__fail'}
                          />
                        }
                        onClick={() => setShown(attempt.number)}
                      >
                        <SidebarRowLabel label={attempt.url || '(empty)'} detail={errorName(attempt.result.errorCode)} />
                      </SidebarMenuButton>
                    </SidebarMenuItem>
                  ))}
                </SidebarMenu>
              </SidebarGroupContent>
            </SidebarGroup>
          )}
        </>
      }
      footer={<EventBar lastEvent={lastEvent} log={log} onClear={clear} />}
    >
      <div className="example-window__scroll">
        {tab === 'open' ? (
          <Panel>
            <form
              className="url-opener-view__field"
              onSubmit={e => {
                e.preventDefault()
                open()
              }}
            >
              <TextField
                mono
                value={url}
                placeholder="https://example.com"
                aria-label="URL"
                onChange={e => setUrl((e.target as HTMLInputElement).value)}
              />
              <Button type="submit" variant="filled">
                <Icon icon={Open16Regular} />
                Open URL
              </Button>
            </form>
            <p className="example-panel__note">
              canOpen → <strong>{String(canOpen)}</strong>
              {scheme ? ` · scheme ${scheme}:` : ' · no scheme'} · handed to the system as is
            </p>
            {current ? (
              <>
                <ReadBack
                  label={`Result of open #${current.number}`}
                  action={
                    <Badge size="small" variant="tinted" tint={current.result.success ? 'success' : 'danger'}>
                      {current.result.success ? 'Opened' : 'Failed'}
                    </Badge>
                  }
                  columns={1}
                  keyWidth="7rem"
                  rows={[
                    ['url', `"${current.url}"`],
                    ['success', String(current.result.success)],
                    ['errorCode', errorName(current.result.errorCode)],
                    ['errorMessage', current.result.errorMessage ? `"${current.result.errorMessage}"` : '""'],
                    ['handled by', current.handler ?? '—'],
                  ]}
                />
                {!current.result.success && (
                  <Callout size="small" tint="danger" title={`Could not open ${current.url || 'an empty URL'}`}>
                    {ERROR_CODES.find(c => c.code === current.result.errorCode)?.when}
                  </Callout>
                )}
              </>
            ) : (
              <p className="example-panel__note">Nothing opened yet. Pick a preset or type a URL, then press Open URL.</p>
            )}
          </Panel>
        ) : (
          <Panel>
            <PanelPreferences>
              <PreferenceSection label="UrlOpenErrorCode">
                {ERROR_CODES.map(entry => (
                  <PreferenceRow
                    key={entry.code}
                    title={<span className="url-opener-view__code">{entry.name}</span>}
                    subtitle={entry.when}
                    icon={
                      latest?.result.errorCode === entry.code ? (
                        <Icon
                          icon={entry.code === 'none' ? CheckmarkCircle16Filled : DismissCircle16Filled}
                          className={entry.code === 'none' ? 'url-opener-view__ok' : 'url-opener-view__fail'}
                        />
                      ) : (
                        <span className="url-opener-view__dot" />
                      )
                    }
                  >
                    {entry.code in SAMPLE_FOR && (entry.code !== 'invocationFailed' || os === 'linux') ? (
                      <Button
                        size="small"
                        variant="normal"
                        onClick={() => {
                          fill(SAMPLE_FOR[entry.code]!)
                          setTab('open')
                        }}
                      >
                        Try
                      </Button>
                    ) : (
                      <Badge size="small" variant="outlined">
                        {entry.code === 'unsupportedPlatform' ? 'Android, iOS…' : 'Linux only'}
                      </Badge>
                    )}
                  </PreferenceRow>
                ))}
              </PreferenceSection>
            </PanelPreferences>
          </Panel>
        )}
      </div>
    </ExampleWindow>
  )

  return (
    <DesktopStage
      platform={platform}
      appName="URL Opener Example"
      layout="free"
      onPress={() => setFront('example')}
    >
      <DesktopWindow x={48} y={40} z={front === 'example' ? 2 : 1} onPress={() => setFront('example')}>
        {exampleWindow}
      </DesktopWindow>
      {handlerOpen && latest?.handler && (
        <DesktopWindow x={600} y={150} z={front === 'handler' ? 2 : 1} onPress={() => setFront('handler')}>
          <HandlerWindow
            platform={platform}
            handler={latest.handler}
            url={latest.url}
            inactive={front !== 'handler'}
            onClose={() => setHandlerOpen(false)}
          />
        </DesktopWindow>
      )}
    </DesktopStage>
  )
}
