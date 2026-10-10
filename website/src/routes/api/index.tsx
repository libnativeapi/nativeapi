import { createFileRoute, redirect } from '@tanstack/react-router'
import { defaultApiLanguage } from '@/lib/api'

export const Route = createFileRoute('/api/')({
  beforeLoad: () => {
    throw redirect({ to: '/api/$lang', params: { lang: defaultApiLanguage } })
  },
})
