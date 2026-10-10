import { GithubIcon, MenuIcon, XIcon } from 'lucide-react'
import { useState } from 'react'
import { Button } from '@/components/ui/button'
import { Separator } from '@/components/ui/separator'
import {
  docsUrl,
  getMessages,
  githubUrl,
  localePath,
  localizedPath,
  type Locale,
} from '@/lib/i18n'
import { Container } from './container'
import { LocaleSwitcher } from './locale-switcher'
import { BrandLockup } from './brand-lockup'

export function SiteHeader({
  locale,
  pagePath = '/',
}: {
  locale: Locale
  pagePath?: string
}) {
  const [open, setOpen] = useState(false)
  const copy = getMessages(locale)
  const navigation = [
    {
      href: `${localizedPath(locale, '/')}#features`,
      label: copy.header.features,
    },
    { href: docsUrl(locale), label: copy.header.docs },
    { href: localizedPath(locale, '/api'), label: copy.header.api },
  ] as const

  return (
    <header
      className="fixed inset-x-0 top-0 z-50 h-(--header-height) w-full transition-all duration-300"
      style={{ backdropFilter: 'blur(8px)' }}
    >
      <Container className="flex h-full items-center justify-between gap-2">
        <Button
          variant="ghost"
          className="h-8 touch-manipulation justify-start gap-2.5 p-0! hover:bg-transparent lg:hidden"
          onClick={() => setOpen(value => !value)}
          aria-label={open ? copy.header.closeMenu : copy.header.openMenu}
        >
          {open ? <XIcon /> : <MenuIcon />}
          <span className="text-lg leading-none font-medium">
            {copy.header.menu}
          </span>
        </Button>
        <a
          href={localePath(locale)}
          className="hidden items-center gap-[5px] rounded-md transition-colors focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-ring/40 lg:flex"
        >
          <BrandLockup className="h-10 w-auto" />
        </a>
        <div className="flex-1" />
        <nav className="hidden items-center lg:flex">
          {navigation.map(item => (
            <Button
              key={item.href}
              variant="ghost"
              asChild
              className="text-base text-muted-foreground transition-colors hover:bg-transparent hover:text-foreground"
            >
              <a href={item.href}>{item.label}</a>
            </Button>
          ))}
        </nav>
        <div className="flex h-full items-center py-6 pr-4">
          <Separator orientation="vertical" className="hidden lg:block" />
        </div>
        <LocaleSwitcher locale={locale} pagePath={pagePath} />
        <Button
          asChild
          variant="ghost"
          size="icon"
          className="hidden sm:inline-flex"
          title="GitHub"
        >
          <a href={githubUrl} target="_blank" rel="noreferrer" aria-label="GitHub">
            <GithubIcon />
          </a>
        </Button>
        <Button asChild className="cursor-pointer">
          <a href={`${localizedPath(locale, '/')}#install`}>
            {copy.header.install}
          </a>
        </Button>
      </Container>
      {open && (
        <div className="h-[calc(100svh-var(--header-height))] border-t bg-background/90 px-6 py-6 backdrop-blur lg:hidden">
          <nav className="flex flex-col gap-3">
            <a
              href={localePath(locale)}
              className="text-2xl font-medium"
              onClick={() => setOpen(false)}
            >
              {copy.header.home}
            </a>
            {navigation.map(item => (
              <a
                key={item.href}
                href={item.href}
                className="text-2xl font-medium"
                onClick={() => setOpen(false)}
              >
                {item.label}
              </a>
            ))}
            <a
              href={githubUrl}
              target="_blank"
              rel="noreferrer"
              className="text-2xl font-medium"
              onClick={() => setOpen(false)}
            >
              GitHub
            </a>
          </nav>
        </div>
      )}
    </header>
  )
}
