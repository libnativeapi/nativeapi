import type { CSSProperties, ReactNode } from 'react'

import {
  cx,
  Sidebar,
  SidebarContent,
  SidebarFooter,
  SidebarHeader,
  SidebarProvider,
  WindowBody,
  WindowContent,
  WindowFooter,
  WindowFrame,
  WindowMain,
  WindowTitlebar,
  type WindowFramePlatform,
} from '@dazzlabs/dazzui'

import { osOf } from '../platform'
import './example-window.css'

export interface ExampleWindowProps {
  platform: WindowFramePlatform
  /** The window's title, and the app's name over the sidebar where no traffic lights sit. */
  appName: string
  /** The pane band's title: what the pane shows. */
  title: ReactNode
  subtitle?: ReactNode
  /** In the band after the title: the pane's tabs. */
  toolbar?: ReactNode
  /** At the band's trailing edge: the pane's actions. */
  trailing?: ReactNode
  /** The sidebar's groups. */
  sidebar: ReactNode
  /** Under the sidebar's groups: its one action. */
  sidebarFooter?: ReactNode
  /** The window's foot, usually the `EventBar`. */
  footer?: ReactNode
  width?: number
  height?: number
  /** Not the key window: the controls and the title grey out. */
  inactive?: boolean
  className?: string
  contentClassName?: string
  style?: CSSProperties
  onClose?: () => void
  children?: ReactNode
}

/**
 * Every example's window: a sidebar from the window's top edge to its foot,
 * and the pane beside it with its own band — the title leading, the tabs and
 * the actions trailing — its content, and the event bar at its foot. On macOS
 * the traffic lights sit over the sidebar's head; elsewhere the caption
 * buttons sit at the end of the pane's band, and Hyprland draws none.
 */
export function ExampleWindow({
  platform,
  appName,
  title,
  subtitle,
  toolbar,
  trailing,
  sidebar,
  sidebarFooter,
  footer,
  width = 800,
  height = 560,
  inactive,
  className,
  contentClassName,
  style,
  onClose,
  children,
}: ExampleWindowProps) {
  const mac = osOf(platform) === 'macos'
  const bandControls = !mac && platform !== 'omarchy'

  return (
    <WindowFrame
      platform={platform}
      title={appName}
      fullSizeContent
      titlebar={false}
      width={width}
      height={height}
      inactive={inactive}
      className={cx('example-window', className)}
      style={style}
      controls={mac ? undefined : false}
      onClose={onClose}
    >
      <WindowBody>
        <SidebarProvider>
          <Sidebar>
            <SidebarHeader>{!mac && <span className="example-window__app-name">{appName}</span>}</SidebarHeader>
            <SidebarContent>{sidebar}</SidebarContent>
            {sidebarFooter && <SidebarFooter>{sidebarFooter}</SidebarFooter>}
          </Sidebar>
          <WindowMain>
            <WindowTitlebar
              title={title}
              subtitle={subtitle}
              controls={bandControls ? { close: true, minimize: true, maximize: true } : false}
              onClose={onClose}
              toolbar={toolbar}
              // The trailing slot is what keeps the caption buttons' room on
              // GNOME and KDE, where they float over the band's end: an empty
              // one still keeps it.
              trailing={trailing ?? <span aria-hidden />}
            />
            <WindowContent className={cx('example-window__content', contentClassName)}>{children}</WindowContent>
            {footer && <WindowFooter>{footer}</WindowFooter>}
          </WindowMain>
        </SidebarProvider>
      </WindowBody>
    </WindowFrame>
  )
}

/** A sidebar row's label: its name, and a muted second line. */
export function SidebarRowLabel({ label, detail }: { label: ReactNode; detail?: ReactNode }) {
  return (
    <span className="example-window__row">
      <span>{label}</span>
      {detail && <span className="example-window__row-detail">{detail}</span>}
    </span>
  )
}
