import {
  createRootRoute,
  HeadContent,
  Outlet,
  Scripts,
  useRouterState,
} from '@tanstack/react-router'
import type { ReactNode } from 'react'
import appCss from '../styles.css?url'
import { brandMarkSvg } from '@/components/brand-icon'
import { getMessages, localeFromPath } from '@/lib/i18n'

const themeInitScript = `(function () {
  var media = window.matchMedia('(prefers-color-scheme: dark)')
  var applyTheme = function () {
    document.documentElement.classList.toggle('dark', media.matches)
  }
  applyTheme()
  if (media.addEventListener) media.addEventListener('change', applyTheme)
})()`

const faviconSvg = `data:image/svg+xml,${encodeURIComponent(brandMarkSvg)}`

export const Route = createRootRoute({
  head: () => ({
    meta: [
      { charSet: 'utf-8' },
      { name: 'viewport', content: 'width=device-width, initial-scale=1' },
      { title: getMessages('en').metadata.title },
      { name: 'description', content: getMessages('en').metadata.description },
    ],
    links: [
      { rel: 'stylesheet', href: appCss },
      { rel: 'icon', type: 'image/svg+xml', href: faviconSvg },
    ],
    scripts: [{ children: themeInitScript }],
  }),
  component: RootComponent,
})

function RootComponent() {
  const pathname = useRouterState({ select: state => state.location.pathname })
  return (
    <RootDocument locale={localeFromPath(pathname)}>
      <Outlet />
    </RootDocument>
  )
}

function RootDocument({
  children,
  locale,
}: {
  children: ReactNode
  locale: 'en' | 'ja' | 'zh'
}) {
  return (
    <html
      lang={locale === 'zh' ? 'zh-CN' : locale}
      suppressHydrationWarning
    >
      <head>
        <HeadContent />
      </head>
      <body>
        {children}
        <Scripts />
      </body>
    </html>
  )
}
