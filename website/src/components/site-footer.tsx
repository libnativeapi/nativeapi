import { GithubIcon, MessageCircleIcon } from 'lucide-react'
import { Button } from '@/components/ui/button'
import {
  docsUrl,
  getMessages,
  githubUrl,
  localizedPath,
  type Locale,
} from '@/lib/i18n'
import { BrandIcon } from './brand-icon'
import { Container } from './container'

const socialLinks = [
  {
    label: 'GitHub',
    href: githubUrl,
    icon: GithubIcon,
  },
  {
    label: 'Issues',
    href: `${githubUrl}/issues`,
    icon: MessageCircleIcon,
  },
] as const

export function SiteFooter({ locale }: { locale: Locale }) {
  const copy = getMessages(locale)
  const columns = [
    {
      title: copy.footer.columns[0].title,
      links: [
        {
          label: copy.footer.columns[0].links[0],
          href: `${localizedPath(locale, '/')}#install`,
        },
        {
          label: copy.footer.columns[0].links[1],
          href: `${localizedPath(locale, '/')}#features`,
        },
        {
          label: copy.footer.columns[0].links[2],
          href: githubUrl,
          external: true,
        },
      ],
    },
    {
      title: copy.footer.columns[1].title,
      links: [
        { label: copy.footer.columns[1].links[0], href: `${localizedPath(locale, '/')}#faq` },
        {
          label: copy.footer.columns[1].links[1],
          href: docsUrl(locale),
        },
        {
          label: copy.footer.columns[1].links[2],
          href: localizedPath(locale, '/support'),
        },
      ],
    },
    {
      title: copy.footer.columns[2].title,
      links: [
        {
          label: copy.footer.columns[2].links[0],
          href: 'https://github.com/libnativeapi/nativeapi-core',
          external: true,
        },
        {
          label: copy.footer.columns[2].links[1],
          href: 'https://pub.dev/packages/window_manager',
          external: true,
        },
        {
          label: copy.footer.columns[2].links[2],
          href: 'https://pub.dev/packages/tray_manager',
          external: true,
        },
      ],
    },
  ]

  return (
    <footer className="border-t bg-background">
      <Container className="pt-16 pb-8">
        <div className="mb-12 grid grid-cols-1 gap-8 text-center sm:grid-cols-2 sm:text-left lg:grid-cols-[2fr_1fr_1fr_1fr] lg:gap-10">
          <div className="grid grid-cols-[auto_1fr] gap-5 items-center sm:col-span-2 lg:col-span-1">
            <BrandIcon className="size-16 md:size-20" />
            <div>
              <p className="text-sm leading-6 text-muted-foreground">
                {copy.footer.tagline}
                <br />
                {copy.footer.taglineSecond}
              </p>
            </div>
          </div>

          {columns.map(column => (
            <div
              key={column.title}
              className="flex flex-col items-center gap-2.5 sm:items-start"
            >
              <h3 className="mb-1 text-xs font-bold text-foreground uppercase">
                {column.title}
              </h3>
              {column.links.map(link => (
                <a
                  key={link.label}
                  href={link.href}
                  target={
                    'external' in link && link.external ? '_blank' : undefined
                  }
                  rel={
                    'external' in link && link.external
                      ? 'noreferrer'
                      : undefined
                  }
                  className="text-sm text-muted-foreground transition-colors hover:text-foreground"
                >
                  {link.label}
                </a>
              ))}
            </div>
          ))}
        </div>

        <div className="flex flex-col items-center justify-between gap-4 border-t pt-6 text-center sm:flex-row sm:text-left">
          <span className="text-sm text-muted-foreground">
            &copy; 2025-{new Date().getFullYear()} LiJianying. MIT License.
          </span>
          <div className="flex items-center gap-1">
            {socialLinks.map(link => (
              <Button
                key={link.label}
                asChild
                variant="ghost"
                size="icon"
                title={link.label}
              >
                <a
                  href={link.href}
                  target="_blank"
                  rel="noreferrer"
                  aria-label={link.label}
                >
                  <link.icon />
                </a>
              </Button>
            ))}
          </div>
        </div>
      </Container>
    </footer>
  )
}
