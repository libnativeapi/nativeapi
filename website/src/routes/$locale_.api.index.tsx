import { createFileRoute, notFound, redirect } from '@tanstack/react-router'
import { defaultApiLanguage } from '@/lib/api'
import { isLocale } from '@/lib/i18n'

export const Route = createFileRoute('/$locale_/api/')({
  beforeLoad: ({ params }) => {
    if (!isLocale(params.locale)) throw notFound()
    if (params.locale === 'en') throw redirect({ to: '/api' })
    throw redirect({
      to: '/$locale/api/$lang',
      params: { locale: params.locale, lang: defaultApiLanguage },
    })
  },
})
