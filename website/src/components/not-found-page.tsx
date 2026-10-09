import { useRouterState } from '@tanstack/react-router'
import { Button } from '@/components/ui/button'
import { localeFromPath, localePath } from '@/lib/i18n'
import { Container } from './container'
import { BrandLockup } from './brand-lockup'

const messages = {
  en: {
    title: 'Page not found',
    description:
      'The page you are looking for does not exist or may have moved.',
    action: 'Back to home',
  },
  ja: {
    title: 'ページが見つかりません',
    description: 'お探しのページは存在しないか、移動した可能性があります。',
    action: 'ホームに戻る',
  },
  zh: {
    title: '页面未找到',
    description: '你访问的页面不存在，或者已经被移动。',
    action: '返回首页',
  },
} as const

export function NotFoundPage() {
  const pathname = useRouterState({ select: state => state.location.pathname })
  const locale = localeFromPath(pathname)
  const copy = messages[locale]

  return (
    <main className="flex min-h-svh items-center bg-background py-16">
      <Container className="flex max-w-xl flex-col items-center gap-6 text-center">
        <a
          href={localePath(locale)}
          className="flex items-center gap-3"
          aria-label="nativeapi"
        >
          <BrandLockup className="h-10 w-auto" />
        </a>
        <div className="flex flex-col gap-3">
          <p className="text-sm font-semibold text-primary">404</p>
          <h1 className="text-3xl font-bold md:text-4xl">{copy.title}</h1>
          <p className="text-lg leading-8 text-muted-foreground">
            {copy.description}
          </p>
        </div>
        <Button asChild>
          <a href={localePath(locale)}>{copy.action}</a>
        </Button>
      </Container>
    </main>
  )
}
