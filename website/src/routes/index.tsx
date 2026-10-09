import { createFileRoute } from '@tanstack/react-router'
import {
  AppWindowIcon,
  ArrowRightIcon,
  BookOpenIcon,
  CheckIcon,
  ChevronDownIcon,
  CodeIcon,
  CopyIcon,
  CpuIcon,
  GitForkIcon,
  GithubIcon,
  LayersIcon,
  MenuSquareIcon,
  MonitorIcon,
  RefreshCwIcon,
  Settings2Icon,
  StarIcon,
  TerminalIcon,
} from 'lucide-react'
import { useEffect, useState } from 'react'
import { Container } from '@/components/container'
import { FeatureIllustration } from '@/components/feature-illustrations'
import { HeroIllustration } from '@/components/hero-illustration'
import { SiteFooter } from '@/components/site-footer'
import { SiteHeader } from '@/components/site-header'
import { Badge } from '@/components/ui/badge'
import { Button } from '@/components/ui/button'
import {
  Select,
  SelectContent,
  SelectGroup,
  SelectItem,
  SelectTrigger,
} from '@/components/ui/select'
import { getGithubStats, type GithubStats } from '@/lib/github-stats'
import { docsPath } from '@/lib/docs'
import {
  docsUrl,
  getMessages,
  githubUrl,
  homeLocaleRedirectScript,
  localeStorageKey,
  localePath,
  locales,
  type Locale,
} from '@/lib/i18n'

const defaultCopy = getMessages('en')

export const Route = createFileRoute('/')({
  head: () => ({
    meta: [
      { title: defaultCopy.metadata.title },
      {
        name: 'description',
        content: defaultCopy.metadata.description,
      },
    ],
    links: locales.map(locale => ({
      rel: 'alternate',
      hrefLang: locale,
      href: localePath(locale),
    })),
    scripts: [{ children: homeLocaleRedirectScript() }],
  }),
  loader: () => getGithubStats(),
  component: RouteComponent,
})

function RouteComponent() {
  const githubStats = Route.useLoaderData()
  return <HomePage locale="en" githubStats={githubStats} />
}

const baseBenefits = [
  { icon: CpuIcon },
  { icon: LayersIcon },
  { icon: RefreshCwIcon },
] as const

const baseFeatures = [
  { id: 'windows', icon: AppWindowIcon },
  { id: 'tray-menus', icon: MenuSquareIcon },
  { id: 'displays-input', icon: MonitorIcon },
  { id: 'system-services', icon: Settings2Icon },
  { id: 'generated-bindings', icon: CodeIcon },
] as const

const baseSteps = [{ number: '1' }, { number: '2' }, { number: '3' }] as const

const baseBindings = [
  {
    id: 'flutter',
    name: 'Flutter',
    color: '#02569B',
    command: 'flutter pub add nativeapi_flutter',
    docs: 'bindings/dart',
  },
  {
    id: 'dart',
    name: 'Dart',
    color: '#0175C2',
    command: 'dart pub add nativeapi',
    docs: 'bindings/dart',
  },
  {
    id: 'rust',
    name: 'Rust',
    color: '#CE422B',
    command: 'cargo add nativeapi',
    docs: 'bindings/rust',
  },
  {
    id: 'csharp',
    name: 'C#',
    color: '#512BD4',
    command: 'git clone --recursive https://github.com/libnativeapi/nativeapi.git',
    docs: 'bindings/csharp',
  },
  {
    id: 'js',
    name: 'JS / TS',
    color: '#D9A400',
    command: 'git clone --recursive https://github.com/libnativeapi/nativeapi.git',
    docs: 'bindings/js',
  },
  {
    id: 'python',
    name: 'Python',
    color: '#3776AB',
    command: 'git clone --recursive https://github.com/libnativeapi/nativeapi.git',
    docs: 'bindings/python',
  },
  {
    id: 'go',
    name: 'Go',
    color: '#00ADD8',
    command: 'git clone --recursive https://github.com/libnativeapi/nativeapi.git',
    docs: 'bindings/go',
  },
] as const

type Binding = (typeof baseBindings)[number]['id']

export function HomePage({
  locale,
  githubStats,
}: {
  locale: Locale
  githubStats: GithubStats
}) {
  const copy = getMessages(locale)
  const docsGettingStarted = docsUrl(locale, true)
  const benefits = baseBenefits.map((benefit, index) => ({
    ...benefit,
    ...copy.home.benefits[index],
  }))
  const features = baseFeatures.map((feature, index) => ({
    ...feature,
    ...copy.home.featureItems[index],
  }))
  const steps = baseSteps.map((step, index) => ({
    ...step,
    ...copy.home.steps[index],
  }))
  const faqs = copy.home.faqs
  const installCommands = baseBindings.map((binding, index) => ({
    ...binding,
    description: copy.home.bindingDescriptions[index],
  }))
  const [selectedPlatform, setSelectedPlatform] = useState<Binding>('flutter')
  const [copiedInstall, setCopiedInstall] = useState<Binding | null>(null)
  const selectedInstall =
    installCommands.find(binding => binding.id === selectedPlatform) ??
    installCommands[0]
  const trustItems = [
    { value: '6', label: copy.home.trustLabels[0] },
    { value: '5', label: copy.home.trustLabels[1] },
    {
      value: formatNumber(githubStats.stars, locale),
      label: copy.home.trustLabels[2],
    },
    { value: 'MIT', label: copy.home.trustLabels[3] },
  ]

  useEffect(() => {
    // '/' handles its own redirect synchronously via homeLocaleRedirectScript
    // (see the route's head()) so it never flashes the English homepage.
    if (locale !== 'en') {
      window.localStorage.setItem(localeStorageKey, locale)
    }
  }, [locale])

  async function copyInstallCommand(
    platform: Binding,
    command: string
  ) {
    try {
      await navigator.clipboard.writeText(command)
      setCopiedInstall(platform)
      window.setTimeout(() => {
        setCopiedInstall(current => (current === platform ? null : current))
      }, 2000)
    } catch {
      setCopiedInstall(null)
    }
  }

  return (
    <div className="relative min-h-svh overflow-x-hidden bg-background">
      <SiteHeader locale={locale} />
      <div className="absolute inset-x-0 top-0 h-[520px] bg-linear-to-b from-blue-50 via-white to-white dark:from-gray-900 dark:via-gray-800 dark:to-background" />

      <main className="relative z-10">
        <section className="pt-(--header-height)">
          <Container className="grid items-center gap-12 pt-10 pb-8 md:pb-12 lg:min-h-[620px] lg:grid-cols-[0.9fr_1.1fr] lg:gap-16 lg:pt-16 lg:pb-20">
            <div className="order-2 text-center lg:order-1 lg:text-left">
              <Badge variant="outline" className="mb-5 bg-background/70">
                <GithubIcon />
                {copy.home.badge}
              </Badge>
              <h1 className="text-4xl leading-[1.12] font-bold md:text-5xl">
                <span className="block">{copy.home.heroLine1}</span>
                <span className="block">
                  {copy.home.heroLine2}
                  <span className="text-primary">
                    {copy.home.heroHighlight}
                  </span>
                </span>
              </h1>
              <p className="mx-auto mt-5 max-w-xl text-lg leading-8 text-muted-foreground md:text-xl lg:mx-0">
                {copy.home.heroDescription}
              </p>
              <div className="mt-8 flex flex-col items-center gap-3 sm:flex-row sm:justify-center lg:justify-start">
                <Button size="lg" className="min-w-40 font-semibold" asChild>
                  <a href="#install">
                    <TerminalIcon />
                    {copy.home.installCta}
                  </a>
                </Button>
                <Button
                  size="lg"
                  variant="outline"
                  className="min-w-40 border-2 border-primary bg-transparent font-semibold text-primary hover:bg-primary/5 hover:text-primary"
                  asChild
                >
                  <a href={docsGettingStarted}>
                    <BookOpenIcon />
                    {copy.home.readDocs}
                  </a>
                </Button>
              </div>
              <p className="mt-4 text-sm text-muted-foreground">
                {copy.home.heroNote}
              </p>
            </div>

            <div className="order-1 lg:order-2">
              <HeroIllustration className="mx-auto aspect-[16/10] w-full max-w-xl lg:max-w-none" />
            </div>
          </Container>
        </section>

        <section className="border-y bg-background/70">
          <Container>
            <div className="grid grid-cols-2 lg:grid-cols-4">
              {trustItems.map((item, index) => (
                <div
                  key={item.label}
                  className={`py-7 text-center ${
                    index % 2 ? 'border-l' : ''
                  } ${index >= 2 ? 'border-t lg:border-t-0' : ''} ${
                    index === 2 ? 'lg:border-l' : ''
                  }`}
                >
                  <strong className="block text-2xl font-bold md:text-3xl">
                    {item.value}
                  </strong>
                  <span className="mt-1 block text-xs font-medium text-muted-foreground uppercase">
                    {item.label}
                  </span>
                </div>
              ))}
            </div>
          </Container>
        </section>

        <section className="landing-reveal border-b py-20 md:py-24">
          <Container className="max-w-4xl text-center">
            <p className="text-xl leading-9 text-muted-foreground md:text-2xl">
              {copy.home.problem}
              <br className="hidden md:block" />
              {copy.home.problemContinuation}
            </p>
            <h2 className="mt-5 text-3xl font-bold md:text-4xl">
              {copy.home.promise}
            </h2>
          </Container>
        </section>

        <section id="features" className="scroll-mt-20 py-20 md:py-24">
          <Container>
            <div className="mx-auto mb-10 max-w-2xl text-center md:mb-14">
              <p className="text-sm font-semibold text-primary uppercase">
                {copy.home.featuresEyebrow}
              </p>
              <h2 className="mt-3 text-3xl font-bold md:text-4xl">
                {copy.home.featuresTitle}
              </h2>
              <p className="mt-4 text-lg text-muted-foreground">
                {copy.home.featuresDescription}
              </p>
            </div>

            <div className="border-t">
              {features.map(feature => (
                <article
                  id={feature.id}
                  key={feature.id}
                  className="landing-reveal flex scroll-mt-24 flex-col items-center gap-8 border-b py-14 lg:flex-row lg:gap-16 lg:py-20 lg:even:flex-row-reverse"
                >
                  <div className="w-full lg:basis-1/2">
                    <FeatureIllustration id={feature.id} />
                  </div>
                  <div className="w-full lg:basis-1/2">
                    <Badge variant="secondary" className="mb-4">
                      <feature.icon />
                      {feature.category}
                    </Badge>
                    <h3 className="text-2xl font-semibold md:text-3xl">
                      {feature.title}
                    </h3>
                    <p className="mt-4 text-lg leading-8 text-muted-foreground">
                      {feature.details}
                    </p>
                    <Button
                      asChild
                      variant="link"
                      className="mt-5 h-auto px-0 text-base"
                    >
                      <a href={docsGettingStarted}>
                        {copy.home.viewDocs}
                        <ArrowRightIcon />
                      </a>
                    </Button>
                  </div>
                </article>
              ))}
            </div>
          </Container>
        </section>

        <section className="landing-reveal border-y bg-muted/30 py-20">
          <Container>
            <div className="grid gap-4 md:grid-cols-3">
              {benefits.map(benefit => (
                <div
                  key={benefit.title}
                  className="rounded-lg border bg-background p-6"
                >
                  <div className="mb-5 flex size-11 items-center justify-center rounded-lg bg-primary/10 text-primary">
                    <benefit.icon />
                  </div>
                  <h3 className="text-lg font-semibold">{benefit.title}</h3>
                  <p className="mt-2 leading-7 text-muted-foreground">
                    {benefit.description}
                  </p>
                </div>
              ))}
            </div>
          </Container>
        </section>

        <section className="landing-reveal py-20 md:py-24">
          <Container className="max-w-5xl">
            <h2 className="text-center text-3xl font-bold md:text-4xl">
              {copy.home.stepsTitle}
            </h2>
            <div className="mt-12 grid gap-8 md:grid-cols-3">
              {steps.map(step => (
                <div key={step.number} className="text-center">
                  <div className="mx-auto flex size-12 items-center justify-center rounded-lg border border-primary/20 bg-primary/10 text-lg font-bold text-primary">
                    {step.number}
                  </div>
                  <h3 className="mt-5 text-lg font-semibold">{step.title}</h3>
                  <p className="mx-auto mt-2 max-w-xs leading-7 text-muted-foreground">
                    {step.description}
                  </p>
                </div>
              ))}
            </div>
            <div className="mt-14 flex flex-col items-center gap-3">
              <Button size="lg" asChild>
                <a href="#install">
                  <TerminalIcon />
                  {copy.home.installCta}
                </a>
              </Button>
              <span className="text-sm text-muted-foreground">
                {copy.home.nativeCli}
              </span>
            </div>
          </Container>
        </section>

        <section className="landing-reveal border-y bg-muted/30 py-20 md:py-24">
          <Container className="grid items-center gap-10 md:grid-cols-[1fr_auto]">
            <div>
              <h2 className="mt-5 text-3xl font-bold md:text-4xl">
                {copy.home.communityTitle}
              </h2>
              <p className="mt-4 max-w-2xl text-lg leading-8 text-muted-foreground">
                {copy.home.communityDescription}
              </p>
              <Button className="mt-6" variant="outline" asChild>
                <a
                  href={githubUrl}
                  target="_blank"
                  rel="noreferrer"
                >
                  <GithubIcon />
                  {copy.home.viewGithub}
                </a>
              </Button>
            </div>
            <div className="grid grid-cols-2 gap-3">
              <div className="min-w-32 rounded-lg border bg-background p-5 text-center">
                <StarIcon className="mx-auto text-primary" />
                <strong className="mt-3 block text-2xl">
                  {formatNumber(githubStats.stars, locale)}
                </strong>
                <span className="text-xs text-muted-foreground">Stars</span>
              </div>
              <div className="min-w-32 rounded-lg border bg-background p-5 text-center">
                <GitForkIcon className="mx-auto text-primary" />
                <strong className="mt-3 block text-2xl">
                  {formatNumber(githubStats.forks, locale)}
                </strong>
                <span className="text-xs text-muted-foreground">Forks</span>
              </div>
            </div>
          </Container>
        </section>

        <section
          id="faq"
          className="landing-reveal scroll-mt-20 py-20 md:py-24"
        >
          <Container className="max-w-4xl">
            <h2 className="text-center text-3xl font-bold md:text-4xl">
              {copy.home.faqTitle}
            </h2>
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

        <section
          id="install"
          className="landing-reveal scroll-mt-20 border-t py-20 text-center md:py-24"
        >
          <Container className="max-w-5xl">
            <h2 className="text-3xl font-bold md:text-5xl">
              {copy.home.installTitle}
            </h2>
            <p className="mt-3 text-lg text-muted-foreground">
              {copy.home.installDescription}
            </p>

            <div className="mx-auto mt-10 max-w-3xl">
              <div className="flex h-14 min-w-0 items-stretch overflow-hidden rounded-lg border bg-background shadow-xs transition-shadow focus-within:ring-2 focus-within:ring-ring/20">
                <Select
                  value={selectedPlatform}
                  onValueChange={value => {
                    setSelectedPlatform(value as Binding)
                    setCopiedInstall(null)
                  }}
                >
                  <SelectTrigger
                    className="h-full w-32 shrink-0 rounded-none border-0 border-r px-4 shadow-none focus:ring-0 sm:w-40"
                    aria-label={copy.home.selectBinding}
                  >
                    <span className="inline-flex items-center gap-2 font-medium">
                      <BindingDot color={selectedInstall.color} />
                      {selectedInstall.name}
                    </span>
                  </SelectTrigger>
                  <SelectContent>
                    <SelectGroup>
                      {installCommands.map(platform => (
                        <SelectItem key={platform.id} value={platform.id}>
                          <BindingDot color={platform.color} />
                          {platform.name}
                        </SelectItem>
                      ))}
                    </SelectGroup>
                  </SelectContent>
                </Select>

                <code className="min-w-0 flex-1 overflow-x-auto whitespace-nowrap px-4 py-[17px] text-left font-mono text-sm text-foreground">
                  {selectedInstall.command}
                </code>

                <div className="flex shrink-0 items-center border-l px-2">
                  <Button
                    type="button"
                    size="icon"
                    variant="ghost"
                    onClick={() =>
                      void copyInstallCommand(
                        selectedInstall.id,
                        selectedInstall.command
                      )
                    }
                    aria-label={`${copy.home.copyCommand}: ${selectedInstall.name}`}
                    title={
                      copiedInstall === selectedInstall.id
                        ? copy.home.copied
                        : copy.home.copyCommand
                    }
                  >
                    {copiedInstall === selectedInstall.id ? (
                      <CheckIcon data-icon="inline-start" />
                    ) : (
                      <CopyIcon data-icon="inline-start" />
                    )}
                  </Button>
                </div>
              </div>

              <div className="mt-3 flex flex-col items-center justify-between gap-2 text-sm text-muted-foreground sm:flex-row">
                <span>{selectedInstall.description}</span>
                <a
                  href={docsPath(locale, selectedInstall.docs)}
                  className="inline-flex items-center gap-1 text-primary hover:underline"
                >
                  {copy.home.quickStart}
                  <ArrowRightIcon className="size-4" />
                </a>
              </div>
            </div>
          </Container>
        </section>
      </main>
      <SiteFooter locale={locale} />
    </div>
  )
}

function BindingDot({ color }: { color: string }) {
  return (
    <span
      className="inline-block size-2.5 shrink-0 rounded-full"
      style={{ backgroundColor: color }}
      aria-hidden="true"
    />
  )
}

function formatNumber(value: number, locale: Locale) {
  return new Intl.NumberFormat(locale).format(value)
}
