import {
  ArrowUpRightIcon,
  BookOpenIcon,
  ChevronDownIcon,
  CircleHelpIcon,
  MessageCircleIcon,
} from 'lucide-react'
import { Container } from '@/components/container'
import { SiteFooter } from '@/components/site-footer'
import { SiteHeader } from '@/components/site-header'
import { Button } from '@/components/ui/button'
import {
  Card,
  CardContent,
  CardDescription,
  CardHeader,
  CardTitle,
} from '@/components/ui/card'
import {
  docsUrl,
  getMessages,
  githubUrl,
  localePath,
  type Locale,
} from '@/lib/i18n'
import { getSupportCopy } from './support-copy'


export function SupportPage({ locale }: { locale: Locale }) {
  const copy = getSupportCopy(locale)
  const faqs = getMessages(locale).home.faqs
  const cards = [
    {
      ...copy.cards[0],
      icon: BookOpenIcon,
      href: docsUrl(locale),
      external: false,
    },
    {
      ...copy.cards[1],
      icon: MessageCircleIcon,
      href: `${githubUrl}/issues`,
      external: true,
    },
    {
      ...copy.cards[2],
      icon: CircleHelpIcon,
      href: '#faq',
      external: false,
    },
  ] as const

  return (
    <div className="relative min-h-svh overflow-x-hidden bg-background">
      <SiteHeader locale={locale} pagePath="/support" />
      <div className="absolute inset-x-0 top-0 h-[34rem] bg-linear-to-b from-primary/10 via-background to-background" />

      <main className="relative pt-(--header-height)">
        <section className="pt-8 pb-12 text-center md:pt-12 md:pb-16">
          <Container className="max-w-4xl">
            <p className="text-sm font-semibold tracking-wide text-primary uppercase">
              {copy.eyebrow}
            </p>
            <h1 className="mt-4 text-4xl font-bold tracking-tight md:text-6xl">
              {copy.title}
            </h1>
            <p className="mx-auto mt-5 max-w-2xl text-lg leading-8 text-muted-foreground md:text-xl">
              {copy.description}
            </p>
          </Container>
        </section>

        <section className="pb-20 md:pb-24">
          <Container>
            <div className="grid gap-5 md:grid-cols-3">
              {cards.map(card => (
                <Card key={card.title} className="flex flex-col">
                  <CardHeader>
                    <div className="mb-4 flex size-11 items-center justify-center rounded-lg bg-primary/10 text-primary">
                      <card.icon className="size-5" aria-hidden="true" />
                    </div>
                    <CardTitle>{card.title}</CardTitle>
                    <CardDescription>{card.description}</CardDescription>
                  </CardHeader>
                  <CardContent className="mt-auto">
                    <Button asChild variant="outline" className="w-full">
                      <a
                        href={card.href}
                        target={card.external ? '_blank' : undefined}
                        rel={card.external ? 'noreferrer' : undefined}
                      >
                        {card.action}
                        <ArrowUpRightIcon data-icon="inline-end" />
                      </a>
                    </Button>
                  </CardContent>
                </Card>
              ))}
            </div>
          </Container>
        </section>

        <section
          id="faq"
          className="scroll-mt-20 border-y bg-muted/30 py-20 md:py-24"
        >
          <Container className="max-w-4xl">
            <div className="text-center">
              <p className="text-sm font-semibold tracking-wide text-primary uppercase">
                {copy.faqEyebrow}
              </p>
              <h2 className="mt-3 text-3xl font-bold md:text-4xl">
                {copy.faqTitle}
              </h2>
              <p className="mx-auto mt-4 max-w-2xl text-muted-foreground">
                {copy.faqDescription}
              </p>
            </div>
            <div className="mt-10 border-t">
              {faqs.map(item => (
                <details key={item.question} className="group border-b">
                  <summary className="flex cursor-pointer list-none items-center justify-between gap-6 py-6 text-left text-lg font-semibold">
                    {item.question}
                    <ChevronDownIcon className="shrink-0 transition-transform group-open:rotate-180" />
                  </summary>
                  <p className="max-w-3xl pb-6 leading-7 text-muted-foreground">
                    {item.answer}
                  </p>
                </details>
              ))}
            </div>
          </Container>
        </section>

        <section className="py-20 md:py-24">
          <Container className="max-w-3xl text-center">
            <h2 className="text-3xl font-bold">{copy.contactTitle}</h2>
            <p className="mx-auto mt-4 max-w-xl leading-7 text-muted-foreground">
              {copy.contactDescription}
            </p>
            <Button asChild size="lg" className="mt-7">
              <a href={githubUrl} target="_blank" rel="noreferrer">
                <MessageCircleIcon data-icon="inline-start" />
                {copy.contactAction}
              </a>
            </Button>
            <p className="mt-5 text-sm text-muted-foreground">
              <a
                className="underline underline-offset-4 hover:text-foreground"
                href={localePath(locale)}
              >
                {locale === 'en' ? 'Back to nativeapi' : locale === 'ja' ? 'nativeapi に戻る' : '返回 nativeapi'}
              </a>
            </p>
          </Container>
        </section>
      </main>

      <SiteFooter locale={locale} />
    </div>
  )
}
